#ifndef packetHandler
#define packetHandler

#define MAXPACKETSIZE 1024
#define HEADERSIZE 20

typedef struct Packet {
    int ack;
    int seq;
    int retrans;
    int syn;
    int fin;
    int len;
    char *buf;
} Packet;

int sendPacket(int sockfd, struct sockaddr *dest_addr, socklen_t addrlen, Packet *pkt);

int recvPacket(int sockfd, struct sockaddr *src_addr, socklen_t *addrlen, Packet *pkt);

#endif
