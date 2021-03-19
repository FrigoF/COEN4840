// usyslog.c - Send message to remote UDP SYSLOG server
//
//     Marquette University - COEN 4840
//     Fred J. Frigo
//     18-Mar-2021
//
//

#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#define MAX 256 
#define PORT 514 
#define SA struct sockaddr 

void func(int sockfd, struct sockaddr_in *servaddr) 
{ 
    char syslog_msg[4*MAX];
    char syslog_time[MAX];
    char host_name[MAX];
    char buff[MAX]; 
    int n, status; 
    int pri = (13*8)+6;  // RFC 3164: priority 13 = log audit, priority = 6 info
    time_t current_time;
    char* c_time_string;

    bzero(buff, MAX);
    bzero(syslog_time, MAX);
    bzero(syslog_msg, MAX);

    // Obtain current time. 
    current_time = time(NULL);

    // Convert to local time format. 
    c_time_string = ctime(&current_time);
    strncpy(syslog_time, &c_time_string[4], 15);  // copy only characters needed
    
    // Get hostname
    status = gethostname( host_name, MAX);
    if( status != 0 )
    {
        strcpy(host_name, "UNKNOWN");
    }
   
    printf("Enter the SYSLOG message to send : "); 
    n = 0; 
    while ((buff[n++] = getchar()) != '\n') 
        ;
    buff[n-1]= '\0'; // remove the new line at end of string 

    sprintf(syslog_msg, "<%d> %s %s UDP:test_message %s", pri, syslog_time, host_name, buff );

    sendto(sockfd, (const char *)syslog_msg, strlen(syslog_msg), 
        MSG_DONTWAIT, (const struct sockaddr *) servaddr,  
        sizeof(struct sockaddr_in));     

    printf("SYSLOG message sent: %s\n",syslog_msg);

} 
  
int main(int argc, char *argv[]) 
{ 
    int sockfd; 
    struct sockaddr_in servaddr; 
    struct addrinfo hints, *infoptr; 
    struct addrinfo *p;
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
        puts(host);
    }
    freeaddrinfo(infoptr);

  
    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation to SYSLOG server failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket successfully created..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(host); 
    servaddr.sin_port = htons(PORT); 
  
  
    // function to ask user to input the test message
    func(sockfd, &servaddr); 
  
    // close the socket 
    close(sockfd); 
} 
