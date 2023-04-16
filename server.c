// server.c - Example of TCP/IP Server using sockets
//            COEN 4840
//            02-Feb-2020 - terminate on message “exit” from client
//            15-Apr-2023 - terminate after receiving any message from client
//
// See:  https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
//
//  To compile: gcc -Wall -o server server.c -lc
//

#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>
#define MAX 256
#define PORT 8080 
#define SA struct sockaddr 
  
// Wait for a message from a client, respond, then exit
void wait_for_client(int sockfd) 
{ 
    char buff[MAX*2]; 
    char myHost[MAX];
    char *username;
    int status;

    // Get username and local host name
    username = getenv("USER");
    gethostname(myHost, MAX);
    
    // Wait for message from a client
    bzero(buff, sizeof(buff)); 
    status = read(sockfd, buff, sizeof(buff)); 
    if (status > 0)    
    {
        printf("From Client : %s\n", buff); 
    }
    
    // Send reply to client
    sprintf( buff, "Message received by %s on %s\n", username, myHost);    
    write(sockfd, buff, sizeof(buff));  
} 

int main() 
{ 
    int sockfd, connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    // socket create and verification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 

    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Binding newly created socket to given IP and verification 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("socket bind failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully binded..\n"); 
  
    // Now server is ready to listen and verification 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Listen failed...\n"); 
        exit(0); 
    } 
    else
        printf("Server listening ...\n"); 
    len = sizeof(cli); 
  
    // Accept the data packet from client and verification 
    connfd = accept(sockfd, (SA*)&cli, (socklen_t *)&len); 
    if (connfd < 0) { 
        printf("server acccept failed...\n"); 
        exit(0); 
    } 
    else
        printf("server acccept the client...\n"); 
  
    // Function to wait for incoming client message 
    wait_for_client(connfd); 
  
    // close the socket 
    close(sockfd); 
} 
