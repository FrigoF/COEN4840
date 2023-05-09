// usyslog.c - Send message to remote UDP SYSLOG server
//
//     Marquette University - COEN 4840
//     Fred J. Frigo
//     18-Mar-2021
//
//     09-May-2023 - Updates for RFC 5424 format
//
// To compile: gcc -Wall -o usyslog usyslog.c
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
    char rfc3164_syslog_msg[4*MAX];
    char rfc5424_syslog_msg[4*MAX];
    char *syslog_msg;
    char rfc3164_time[MAX];
    char rfc5424_time[MAX];
    char myHost[MAX];
    char myMessage[MAX];
    char *username;
    int pri = (13*8)+6;  // RFC 3164: priority 13 = log audit, priority = 6 infoa
    int version = 1;     // RFC 5424: version
    struct timespec current_time;
    int pid, usec;
    struct tm *time;
    int tz_hour, tz_min; 
    char* c_time_string;

    // Get username and local host name
    username = getenv("USER");
    gethostname(myHost, MAX);

    // Get message
    printf("Enter message to send to server: ");
    fgets(myMessage, sizeof(myMessage), stdin);
    myMessage[strlen(myMessage)-1] = 0; // get rid of the '/n' character

    // Obtain current time. 
    clock_gettime( CLOCK_REALTIME, &current_time);

    // RFC 3164 message 
    c_time_string = ctime(&current_time.tv_sec);
    strncpy(rfc3164_time, &c_time_string[4], 15);  // copy only characters needed
    sprintf(rfc3164_syslog_msg, "<%d>%s %s UDP: RFC3164 message from %s: %s", pri, rfc3164_time, myHost, username, myMessage );

    // RFC 5425 message with NO Structured Data
    time = localtime(&current_time.tv_sec);
    tz_hour = (int)(time->tm_gmtoff/(60*60)); 
    tz_min = (int)(labs(time->tm_gmtoff/60) - labs(tz_hour*60));
    pid = (int)getpid();
    usec = (int)current_time.tv_nsec/1000;
    sprintf(rfc5424_time,"%4.4d%c%2.2d%c%2.2dT%2.2d:%2.2d:%2.2d.%6.6d%+2.2d:%2.2d",
       time->tm_year+1900, '-', time->tm_mon+1, '-', time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec, usec, tz_hour, tz_min ); 
    sprintf(rfc5424_syslog_msg, "<%d>%d %s %s UDP %s %d %c RFC5424 message from %s: %s", 
       pri, version, rfc5424_time, myHost, "usyslog", pid, '-', username,  myMessage );

    // Send Syslog mesage
    //syslog_msg = rfc3164_syslog_msg;
    syslog_msg = rfc5424_syslog_msg;
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
