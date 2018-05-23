#ifndef CLOCK_H
#define CLOCK_H

#include <pthread.h>
#include <sys/time.h>

#include "types.h"

struct clock
{
    Socket     *socket;
    int        running;
    ulong_t    usecs;
    pthread_t  pthread;
};

typedef struct clock Clock;

void clock_init(Clock *clk, Socket *socket, ulong_t usecs);
void clock_stop(Clock *clk);

#endif
