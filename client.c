// client.c - Example of TCP/IP Client using sockets
//            COEN 4840
//            05-Feb-2020 - terminate client on message “exit” from server
//            15-Apr-2023 - terminate client after receiving any message from server
//
// See:  https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
//
//  To compile: gcc -Wall -o client client.c -lc
//

#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <arpa/inet.h>
#define MAX 256
#define PORT 8080   // This client connects on port 8080
#define SA struct sockaddr 

// Send message to server and wait for response
void say_hello(int sockfd) 
{ 
    char buff[MAX*3]; 
    char myHost[MAX];
    char myMessage[MAX];
    char *username;
    int status;

    // Get username and local host name
    username = getenv("USER");
    gethostname(myHost, MAX);

    // Get message
    printf("Enter message to send to server: ");
    fgets(myMessage, sizeof(myMessage), stdin);
    myMessage[strlen(myMessage)-1] = 0; // get rid of the '/n' character
    
    // Send message to server
    sprintf( buff, "%s from %s on %s\n", myMessage, username, myHost);    
    write(sockfd, buff, sizeof(buff)); 

    // Read response from server, timeout configured with setsockopt()
    bzero(buff, sizeof(buff)); 
    status = read(sockfd, buff, sizeof(buff)); 
    if (status > 0)    
    {
        printf("From Server : %s\n", buff); 
    }
    else
    {
        printf("No response from Server.\n");
    }
} 
  
int main(int argc, char *argv[]) 
{ 
    int sockfd; 
    struct sockaddr_in servaddr; 
    struct addrinfo hints, *infoptr; 
    struct addrinfo *p;
    struct timeval tv;
    char host[256];

    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    int result = getaddrinfo(argv[1], NULL, &hints, &infoptr);
    if (result) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
        exit(1);
    }

    for (p = infoptr; p != NULL; p = p->ai_next) 
    {
        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST);
    }
    freeaddrinfo(infoptr);

    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 

    // Set timeout for socket read
    tv.tv_sec = 5; // timeout in seconds
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
  
    // assign IP, PORT 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(host); 
    servaddr.sin_port = htons(PORT); 
  
    // connect the client socket to server socket 
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("connection with the server failed...\n"); 
        exit(0); 
    } 
    else
        printf("connected to the server..\n"); 
  
    // send message to server & wait for response
    say_hello(sockfd);
  
    // close the socket 
    close(sockfd); 
} 
