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
    int numbytes;
    char buf[1024];

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
        fprintf(stderr, "ERROR: getaddrinfo");
        return 1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("ERROR: socket creation");
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

}
