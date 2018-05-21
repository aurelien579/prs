#ifndef CONSTS_H
#define CONSTS_H

#define CLK_US          1000    /* useconds between each clocl tick */

#define PACKET_SIZE     1500
#define HEADER_SIZE     6
#define DATA_SIZE       (PACKET_SIZE - HEADER_SIZE)

#define BUFSIZE         DATA_SIZE

#define INITRTT         1000*25

#define INIT_WINDOW     3
#define MAX_WINDOW      50

#define ALPHA           0.125
#define BETA            0.25
#define K               1

#endif
