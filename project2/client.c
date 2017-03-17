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

struct PacketInfo{
	int seqNum;
	int recvdFlag;
    char data[MAXPACKETSIZE];
};

void printSendMsg(int ackNum, int retrFlag, int synFlag, int finFlag)
{
	char buffer[70] = "Sending packet";
	char ackNumBuf[10];
	sprintf(ackNumBuf," %d", ackNum);
	strcat(buffer, ackNumBuf);

	if (retrFlag)
	{
		strcat(buffer, " Retransmission");
	}
	if (synFlag)
	{
		strcat(buffer, " SYN");
	}
	if (finFlag)
	{
		strcat(buffer, " FIN");
	}
	printf("%s", buffer);
}

void printRcvMsg(int seqNum)
{
	char buffer[50] = "Receiving packet";
	char seqNumBuf[10];
	sprintf(seqNumBuf, " %d", seqNum);
	strcat(buffer, seqNumBuf);
	printf("%s", buffer);	
}



int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char* hostName;
    int portNum;

    int numbytes;
    char buf[MAXPACKETSIZE];

    if (argc != 4)
    {
        fprintf(stderr,"ERROR args: client < hostname > < portnumber > < filename >\n");
        exit(1);
    }

    char *port = argv[2];


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
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1);
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

    int seq = 0;
    int ack = 0;
    int len = 0;
    int fin = 0;
    int next_seq = 0;

    FILE *fp = fopen("received.data", "w+");

    while (fin != 1)
    {
        numbytes = recvPacket(sockfd, buf, &len, p->ai_addr, &(p->ai_addrlen), &seq, &ack, &fin);
        buf[numbytes] = '\0';
        printf("Received %d bytes with sequence #%d\n", numbytes, seq);

        //receive packet out of window, needs to be ACKed anyways

        //receive packet
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



        numbytes = sendPacket(sockfd, buf, len, p->ai_addr, p->ai_addrlen, seq, seq+len, fin);
        fwrite(buf, 1, len, fp);
        printf("Sent ACK #%d\n", seq+len);
        next_seq += MAXPACKETSIZE;
    }

    fclose(fp);
    close(sockfd);
    return 0;
}
