#include "tcp.h"
#include "log.h"
#include "utils.h"
#include "consts.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>


#if LOG_LEVEL >= LOG_DEBUG
    #define DEBUG(...)   log_debug("TCP", __VA_ARGS__)
#else
    #define DEBUG(...)
#endif

#if LOG_LEVEL >= ERROR
    #define ERROR(...)   log_error("TCP", __VA_ARGS__);
    #define ERRNO(...)   log_errno("TCP", __VA_ARGS__);
#else
    #define ERROR(...)
    #define ERRNO(...)
#endif

static Socket *tcp_socket_new(int fd)
{
    Socket *sock;

    sock = malloc(sizeof(Socket));
    memset(sock, 0, sizeof(Socket));

    sock->fd = fd;
    sock->snd_nxt = 1;
    sock->snd_wnd = INIT_WINDOW;
    sock->snd_una = 1;
    sock->que_nxt = 1;
    sock->ssthresh= INI_SSTRESH;

    sock->srtt = INITRTT;

    return sock;
}

static ssize_t send_data(Socket *sock, const char *data, size_t sz)
{
    ssize_t ret;

    ret = send(sock->fd, data, sz, 0);

    return ret;
}

static ssize_t recv_data(Socket *sock, char *data, size_t sz)
{
    ssize_t ret;

    ret = recv(sock->fd, data, sz, 0);

    return ret;
}

static int associate_socket(Socket *s, struct sockaddr_in *addr)
{
    return connect(s->fd, (struct sockaddr *) addr, sizeof(struct sockaddr_in));
}

static int disassociate_socket(Socket *sock)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_UNSPEC;
    return associate_socket(sock, &addr);
}

static Socket *create_dedicated_socket(struct sockaddr_in *distant, u16 *port)
{
    static u16 last_port = 1023;

    Socket *sock;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in localaddr;
    int ret;

    sock = tcp_socket();
    if (!sock) return NULL;

    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = 0;

    /* Try to get a free port number between 1023 and 9999 */
    *port = last_port;
    do {
        if (*port >= 9999) {
            close(sock->fd);
            ERRNO("Can't find port free number");
            return NULL;
        }

        (*port)++;
        localaddr.sin_port = htons(*port);
        ret = bind(sock->fd, (struct sockaddr *) &localaddr, addrlen);
    } while (ret != 0);

    associate_socket(sock, distant);

    last_port = *port;
    return sock;
}

int recv_ack(Socket *s)
{
    char buffer[10];

    recv_data(s, buffer, sizeof(buffer));
    return atoi(buffer + 3);
}

static int set_socket_timeout(Socket *s, int secs, int usecs)
{
    struct timeval tv;
    tv.tv_sec = secs;
    tv.tv_usec = usecs;
    return setsockopt(s->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *) &tv, sizeof(tv));
}

static void make_packet(char *packet, const char *data, size_t sz, u16 seq)
{
    memset(packet, 0, PACKET_SIZE);
    snprintf(packet, PACKET_SIZE, "%06d", seq);
    memcpy(packet + HEADER_SIZE, data, sz);
}

void tcp_bind(Socket *sock, const char *ip, u16 port)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);

    bind(sock->fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
}

Socket *tcp_socket()
{
    int fd, yes = 1;

    DEBUG("Creating TCP socket");

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return NULL;

    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    return tcp_socket_new(fd);
}

Socket *tcp_accept(Socket *sock, struct sockaddr_in *distant_addr)
{
    char buffer[BUFSIZE];
    ssize_t ret;
    Socket *new_sock;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    u16 new_port;

    /* Receiving first SYN */
    ret = recvfrom(sock->fd, buffer, BUFSIZE, 0,
                   (struct sockaddr *) distant_addr, &addrlen);
    if (ret < 0) {
        ERRNO("Can't receive SYN");
        return NULL;
    }

    if (strncmp(buffer, "SYN", BUFSIZE)) {
        ERROR("Invalid packet received: %s", buffer);
        return NULL;
    }

    DEBUG("SYN received from : %s", inet_ntoa(distant_addr->sin_addr));

    /* Creating the new dedicated socket */
    new_sock = create_dedicated_socket(distant_addr, &new_port);
    if (!new_sock) {
        ERROR("Can't create dedicated socket");
        return NULL;
    }

    DEBUG("Dedicated socket created with port %d", new_port);

    /* Sending the dedicated port number */
    associate_socket(sock, distant_addr);
    ret = snprintf(buffer, BUFSIZE, "SYN-ACK%d", new_port);
    ret = send_data(sock, buffer, ret);
    if (ret < 0) {
        ERRNO("Can't send SYN");
        tcp_close(new_sock);
        return NULL;
    }

    DEBUG("SYN-ACK sent");

    /* Receiving the last ACK */
    ret = recv_data(sock, buffer, BUFSIZE);
    if (ret < 0) {
        ERRNO("Can't receive the last ACK");
        tcp_close(new_sock);
        return NULL;
    }
    if (strncmp(buffer, "ACK", BUFSIZE)) {
        ERROR("Invalid packet received: %s", buffer);
        tcp_close(new_sock);
        return NULL;
    }
    DEBUG("Last ACK received, connection established");

    disassociate_socket(sock);

    return new_sock;
}

void tcp_start_transfer(Socket *sock)
{
    queue_init(&sock->queue, MAX_WINDOW);
    clock_init(&sock->clock, sock, CLK_US);
    recv_thread_init(&sock->recv_thread, sock);
}

void tcp_close(Socket *socket)
{
    recv_thread_stop(&socket->recv_thread);
    for (int i = 0; i < 10; i++)
        send_data(socket, "FIN", 4);
    close(socket->fd);

}

static ssize_t send_and_retransmit(Socket *s, const char *in, size_t data_size)
{
    u16 ack = 0;
    ssize_t ret;

    set_socket_timeout(s, 0, 1000);

    while (ack != s->snd_nxt) {
        DEBUG("Sending packet (seq = %d, size = %d)", s->snd_nxt, data_size);

        ret = send_data(s, in, data_size + 6);
        ack = recv_ack(s);

        DEBUG("ACK Received : %d", ack);
    }

    return ret;
}

/* sz < BUFSIZE !!! */
void tcp_send(Socket *s, const char *in, size_t sz)
{
    char packet[PACKET_SIZE];
    QueueEntry *entry;

    if (sz > DATA_SIZE) return;

    make_packet(packet, in, sz, s->que_nxt);
    entry = queue_entry_new(packet, s->que_nxt, sz + HEADER_SIZE, 0, 0);
    s->que_nxt++;

    queue_insert_ordered(&s->queue, entry);

    if (s->snd_nxt < s->snd_una + s->snd_wnd)
        tcp_output(s);
}

ssize_t tcp_recv(Socket *s, char *out, size_t sz)
{
    return recv_data(s, out, sz);
}

void tcp_output(Socket *sock)
{
    QueueEntry *entry;

    pthread_mutex_lock(&sock->queue.mutex);

    entry = sock->queue.top;
    while (entry) {
        if (entry->rtx_usecs == 0) {
            if (entry->rtx_count != 0 || sock->snd_nxt < sock->snd_una + sock->snd_wnd) {
                send(sock->fd, entry->packet, entry->size, 0);

                if (entry->rtx_count == 0) {
                    //printf("SEND %d\n", entry->seq);
                    sock->snd_nxt++;
                } else {
                    //printf("RESEND %d %d\n", entry->seq, entry->rtx_count); // HOP ON DETECTE UNE COLLISION
#ifndef NO_CONGESTION
                    sock -> snd_wnd = max(MIN_WINDOW, sock ->snd_wnd - sock ->snd_wnd/4);
                    if(entry->rtx_count==1){
                      sock->ssthresh = max (SSTHRESH_OFFSET+(int)((sock->snd_nxt-sock->snd_una) / 2), MIN_SSTHRESH);
                    }
#endif
                }

                entry->rtx_count++;
                entry->rtx_usecs = sock->srtt + K*sock->rttvar;
                gettimeofday(&entry->tx_time, NULL);
            }
        }

        entry = entry->next;
    }

    pthread_mutex_unlock(&sock->queue.mutex);
}

void tcp_wait(Socket *sock)
{
    while (sock->queue.top) {
        //usleep(10);
    }
}
