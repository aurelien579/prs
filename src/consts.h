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

#ifndef RTT_OFFSET
#define RTT_OFFSET      0
#endif

#ifndef INIT_WINDOW
#define INIT_WINDOW     300
#endif

#define MAX_WINDOW      500
#define MIN_WINDOW      300

#define ALPHA           0.9
#define BETA            0.25*0
#define K               1

#define MIN_SSTHRESH    15
#define INI_SSTRESH     128
#define SSTHRESH_OFFSET 5

#endif
