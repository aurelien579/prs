#include "send.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

static void *__send(void *_thread)
{
    SendThread *thread = _thread;
    QueueEntry *entry;
    int i;

    while (thread->running) {
        pthread_mutex_lock(&thread->queue->mutex);

        entry = thread->queue->top;
        i = 0;

        while (entry && i < 500) {
            //printf("Sending %d\n", entry->seq);
            send(thread->fd, entry->packet, entry->size, 0);
            entry = entry->next;
            i++;
        }

        pthread_mutex_unlock(&thread->queue->mutex);

        usleep(5);
    }

    return NULL;
}

void send_thread_init(SendThread *thread, int fd, Queue *queue)
{
    thread->running = 1;
    thread->fd = fd;
    thread->queue = queue;
    pthread_create(&thread->pthread, NULL, __send, thread);
}

void send_thread_exit(SendThread *thread)
{
    thread->running = 0;
    pthread_cancel(thread->pthread);
}
