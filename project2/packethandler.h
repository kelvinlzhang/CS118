#ifndef packetHandler
#define packetHandler

#define MAXPACKETSIZE 1024
#define HEADERSIZE 20

int sendPacket(int sockfd, struct sockaddr *dest_addr, socklen_t addrlen, Packet *pkt);

int recvPacket(int sockfd, struct sockaddr *src_addr, socklen_t *addrlen, Packet *pkt);

#endif
