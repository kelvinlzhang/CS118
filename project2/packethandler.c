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

int sendPacket(int sockfd, char *buf, int len, struct sockaddr *dest_addr, socklen_t addrlen, int seq, int ack, int fin)
{
    char* temp = malloc(HEADERSIZE + len);
    bzero(temp, HEADERSIZE + len);

    memcpy(temp, &seq, sizeof(int));
    memcpy(temp+4, &ack, sizeof(int));
    memcpy(temp+8, &fin, sizeof(int));
    memcpy(temp+12, &len, sizeof(int));
    memcpy(temp+16, buf, len);

    int byteSent = sendto(sockfd, temp, HEADERSIZE + len, 0, dest_addr, addrlen);

    free(temp);

    return byteSent;
}


int recvPacket(int sockfd, char *buf, int *len, struct sockaddr *src_addr, socklen_t *addrlen, int *seq, int *ack, int *fin)
{
    char temp[MAXPACKETSIZE];
    bzero(buf, MAXPACKETSIZE);

    int byteRecv = recvfrom(sockfd, temp, HEADERSIZE + MAXPACKETSIZE, 0, src_addr, addrlen);

    memcpy(seq, temp, sizeof(int));
    memcpy(ack, temp+4, sizeof(int));
    memcpy(fin, temp+8, sizeof(int));
    memcpy(len, temp+12, sizeof(int));
    memcpy(buf, temp+16, MAXPACKETSIZE-HEADERSIZE);

    return byteRecv;
}
