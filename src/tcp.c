#include "tcp.h"
#include "log.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>

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
    sock -> snd_nxt=1;
    return sock;
}

static ssize_t send_data(Socket *sock, const char *data, size_t sz)
{
    ssize_t ret;

    ret = send(sock->fd, data, sz+1, 0);

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
    Socket *sock;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    struct sockaddr_in localaddr;
    int ret;

    sock = tcp_socket();
    if (!sock) return NULL;

    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = 0;

    /* Try to get a free port number between 1023 and 9999 */
    *port = 1023;
    do {
        if (*port >= 9999) {
            close(sock->fd);
            ERRNO("Can't find port free number");
            return NULL;
        }

        (*port)++;
        localaddr.sin_port = *port;
        ret = bind(sock->fd, (struct sockaddr *) &localaddr, addrlen);
    } while (ret < 0);

    associate_socket(sock, distant);

    return sock;
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

void tcp_close(Socket *socket)
{
    send_data(socket, "FIN", 3);
    close(socket->fd);
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

ssize_t tcp_send(Socket *s, const char *in, size_t sz)
{
    char buffer[BUFSIZE+6];
    char ack[10];
    ssize_t ret;
    snprintf(buffer, BUFSIZE+6, "%06d", s->snd_nxt);
    u8 acked = 0;

    sz = min(sz, BUFSIZE);
    memcpy(buffer+6, in, sz);

    snprintf(ack, 10, "ACK%06d", s->snd_nxt);

    DEBUG("Waiting for : %s", ack);

    while (!acked) {
        DEBUG("Sending : %s", buffer);
        ret = send_data(s, buffer, sz + 5);

        recv_data(s, buffer, BUFSIZE);
        acked = (strcmp(buffer, ack) == 0);
        DEBUG("Received : %s", buffer);
    }

    s->snd_nxt += 1;

    return ret;
}

ssize_t tcp_recv(Socket *s, char *out, size_t sz)
{
    return recv_data(s, out, sz);
}
