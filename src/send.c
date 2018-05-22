#include "send.h"
#include "utils.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

static void *__send(void *_thread)
{
    SendThread *thread = _thread;
    QueueEntry *entry;
    Queue *q = thread->queue;
    int count = 0;

    while (thread->running) {
        pthread_spin_lock(&q->lock);

        count = min(thread->count, queue_readable(q));

        for (int i = 0; i < count; i++) {
            entry = queue_get(q, q->r + i);
            send(thread->fd, entry->packet, entry->size, 0);
        }

        pthread_spin_unlock(&q->lock);

        usleep(thread->sleep);
    }

    return NULL;
}

void send_thread_init(SendThread *thread, int fd, Queue *queue, ulong_t sleep, int count)
{
    thread->running = 1;
    thread->fd = fd;
    thread->queue = queue;
    thread->sleep = sleep;
    thread->count = count;
    
    pthread_create(&thread->pthread, NULL, __send, thread);
}

void send_thread_exit(SendThread *thread)
{
    thread->running = 0;
    pthread_cancel(thread->pthread);
}
