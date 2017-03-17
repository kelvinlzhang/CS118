#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "packethandler.h"


//char *buf, int len,  int seq, int ack, int fin
int sendPacket(int sockfd, struct sockaddr *dest_addr, socklen_t addrlen, Packet *pkt)
{
    char* temp = malloc(HEADERSIZE + pkt->len);
    bzero(temp, HEADERSIZE + pkt->len);

    memcpy(temp, pkt->seqNum, sizeof(int));
    memcpy(temp+4, pkt->ackNum, sizeof(int));
    memcpy(temp+8, pkt->fin, sizeof(int));
    memcpy(temp+12, pkt->len, sizeof(int));
    memcpy(temp+16, pkt->syn, sizeof(int));
    memcpy(temp+20, pkt->buf, pkt->len);

    int byteSent = sendto(sockfd, temp, HEADERSIZE + pkt->len, 0, dest_addr, addrlen);

    free(temp);

    return byteSent;
}


int recvPacket(int sockfd, struct sockaddr *src_addr, socklen_t *addrlen, Packet *pkt)
{
    char temp[MAXPACKETSIZE];
    bzero(pkt->buf, MAXPACKETSIZE);

    int byteRecv = recvfrom(sockfd, temp, HEADERSIZE + MAXPACKETSIZE, 0, src_addr, addrlen);

    memcpy(pkt->seq, temp, sizeof(int));
    memcpy(pkt->ack, temp+4, sizeof(int));
    memcpy(pkt->fin, temp+8, sizeof(int));
    memcpy(pkt->len, temp+12, sizeof(int));
    memcpy(pkt->syn, temp+16, sizeof(int));
    memcpy(pkt->buf, temp+20, MAXPACKETSIZE-HEADERSIZE);

    return byteRecv;
}


