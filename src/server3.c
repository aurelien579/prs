#include "log.h"
#include "tcp.h"
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

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

int send_file(const char *filename, Socket *s)
{
    char buffer[BUFSIZE];
    size_t size;
    FILE *file = fopen(filename, "r");

    if (!file) {
        ERRNO("Can't open file : %s", filename);
        return -1;
    }

    tcp_start_transfer(s, 0, 100);

    while ((size = fread(buffer, 1, BUFSIZE, file)) > 0) {
        tcp_send(s, buffer, size);
    }

    tcp_wait(s);

    DEBUG("File sent, last ret = %d", size);

    return 0;
}

void *client_handler(void *_socket)
{
    Socket *socket = _socket;
    char buffer[BUFSIZE];

    printf("Receiving filename...\n");

    tcp_recv(socket, buffer, BUFSIZE);
    DEBUG("Sending file : %s", buffer);

    send_file(buffer, socket);

    tcp_close(socket);

    return NULL;
}

int main(int argc, char **argv)
{
    u16 port;
    struct sockaddr_in distant;
    pthread_t clients[100];
    int clients_count = 0;
    memset(clients, 0, sizeof(clients));

    if (argc < 2) usage(argv[0]);

    port = (u16) atoi(argv[1]);

    log_init(NULL);
    DEBUG("Server start");

    Socket *socket = tcp_socket();
    tcp_bind(socket, "0.0.0.0", port);
    Socket *other = NULL;

    while ((other = tcp_accept(socket, &distant))) {
        pthread_create(&clients[clients_count++], NULL, client_handler, other);
    }

    for (int i = 0; i < clients_count; i++) {
        if (clients[i] != 0) {
            pthread_join(clients[i], NULL);
        }
    }

    tcp_close(socket);

    DEBUG("Server close");
    return 0;
}
