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
    #define ERROR(...)   log_error("SRV", __VA_ARGS__)
    #define ERRNO(...)   log_errno("SRV", __VA_ARGS__)
#else
    #define ERROR(...)
    #define ERRNO(...)
#endif



static void usage(const char *prog)
{
    printf("Usage: %s port\n", prog);
    exit(0);
}

static int send_file(const char *filename, Socket *sock)
{
    char buffer[BUFSIZE];
    size_t size;
    FILE *file = fopen(filename, "r");

    if (!file) return -1;

    tcp_start_transfer(sock);

    while ((size = fread(buffer, 1, BUFSIZE, file)) > 0) {
        tcp_send(sock, buffer, size);
    }

    tcp_wait(sock);

    DEBUG("File sent, last ret = %d", size);

    return 0;
}

static void handle_client(Socket *client)
{
    char buffer[BUFSIZE];

    tcp_recv(client, buffer, sizeof(buffer));
    DEBUG("Sending file : %s", buffer);

    send_file(buffer, client);

    tcp_close(client);
}

int main(int argc, char **argv)
{
    unsigned short port;
    struct sockaddr_in distant;
    Socket *socket;
    Socket *client;

    if (argc < 2) usage(argv[0]);
    port = (unsigned short) atoi(argv[1]);

    log_init(NULL);
    DEBUG("Server start");

    socket = tcp_socket();
    tcp_bind(socket, "0.0.0.0", port);

    client = tcp_accept(socket, &distant);

    handle_client(client);

    tcp_close(socket);

    return 0;
}
