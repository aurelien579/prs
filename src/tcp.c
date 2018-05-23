#include "tcp.h"
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
    sock->ssthresh = INI_SSTRESH;

    sock->srtt = INITRTT;

    return sock;
}

static ssize_t send_data(Socket *sock, const char *data, size_t sz)
{
    return send(sock->fd, data, sz, 0);
}

static ssize_t recv_data(Socket *sock, char *data, size_t sz)
{
    return recv(sock->fd, data, sz, 0);
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
            fprintf(stderr, "Can't find port free number\n");
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

static void make_packet(char *packet, const char *data, size_t sz, seq_t seq)
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
        fprintf(stderr, "Can't receive SYN\n");
        return NULL;
    }

    if (strncmp(buffer, "SYN", BUFSIZE)) {
        fprintf(stderr, "Invalid packet received: %s\n", buffer);
        return NULL;
    }

    /* Creating the new dedicated socket */
    new_sock = create_dedicated_socket(distant_addr, &new_port);
    if (!new_sock) {
        fprintf(stderr, "Can't create dedicated socket\n");
        return NULL;
    }

    /* Sending the dedicated port number */
    associate_socket(sock, distant_addr);
    ret = snprintf(buffer, BUFSIZE, "SYN-ACK%d", new_port);
    ret = send_data(sock, buffer, ret);
    if (ret < 0) {
        fprintf(stderr, "Can't send SYN\n");
        tcp_close(new_sock);
        return NULL;
    }

    /* Receiving the last ACK */
    ret = recv_data(sock, buffer, BUFSIZE);
    if (ret < 0) {
        fprintf(stderr, "Can't receive the last ACK\n");
        tcp_close(new_sock);
        return NULL;
    }
    if (strncmp(buffer, "ACK", BUFSIZE)) {
        fprintf(stderr, "Invalid packet received: %s\n", buffer);
        tcp_close(new_sock);
        return NULL;
    }

    fprintf(stderr, "Last ACK received, connection established\n");

    disassociate_socket(sock);

    return new_sock;
}

void tcp_start_transfer(Socket *sock, ulong_t sleep, int count)
{
    queue_init(&sock->queue);
    recv_thread_init(&sock->recv_thread, sock);
    send_thread_init(&sock->send_thread, sock->fd, &sock->queue, sleep, count);
}

void tcp_close(Socket *socket)
{
    recv_thread_stop(&socket->recv_thread);

    /*for (int i = 0; i < 50; i++) {
        send_data(socket, "FIN", 4);
        usleep(5);
    }*/

    system("killall bin/client1*");

    close(socket->fd);
}

ssize_t tcp_recv(Socket *s, char *out, size_t sz)
{
    return recv_data(s, out, sz);
}

void tcp_send(Socket *s, const char *in, size_t sz)
{
    char packet[PACKET_SIZE];

    if (sz > DATA_SIZE) return;

    make_packet(packet, in, sz, (seq_t) s->que_nxt);

    pthread_spin_lock(&s->queue.lock);
    queue_add_entry(&s->queue, packet, s->que_nxt, sz + HEADER_SIZE, 0, 0);
    pthread_spin_unlock(&s->queue.lock);

    s->que_nxt++;
}

void tcp_wait(Socket *sock)
{
    while (queue_readable(&sock->queue) > 0) usleep(1);
}
