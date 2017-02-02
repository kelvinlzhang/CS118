#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#define PORT "8081" // the port users will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold

char *consolePrint(int);
void sendFile(int, char*);
void createHTTPResponse(int, char*, size_t);

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
int main(int argc, char *argv[])
{
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
        p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
        sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
        close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }
    freeaddrinfo(servinfo); // all done with this structure
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    printf("server: waiting for connections...\n");
    while(1) { // main accept() loop
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family,
        get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("server: got connection from %s\n", s);
        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener

            char *filename = consolePrint(new_fd);
			printf("file name: %s\n",filename);







            sendFile(new_fd, filename); // send file to browser
            close(new_fd);
            exit(0);
        }
        close(new_fd); // parent doesn't need this
    }
    return 0;
}

char *consolePrint(int sock)
{
    char request[1024];
    memset(request, 0, 1024);

    if (read(sock, request, 1023) < 0)
        perror("socket: error reading");

    printf("HTTP Request Message:\n%s", request);

    char *filename;
    filename = strtok(request, " "); //extract file name from request
    filename = strtok(NULL, " ");
    filename++;

    if(strlen(filename) <= 0)
        filename = ""; //if no filename, set "" to handle 404

    return filename;
}

void sendFile (int sock, char *filename)
{
    printf("file name: %s\n",filename);

    //declare this elsewhere
    static char *status_404 =
        "HTTP/1.1 404 Not Found\r\n\r\n<h1>Error 404: File Not Found!<h1>";

    //file not found
    if(strcmp(filename, "") == 0)
    {
        send(sock, status_404, strlen(status_404),0);
        perror("server: no file selected");
        return;
    }

    FILE *fd = fopen(filename, "r");

    if(fd == NULL)
    {
        send(sock, status_404, strlen(status_404),0);
        perror("server: cannot locate file in local dir");
        return;
    }

    if(fseek(fd, 0L, SEEK_END))
    {
        int filelen = ftell(fd);
        if (filelen == -1L)
        {
            send(sock, status_404, strlen(status_404),0);
            perror("server: file size error");
            return;
        }

        char *src = malloc(filelen+1);

        fseek(fd, 0L, SEEK_SET);
        size_t srclen = fread(src, 1, filelen, fd);
        if (ferror(fd)) perror("server: error reading file");

        src[srclen] = '\0';

        createHTTPResponse(sock, filename, srclen);
        send(sock, src, srclen, 0);
        printf("file %s send to browser", filename);

       free(src);
    }
    fclose(fd);
}

void createHTTPResponse(int sock, char *filename, size_t filelen)
{
    char buffer[1024];


    char *header = "HTTP/ 1.1 200 OK\r\n"; 
    // printf("%s", header);

    char *connection;


    char store[50];
    time_t now = time(0);
    struct tm timeinfo = *gmtime(&now); //derefenced?
    strftime(store, sizeof store, "%a, %d %b %Y %H:%M:%S %Z", &timeinfo);
    char datebuff[20] = "Date: ";
    strcat(datebuff, store);
    strcat(datebuff, "\r\n");
    // printf("%s",datebuff);


    char *server = "Server: KelvinLauren/1.1\r\n";

    // char *lastmod;
    // struct tm* lastmoded;
    // struct sta

    // char *content_length;
    char contentlen[50];
    sprintf (len, "")

    strcat("Content-Length: ")


    // char *content_type;
    // if (strstr(filename, ".html") !=NULL) 
    // {
    //     contentType = HTML;
    // } else if ((strstr(filename, ".jpg") !=NULL) || (strstr(filename, ".jpeg") !=NULL)
    // {
    //     contentType = JPEG;
    // } else if (strstr(filename, ".gif") !=NULL)
    // {
    //     contentType = GIF;
    // }

    //generate HTTP response


    static char* header = "HTTP/ 1.1";


}
