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
#include <time.h>

#include "packethandler.h"

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int portNum;

    int numbytes;
    char buf[MAXPACKETSIZE];

    if (argc != 4)
    {
        fprintf(stderr,"ERROR args: client < hostname > < portnumber > < filename >\n");
        exit(1);
    }

    char *port = argv[2];
    portNum = atoi(port);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "ERROR: getaddrinfo\n");
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("ERROR: socket creation\n");
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "ERROR: binding socket\n");
        return 2;
    }

    if ((numbytes = sendto(sockfd, argv[3], strlen(argv[3]), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
        perror("ERROR: failed to send filename\n");
        exit(1);
    }

    freeaddrinfo(servinfo);


    Packet pkt = {0, 0, 0, 0, 0, 0, ""};
    int next_seq = 0;

    FILE *fp = fopen("received.data", "w+");

    //SYN
    //need timer

    //ACK (to follow servers SYN ACK)
    //array of pre window pckts (seqNum, ackNum)


    int fin = 0;
    while (fin != 1)
    {
    	numbytes = recvPacket(sockfd, p->ai_addr, &(p->ai_addrlen), &pkt);
        buf[numbytes] = '\0';
        printf("Received %d bytes with sequence #%d\n", numbytes, pkt.seq);

        //save an array of the past 5 pre-window packets

        //receive packet
        //if packet is 
        //expected sequence number is minimum sequence number?
        //if rcvd sequence number > (expected sequence number + CWND)
        	//discard this packet, no ACKs
        //if rcvd sequence number == expected sequence number
        	//write to file
        	//this struct's seqNum += windowsize ????? upcoming packet may not be a full packet
        	//clear out this struct's buffer
        	//set this struct to unreceived
        	//send an ACK packet out
        //if (rcvd sequence number > expected sequence number) and (rcvd sequence number < (expected sequence number + CWND))
        	//check if we already have this number buffered (check the struct's received flag)
        		//send an ACK if so
        	//if not seen before



        //numbytes



        numbytes = sendPacket(sockfd, p->ai_addr, p->ai_addrlen, &pkt);
        fwrite(buf, 1, pkt.len, fp);
        printf("Sent ACK #%d\n", pkt.seq + pkt.len);
        next_seq += MAXPACKETSIZE;
    }

    fclose(fp);
    close(sockfd);
    return 0;
}
