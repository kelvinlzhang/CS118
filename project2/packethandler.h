#include <time.h>

#define MAXPACKETSIZE 1024
#define CWND 5120
#define MAXSEQNUM 30720
#define RTO 500

typedef struct Packet {
    int type;
    int ack;
    int seq;
    struct timespec timer;
    int len;
    char *buf;
} Packet;

/*
Types
0. DATA
1. ACK
2. REQUEST
3. SYN
4. FIN
*/
