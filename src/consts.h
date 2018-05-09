#ifndef CONSTS_H
#define CONSTS_H

#define CLK_US          1000    /* useconds between each clocl tick */

#define PACKET_SIZE     1500
#define HEADER_SIZE     6
#define DATA_SIZE       (PACKET_SIZE - HEADER_SIZE)

#define BUFSIZE         DATA_SIZE

#define INITRTT         1000*1000

#define WINDOW_SIZE     3

#endif
