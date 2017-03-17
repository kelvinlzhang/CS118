#include <time.h>

#define MAXPACKETSIZE 1024

typedef struct Packet {
    int type; //purpose of packet
    int ack;
    int seq;
    struct timespec timer;
    int len;
    char *buf;
} Packet;

int sendPacket(int sockfd, struct sockaddr *dest_addr, socklen_t addrlen, Packet *pkt);

int recvPacket(int sockfd, struct sockaddr *src_addr, socklen_t *addrlen, Packet *pkt);

