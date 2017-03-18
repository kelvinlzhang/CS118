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
    
    fprintf(stdout, "Received packet %d\n", recv_packet.ack);
    memset((char*)&sent_packet, 0, sizeof(sent_packet));
    sent_packet.type = 1; //ACK
    sent_packet.seq = 0;
    sent_packet.ack = 1;
    if(sendto(sockfd, &sent_packet, sizeof(sent_packet), 0, (struct sockaddr *)&their_addr, addr_len) == -1)
        perror("ERROR: sending SYN-ACK.\n");
    fprintf(stdout, "Sending packet 0 5120 SYN\n");

    struct timeval rto = {0, 50000};
    Packet** timed_packets = malloc(5*sizeof(Packet*));
    
    while(1)
    {
        /*=================REQUEST===================*/
        char* filename;
        
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

        if (fp==NULL)
        {
            printf("ERROR: file not found!\n");
            exit(1);
        }

        fseek(fp, 0L, SEEK_END);
        long file_len= ftell(fp);
        fseek(fp, 0L, SEEK_SET);

        file_data = malloc(file_len + 1);
        fread(file_data, 1, file_len, fp);

        fclose(fp);
        
        /*=================DATA TRANSFER===================*/

        int total_packets = file_len/1024 + (file_len % 1024 != 0);
        int packet = 0;
        int seq = 0;
        int file_pos = 0;
        
        vector waiting_packets;
        vector_init(&waiting_packets);
        
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
                memcpy(sent_packet.buf, file_data + file_pos, sent_packet.len);
                
                //dynamically allocate associated timer
                Packet* packet_timer = malloc(sizeof(Packet));
                packet_timer->type = 0;
                packet_timer->seq = seq;
                packet_timer->len = sent_packet.len;
                memcpy(packet_timer->buf, file_data + file_pos, packet_timer->len);
                clock_gettime(CLOCK_REALTIME, &(packet_timer->timer));
                timed_packets[index] = packet_timer;
                
                vector_add(&waiting_packets, &sent_packet);
                
                if(sendto(sockfd, &sent_packet, sizeof(sent_packet), 0, (struct sockaddr *)&their_addr, addr_len) == -1)
                    perror("ERROR: sending data packet.\n");
                else
                    fprintf(stdout, "Sending packet %d 5120\n", sent_packet.seq);
                
                packet++;
                num_sent++;
                index++;
                file_pos += 1024;
                seq = seq % 30720 + sent_packet.len;
            }

            int num_acked = 0;

            while (num_acked < num_sent)
            {
                struct timespec master_timer;
                clock_gettime(CLOCK_REALTIME, &master_timer); //timestamp of present time
                int i;
                for (i = 0; i < 5; i++)
                {
                    if (timed_packets[i] != NULL)
                    {
                        if (master_timer.tv_sec - timed_packets[i]->timer.tv_sec >= 0.5)
                        { //compare against timestamps of unacked packets
                            if (sendto(sockfd, timed_packets[i], sizeof(*timed_packets[i]), 0, (struct sockaddr *)&their_addr, addr_len) == -1)
                                perror("ERROR: retransmitting packet\n");
                            else
                                fprintf(stdout, "Sending packet %d 5120 Retransmission\n", timed_packets[i]->seq);
                        }
                        timed_packets[i]->timer = master_timer; //get time to compare on next cycle
                    }
                }
                
                if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &rto, sizeof(rto)) < 0)
                    perror("ERROR: socket timeout");
                memset((char*)&recv_packet, 0, sizeof(recv_packet));
                if (recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0,(struct sockaddr *) &their_addr, &addr_len) < 0)
                    fprintf(stdout,"ERROR: receiving packet\n");
                
                if (recv_packet.type == 1)
                {
                    fprintf(stdout, "Receiving packet %d\n", recv_packet.ack);
                    
                    Packet* first_packet = (Packet *) vector_get(&waiting_packets, 0);
                    int first_expected_ack = first_packet->seq +first_packet->len;
                    
                    if (first_expected_ack == recv_packet.ack)
                        ((Packet *)vector_get(&waiting_packets, 0))->ack = -1;
                    
                    while (first_expected_ack == recv_packet.ack && first_packet->ack == -1)
                    {
                        vector_delete(&waiting_packets, 0);
                        num_sent--;
                    }
                    
                    int i;
                    for (i = 0; i < vector_total(&waiting_packets); i++)
                    {
                        if (((Packet *)vector_get(&waiting_packets, i))->seq + ((Packet *)vector_get(&waiting_packets, i))->len == recv_packet.ack)
                            ((Packet *)vector_get(&waiting_packets, i))->ack = -1;
                    }
                    
                    for (i = 0; i < 5; i++)
                    {
                        if (timed_packets[i]->seq + timed_packets[i]->len == recv_packet.ack)
                            timed_packets[i] = NULL;
                    }
                }
            }
        }
        
        /*=================FIN===================*/
        
        memset((char*)&sent_packet, 0, sizeof(sent_packet));
        sent_packet.type = 4;
        sent_packet.seq = seq;
        fprintf(stdout, "Sending packet %d 5120 FIN\n", sent_packet.seq);
        if(sendto(sockfd, &sent_packet, sizeof(sent_packet), 0, (struct sockaddr *)&their_addr, addr_len) == -1)
            perror("ERROR: sending FIN packet.\n");
        
        while(1) {
            memset((char*)&recv_packet, 0, sizeof(recv_packet));
            if (recvfrom(sockfd, &recv_packet, sizeof(recv_packet), 0,(struct sockaddr *) &their_addr, &addr_len) < 0)
                perror("ERROR: receiving FIN-ACK\n");
            if (recv_packet.type == 1)
            {
                printf("Receiving packet %d\n", recv_packet.ack);
                break;
            }
        }
        
        sleep(500);
        fprintf(stdout, "Connection closed\n");
        break;
    }
    
    close(sockfd);
    return 0;
}
