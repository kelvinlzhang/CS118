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

#define CWND 5120

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void printSendMsg(int seqNum, int wnd, bool retrFlag, bool synFlag, bool finFlag)
{
	char buffer[70] = "Sending packet";
	//convert int to string for seqNum
	char seqNumBuf[10]= " ";
	sprintf(seqNumBuf,"%d", seqNum);
	strcat(buffer,seqNumBuf);

	//convert int to string for wnd
	char wndNumBuf[10] = " ";
	sprintf(wndNumBuf, "%d", wnd);
	strcat(buffer, wndNumBuf);

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

void printRcvMsg(int ackNum)
{
	char buffer[50] = "Receiving packet";
	char ackNumBuf[10] = " ";
	sprintf(ackNumBuf, "%d", ackNum);
	strcat(buffer, ackNumBuf);
	printf("%s", buffer);	

}


int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];

    int numbytes;
    char buf[MAXPACKETSIZE];

    if (argc != 2)
    {
        fprintf(stderr,"ERROR args: server < portnumber >\n");
        exit(1);
    }

    char *port = argv[1];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "ERROR: getaddrinfo");
        return 1;
    }

    //create and bind socket
    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("ERROR: socket creation");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("ERROR: binding socket");
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

    if ((numbytes = recvfrom(sockfd, buf, MAXPACKETSIZE-1 , 0, (struct sockaddr *)&their_addr, &addr_len)) == -1)
    {
        perror("ERROR: did not receive filename");
        exit(1);
    }
    buf[numbytes] = '\0';
    printf("Requested filename: %s\n", buf);

    char *file_data = NULL;
    FILE *fp = fopen(buf, "r");
    size_t file_len;

    if (fp==NULL)
    {
        sendPacket(sockfd, buf, strlen(buf), (struct sockaddr *)&their_addr, addr_len, 0, 0, 1);
        printf("ERROR: file not found!\n");
        exit(1);
    }

    if (fseek(fp, 0L, SEEK_END) == 0)
    {
        long fsize = ftell(fp);
        file_data = malloc(fsize + 1);

        fseek(fp, 0L, SEEK_SET);

        file_len = fread(file_data, 1, fsize, fp);

        //int file_sizeToRead = file_len;
        //while (file_sizeTorRead > MAXPACKETSIZE)
        {
        	sendPacket(sockfd, buf, strlen(buf), (s))
        }
        if (file_len == 0)
        {
            sendPacket(sockfd, buf, strlen(buf), (struct sockaddr *)&their_addr, addr_len, 0, 0, 1);
            free(file_data);
            printf("ERROR: could not read file\n");
            exit(1);
        }

        file_data[file_len] = '\0';
    }

    fclose(fp);

    int seq = 0;
    int ack = 0;
    int len = 0;
    int fin = 0;
    int next_ack = 0;
    int bytes_sent = 0;
    int index = 0;

    while (fin != 1)
    {
        while (index < file_len && bytes_sent < CWND)
        {

        }
    }

    close(sockfd);
    return 0;
}
