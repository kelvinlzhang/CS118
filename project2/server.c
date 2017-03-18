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
#include "vector.h"

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    /*=================SOCKET===================*/
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
    
    /*=================SYN===================*/
    Packet sent_packet, recv_packet;
    
    while(1) {
        memset((char*)&recv_packet, 0, sizeof(recv_packet));
        if (recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0,(struct sockaddr *) &their_addr, &addr_len) < 0)
            perror("ERROR: receiving SYN\n");
        if (recv_packet.type == 3)
            break;
    }
    
    fprintf("Received packet %d\n" recv_packet.ack);
    memset((char*)&sent_packet, 0, sizeof(sent_packet));
    sent_packet.type = 1; //ACK
    sent_packet.seq = 0;
    sent_packet.ack = 1;
    if(sendto(sockfd, &sent_packet, sizeof(sent_packet), 0, (struct sockaddr *)&their_addr, &addr_len) == -1)
        perror("ERROR: sending SYN-ACK.\n");
    fprintf(stdout, "Sending packet 0 5120 SYN\n");

    struct timeval rto = {0, 50000};
    Packet** timed_packets = malloc(5*sizeof(Packet*));
    
    while(1)
    {
        /*=================REQUEST===================*/
        memset((char*)&recv_packet, 0, sizeof(recv_packet));
        if (recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0, (struct sockaddr *)&their_addr, &addr_len) < 0)
        {
            perror("ERROR: did not receive filename");
            exit(1);
        }
        
        if (recv_packet.type == 2)
            filename = recv_packet.buf;
        
        printf("Requested filename: %s\n", filename);

        char *file_data = NULL;
        FILE *fp = fopen(buf, "r");
        size_t file_len;

        if (fp==NULL)
        {
            printf("ERROR: file not found!\n");
            exit(1);
        }

        fseek(fp, 0L, SEEK_END);
        long file_len= ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        file_data = malloc(fsize + 1);
        fread(file_data, 1, fsize, fp);

        fclose(fp);
        
        /*=================DATA TRANSFER===================*/

        int total_packets = file_len/1024 + (file_len % 1024 != 0);
        int packet = 0;
        int seq = 0;
        int file_pos = 0;

        while (packet < total_packets)
        {
            int num_sent = 0;
            int index = 0;
            while (num_sent <= 4 && file_pos < file_len)
            {
                //construct packet
                memset((char*)&sent_packet, 0, sizeof(sent_packet));
                sent_packet.type = 0;
                sent_packet.seq = seq;
                if (file_len - file_pos < 1024)
                    sent_packet.len = file_len - file_pos;
                else
                    sent_packet.len = 1024;
                memcpy(sent_packet.buf, file_data + file_pos, sent_packet.size);
                
                //dynamically add associated timer
                Packet* timer = malloc(sizeof(struct packet));
                
                
            }

            int num_acked = 0;

            while (num_acked < num_sent)
            {

            }
        }
        
        /*=================FIN===================*/
    }
    
    close(sockfd);
    return 0;
}
