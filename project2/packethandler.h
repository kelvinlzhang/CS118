#ifndef packetHandler
#define packetHandler

#define MAXPACKETSIZE 1024
#define HEADERSIZE 16

int sendPacket(int sockfd, char *buf, size_t len, struct sockaddr *dest_addr, socklen_t addrlen, int seq, int ack, int fin);

int recvPacket(int sockfd, char *buf, size_t *len, struct sockaddr *src_addr, socklen_t *addrlen, int *seq, int *ack, int *fin);

#endif
