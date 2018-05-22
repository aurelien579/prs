#ifndef CONSTS_H
#define CONSTS_H

#ifndef CLK_US
#define CLK_US          1000    /* useconds between each clocl tick */
#endif

#define PACKET_SIZE     1500
#define HEADER_SIZE     6
#define DATA_SIZE       (PACKET_SIZE - HEADER_SIZE)

#define BUFSIZE         DATA_SIZE

#ifndef INITRTT
#define INITRTT         1000*50
#endif

#ifndef INIT_WINDOW
#define INIT_WINDOW     10
#endif

#define MAX_WINDOW      50
#define MIN_WINDOW      10

#define ALPHA           0.125*7
#define BETA            0.25*3
#define K               1

#define MIN_SSTHRESH    7
#define INI_SSTRESH     128
#define SSTHRESH_OFFSET 5

#endif
