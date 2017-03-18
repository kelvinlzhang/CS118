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

//sendto(sockfd, temp, HEADERSIZE + pkt->len, 0, dest_addr, addrlen);
//recvfrom(sockfd, temp, MAXPACKETSIZE, 0, src_addr, addrlen);

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    int numbytes;
    char buf[MAXPACKETSIZE];

    if (argc != 4)
    {
        fprintf(stderr,"ERROR args: client < hostname > < portnumber > < filename >\n");
        exit(1);
    }

    char *port = argv[2];

    char *filename = argv[3];

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

    freeaddrinfo(servinfo);

    FILE *fp = fopen("received.data", "w+");


    // // SYN: type 3, ack 0, seq 0, timer, retrans 0, len 0, buf """
    // Packet syn = { 3, 0, 0, {0,500000}, 0, 0, ""}; //figure out the type
    // if((numbytes = sendto(sockfd, &syn, HEADERSIZE + syn.len, 0, p->ai_addr, &(p->ai_addrlen))) == -1)
    // {
    // 	perror("ERROR: sending syn\n");
    // }

    // //receive server's SYNACK
    // //need timer
    //has an ack of -1?


    // //REQUEST (to respond to server's SYN ACK)
    // // REQUEST: type 2, ack 0, seq 0, timer, retrans 0, len 0, buf """
     // if ((numbytes = sendto(sockfd, argv[3], strlen(argv[3]), 0, p->ai_addr, p->ai_addrlen)) == -1)
   
	   
    Packet request = { 2, 0, 0, {0,500000}, 0, argv[3]}; //figure out type
	if((numbytes = sendto(sockfd, &request, sizeof(request), 0, p->ai_addr, p->ai_addrlen) == -1))
    {
        perror("ERROR: failed to send filename\n");
    }



    //buffer window of Packet pointers 
    int numPkts = 5;
    int bufferFilled = 0;
    Packet** buffer = malloc(numPkts*sizeof(struct Packet*));
    int i = 0;
    while(i < numPkts)
    {
    	buffer[i]  = NULL;
    }
	//type, ack, seq, retrans, len, buf
    Packet pkt = {0, 0, 0, {0,500000}, 0, ""};
    int nextSeq = 0;

    int fin = 0;
    while (fin != 1)
    {
    	printf("before\n");
    	numbytes = recvfrom(sockfd, &pkt, MAXPACKETSIZE, 0, p->ai_addr, &(p->ai_addrlen));
    	printf("made it\n");
    	printf("%s/n", pkt.buf);
        
        buf[numbytes] = '\0';
        printf("Received %d bytes with sequence #%d\n", numbytes, pkt.seq);


        fwrite(buf, sizeof(char), pkt.len, fp);
        //save an array of the past 5 pre-window packets

        // Packet *prev[5] = {};





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

        numbytes = sendto(sockfd, &pkt, sizeof(pkt), 0, p->ai_addr, p->ai_addrlen);
        fwrite(buf, 1, pkt.len, fp);
        printf("Sent ACK #%d\n", pkt.seq + pkt.len);
        nextSeq += MAXPACKETSIZE;
    }

    fclose(fp);
    close(sockfd);
    return 0;
}
