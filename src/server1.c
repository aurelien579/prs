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

size_t fread_at(FILE *file, char *out, size_t sz, long pos)
{
    fseek(file, pos, SEEK_SET);
    return fread(out, 1, sz, file);
}

int send_file(const char *filename, Socket *s)
{
    char buffer[BUFSIZE];
    size_t size;
    FILE *file = fopen(filename, "r");

    if (!file) {
        ERRNO("Can't open file : %s", buffer);
        return -1;
    }

    while ((size = fread(buffer, 1, BUFSIZE, file)) > 0) {
        tcp_send(s, buffer, size);
    }

    tcp_wait(s);

    DEBUG("File sent, last ret = %d", size);

    return 0;
}



void test()
{
    char *data = "test";
    Queue q;
    queue_init(&q);

    queue_insert_ordered(&q, queue_entry_new(data, 0, 0, 0, 0));
    queue_insert_ordered(&q, queue_entry_new(data, 1, 0, 0, 0));
    queue_insert_ordered(&q, queue_entry_new(data, 2, 0, 0, 0));
    queue_insert_ordered(&q, queue_entry_new(data, 3, 0, 0, 0));
    queue_insert_ordered(&q, queue_entry_new(data, 4, 0, 0, 0));
    queue_insert_ordered(&q, queue_entry_new(data, 5, 0, 0, 0));

    queue_print(&q);

    queue_remove(&q, 1);

    queue_print(&q);

    exit(0);
}

int main(int argc, char **argv)
{
    u16 port;
    char buffer[BUFSIZE];
    struct sockaddr_in distant;

    if (argc < 2) usage(argv[0]);

    port = (u16) atoi(argv[1]);

    log_init(NULL);
    DEBUG("Server start");

    Socket *socket = tcp_socket();
    tcp_bind(socket, "0.0.0.0", port);

    Socket *other = tcp_accept(socket, &distant);

    tcp_recv(other, buffer, BUFSIZE);
    DEBUG("Sending file : %s", buffer);

    send_file(buffer, other);

    tcp_close(socket);
    tcp_close(other);

    DEBUG("Server close");
    return 0;
}
