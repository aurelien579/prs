#include "tcp.h"
#include <stdlib.h>
#include <stdio.h>

#include "queue.h"

static void usage(const char *prog)
{
    printf("Usage: %s port\n", prog);
    exit(0);
}

static int send_file(const char *filename, Socket *s)
{
    char buffer[BUFSIZE];
    size_t size;
    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "Can't open file : %s\n", filename);
        return -1;
    }

    tcp_start_transfer(s, 50, 100);

    while ((size = fread(buffer, 1, BUFSIZE, file)) > 0)
        tcp_send(s, buffer, size);

    tcp_wait(s);

    printf("File sent\n");

    return 0;
}


#ifdef MULTI

static void *srv3_handler(void *_arg)
{
    Socket  *cltsock = _arg;
    char    filename[BUFSIZE];

    tcp_recv(cltsock, filename, BUFSIZE);
    printf("Sending file : %s\n", filename);

    send_file(filename, cltsock);

    tcp_close(cltsock);

    return NULL;
}

static void srv3(Socket *srvsock)
{
    struct sockaddr_in  addr;
    Socket              *cltsock;
    pthread_t           clients[100] = { 0 };
    int                 clients_count = 0;

    while ((cltsock = tcp_accept(srvsock, &addr)))
        pthread_create(&clients[clients_count++], NULL, srv3_handler, cltsock);

    for (int i = 0; i < clients_count; i++)
        if (clients[i] != 0)
            pthread_join(clients[i], NULL);
}

#else

static void srv1(Socket *srvsock)
{
    struct sockaddr_in  addr;
    Socket              *cltsock;
    char                filename[BUFSIZE];

    cltsock = tcp_accept(srvsock, &addr);

    tcp_recv(cltsock, filename, sizeof(filename));
    printf("Sending file : %s\n", filename);

    send_file(filename, cltsock);

    tcp_close(cltsock);
}

#endif


int main(int argc, char **argv)
{
    unsigned short  port;
    Socket          *srvsock;

    if (argc < 2) usage(argv[0]);

    port = (unsigned short) atoi(argv[1]);

    srvsock = tcp_socket();
    tcp_bind(srvsock, "0.0.0.0", port);

#ifdef MULTI
    srv3(srvsock);
#else
    srv1(srvsock);
#endif

    tcp_close(srvsock);

    return 0;
}
