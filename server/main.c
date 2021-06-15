/*
** main.c -- a stream socket server
** code from http://beej.us/guide/bgnet/html/
*/

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
#include <sys/stat.h>
#include <signal.h>

#include "server.h"

#define PORT "3490" // the port users will be connecting to

#define BACKLOG 10 // how many pending connections queue will hold

void sigchld_handler(int s)
{
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET)
    {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int get_socket(char *addr, struct addrinfo *hints)
{
    int sockfd;
    struct addrinfo *servinfo, *p;
    int rv;

    if ((rv = getaddrinfo(addr, PORT, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1)
        {
            perror("server: socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)
    {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    return sockfd;
}

int main(void)
{
    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
    struct addrinfo hints;
    struct sockaddr_storage their_addr; // connector's address information
    socklen_t sin_size;
    struct sigaction sa;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int numbytes;

    // https://www.delftstack.com/howto/c/mkdir-in-c/
    int ret = mkdir("data", S_IRWXU);
    if (ret == -1)
    {
        switch (errno)
        {
        case EACCES:
            printf("the parent directory does not allow write");
            exit(EXIT_FAILURE);
        case EEXIST:
            break;
        case ENAMETOOLONG:
            printf("pathname is too long");
            exit(EXIT_FAILURE);
        default:
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("\'data\' directory created to store database\n");
    }
    

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    sockfd = get_socket(NULL, hints);

    hints.ai_flags = NULL;

    printf("server: waiting for datagrams...\n");

    while (1)
    {
        // main loop
        addr_len = sizeof their_addr;
        if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
            (struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

        printf("listener: got packet from %s\n", s);

        // create new socket to send response messages
        new_fd = get_socket(s, &hints);

        // the server only handles one request per datagram.
        // if the user wants to make more requests it should initiate other connections.
        handle_request(new_fd, buf, numbytes, their_addr, addr_len);
    }
    
    close(sock_fd);

    return 0;
}