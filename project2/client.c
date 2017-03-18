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

    struct timeval rto = {0, 50000};

    
    
    
    // SYN: type 3 , ack 0, seq 0, timer 0, len 0, buf ""
    Packet syn = { 3, 0, 0, {0,500000}, 0, ""}; //figure out the type
    if((numbytes = sendto(sockfd, &syn, sizeof(syn), 0, p->ai_addr, p->ai_addrlen)) == -1)
    {
    	perror("ERROR: sending syn\n");
    }
    printf("Sending packet SYN\n");

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &rto, sizeof(rto)) < 0)
        perror("ERROR: socket timeout");
    
    Packet recvd, request;

    while(1) {
        if(recvfrom(sockfd, &recvd, sizeof(recvd), 0, p->ai_addr, &(p->ai_addrlen)) == -1)
            perror("ERROR: failed to receive SYN ACK\n");
        else
        {
            if(recvd.type == 1 && recvd.ack == 1)
            {
                printf("Receiving packet SYNACK\n");
                break;
            }
        }
    
        if((numbytes = sendto(sockfd, &syn, sizeof(syn), 0, p->ai_addr, p->ai_addrlen)) == -1)
        {
            perror("ERROR: sending syn\n");
        }
        printf("Sending packet SYN Retransmission\n");
        
        //SYN ACK should be ACK and ack 1
 	}
    
    
    struct timeval data_wait = {1000000, 0};
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &data_wait, sizeof(data_wait)) < 0)
        perror("ERROR: socket timeout");

    
    //REQUEST (to respond to server's SYN ACK)
    //REQUEST:type 2 , ack, seq, timer, len, buf
    
    
    filename = argv[3];
    request.type = 2; //figure out type
    request.seq = 0;
    request.ack = 0;
    request.len = strlen(filename);
    strcpy(request.buf, filename);
    
	if((numbytes = sendto(sockfd, &request, sizeof(request), 0, p->ai_addr, p->ai_addrlen) == -1))
    {
        perror("ERROR: failed to send filename \n");
    }
    printf("Sending request packet for filename: %s\n", filename);

    //buffer window of Packet pointers 
    int numPkts = 5;
    int pktBufCount = 0;
    Packet** pktBuf = malloc(numPkts*sizeof(Packet*));
    int i = 0;
    while(i < numPkts)
    {
    	pktBuf[i]  = NULL;
    	i++;
    }
	//type 1 , ack, seq, timer, len, buf
    Packet ack = {1, 0, 0, {0,500000}, 0, ""};
    int nextSeq = 0;

    //int fin = 0;

    int expectedSeq = 0;

    //connection open
    while (1)
    {
    	// printf("First\n");
    	//recieve packet
    	memset((char*)&recvd, 0, sizeof(recvd));
    	if(recvfrom(sockfd, &recvd, MAXPACKETSIZE, 0, p->ai_addr, &(p->ai_addrlen)) == -1)
    	{
    		perror("ERROR: failed to receive packet\n");
    	}
    	// printf("Second\n");
    	//SYNACK type 1 just found, send request with filename
    	if((recvd.type == 1) && (recvd.ack == 0))
    	{
    		printf("Attempting to retransmit filename request\n");
    		if(sendto(sockfd, &request, sizeof(request), 0, p->ai_addr, p->ai_addrlen) == -1)
    		{
    			perror("ERROR: failed to resend filename\n");
    		}
    	}
    	// printf("Third\n");
    	//FIN received, type 4
    	if (recvd.type == 4)
    	{
    		printf("Receiving packet FIN\n");
    		break;
    	}
    	// printf("Fourth\n");
    	//DATA received, type 0
    	if(recvd.type == 0)
    	{
    		printf("Packet #%d\n", recvd.seq);
    		int min = expectedSeq;
    		int max = expectedSeq + 4*MAXPACKETSIZE;
    		int prevMin;
    		int prevMax;
    		if ((expectedSeq - 5*MAXPACKETSIZE) < 0)
    		{
    			prevMin = 0;
    		} else
    		{
    			prevMin = expectedSeq - 5*MAXPACKETSIZE;
    		}
    		if ((expectedSeq - MAXPACKETSIZE) < 0)
    		{
    			prevMax = 4*MAXPACKETSIZE;
    		} else
    		{
    			prevMax = expectedSeq - MAXPACKETSIZE;
    		}

    		//in order packet received
    		if(recvd.seq == expectedSeq)
    		{
    			fwrite(recvd.buf, 1, recvd.len, fp);
    			ack.ack = recvd.seq + recvd.len;
    			expectedSeq += MAXPACKETSIZE;
    		
    			int i = 0;
    			while(i < 5)
    			{
    				if (pktBuf[i]!= NULL)
    				{
    					if(pktBuf[i]->seq == expectedSeq)
    					{
    						fwrite(pktBuf[i]->buf, 1, pktBuf[i]->len, fp);
    						pktBuf[i] = NULL;
    						expectedSeq++;
    						i = -1;
    					}
    				}
    				i++;
    			}
    			Packet **nxtPktBuf = malloc(numPkts*sizeof(Packet*));
    			pktBufCount = 0;
    			int j;
    			for (j =0; j < 5; j++)
    			{
    				nxtPktBuf[j] = NULL;
    			} 
    			for(j = 0; j < 5; j++)
    			{
    				if(pktBuf[j] != NULL)
    				{
    					nxtPktBuf[pktBufCount] = pktBuf[i];
    					pktBufCount++;
    				} 
    			}
    			pktBuf = nxtPktBuf;
    		} else if ((recvd.seq > min) && (recvd.seq <= max)) //out of order
    		{
    			if (pktBufCount < numPkts)
    			{
    				Packet *toInsert = malloc(sizeof(Packet));
    				toInsert->seq = recvd.seq;
    				toInsert->len = recvd.len;
    				memcpy(toInsert->buf, recvd.buf, recvd.len);
    				pktBuf[pktBufCount] = toInsert;
    				pktBufCount++;
    				ack.ack = recvd.seq + recvd.len;
    			} 
    		} else if(recvd.seq < expectedSeq) //previous packet to ACK
    		{
    			ack.ack = recvd.seq;
    		}
    		else{ //ignore
    			continue; 
    		}
    		//send ack
    		if (sendto(sockfd, &ack, sizeof(ack), 0, p->ai_addr, p->ai_addrlen) == -1)
    		{
    			perror("ERROR: failed to send ACK\n");
    		}
    		printf("Sending ACK#%d\n", ack.ack);
    	}


        
    }

    //FINACK, type 1, ack, seq, timer, len, buf
    Packet finAck = {1, recvd.seq, 0, {0,500000}, 0, ""};

    if( sendto(sockfd, &finAck, sizeof(finAck), 0, p->ai_addr, p->ai_addrlen) == -1)
    {
    	perror("ERROR: failed to send FIN ACK\n");
    }
    printf("Sending FINACK#%d\n", finAck.ack);
    printf("Connection closed\n");


    fclose(fp);
    close(sockfd);
    return 0;
}
