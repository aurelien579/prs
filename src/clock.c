#include "clock.h"
#include "tcp.h"
#include "queue.h"

#include <unistd.h>
#include <stdio.h>

static int count = 0;


/*static void *clock_thread(void *_clk)
{
    Clock       *clk = _clk;
    Queue       *queue = &clk->socket->queue;
    QueueEntry  *entry;
    int          should_send = 0;

    while (clk->running) {
        count++;
        pthread_mutex_lock(&queue->mutex);

        entry = queue->top;

        while (entry) {
            if (entry->rtx_usecs > clk->usecs) {
                entry->rtx_usecs -= clk->usecs;
            } else {
                entry->rtx_usecs = 0;
                if (entry->rtx_count > 0)
                    should_send = 1;
            }

            entry = entry->next;
        }

        pthread_mutex_unlock(&queue->mutex);

        if (should_send) {
            tcp_output(clk->socket);
            should_send = 0;
        }

        usleep(clk->usecs);
    }

    return NULL;
}

void clock_init(Clock *clk, Socket *socket, ulong_t usecs)
{
    clk->socket = socket;
    clk->usecs = usecs;
    clk->running = 1;

    pthread_create(&clk->pthread, NULL, clock_thread, clk);
}
*/
