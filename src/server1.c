#include "log.h"
#include "tcp.h"
#include <stdlib.h>
#include <stdio.h>

#if LOG_LEVEL >= LOG_DEBUG
    #define DEBUG(...)   log_debug("SRV", __VA_ARGS__)
#else
    #define DEBUG(...)
#endif

#if LOG_LEVEL >= ERROR
    #define ERROR(...)   log_error("SRV", __VA_ARGS__);
    #define ERRNO(...)   log_errno("SRV", __VA_ARGS__);
#else
    #define ERROR(...)
    #define ERRNO(...)
#endif

void usage(const char *prog)
{
    printf("Usage: %s port\n", prog);
    exit(0);
}

size_t fread_at(FILE *file, char *out, size_t sz, long pos)
{
    fseek(file, pos, SEEK_SET);
    return fread(out, 1, sz, file);
}

int main(int argc, char **argv)
{
    u16 port;
    char buffer[BUFSIZE];
    ssize_t ret;
    FILE *file;

    if (argc < 2) usage(argv[0]);

    port = (u16) atoi(argv[1]);

    log_init(NULL);
    DEBUG("Server start");

    struct sockaddr_in distant;

    Socket *socket = tcp_socket();
    tcp_bind(socket, "0.0.0.0", port);

    Socket *other = tcp_accept(socket, &distant);

    tcp_recv(other, buffer, BUFSIZE);
    DEBUG("Sending file : %s", buffer);

    file = fopen(buffer, "r");

    if (!file) {
        ERRNO("Can't open file : %s", buffer);
        tcp_close(socket);
        tcp_close(other);
        return -1;
    }

    while ((ret = fread(buffer, 1, BUFSIZE, file)) > 0) {
        tcp_send(other, buffer, ret);
    }

    tcp_close(other);
    tcp_close(socket);

    DEBUG("Server close");
    return 0;
}
