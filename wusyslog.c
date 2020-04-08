// wusyslog.c - Send message to remote UDP SYSLOG server (Windows)
//            COEN 4840
//            15-Feb-2020
//
//
//
// To compile with Visual Studio:  CL wusyslog.c 
//
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")


#define DEFAULT_BUFLEN 512
#define PORT 514

#define MAX 256

void func(SOCKET sockfd, struct sockaddr_in *servaddr ) 
{ 

    char syslog_msg[MAX];
    char syslog_time[MAX];
    char host_name[MAX];
    char buff[MAX]; 
    int n, host_name_len, status; 
    int pri = (13*8)+6;  // RFC 3164: priority 13 = log audit, priority = 6 info
    time_t current_time;
    char* c_time_string;

    memset(buff, 0, MAX);
    memset(syslog_time, 0, MAX);
    memset(syslog_msg, 0, MAX);


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

    // Enter Message to send to remote SYSLOG
    printf("Enter the SYSLOG message to send : "); 
    n = 0; 
    while ((buff[n++] = getchar()) != '\n') 
        ;
    buff[n-1]= '\0'; // remove the new line at end of string 
	
    sprintf(syslog_msg, "<%d> %s %s UDP: Windows test_message %s", pri, syslog_time, host_name, buff );

    sendto(sockfd, (const char *)syslog_msg, strlen(syslog_msg), 
        0, (const struct sockaddr *) servaddr,  sizeof(struct sockaddr_in));     

    printf("SYSLOG message sent: %s\n",syslog_msg);
	
} 
  

int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SOCKET SendSocket = INVALID_SOCKET;
    struct sockaddr_in RecvAddr;
    struct addrinfo hints, 
                    *infoptr = NULL,
                    *p = NULL;
    int iResult;
    char host[256];
    
    // Validate the parameters
    if (argc != 2) {
        printf("usage: %s server-name\n", argv[0]);
        return 1;
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;


    iResult = getaddrinfo(argv[1], NULL, &hints, &infoptr);
    if (iResult) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(iResult));
        exit(1);
    }

    for (p = infoptr; p != NULL; p = p->ai_next) 
    {
        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof (host), NULL, 0, NI_NUMERICHOST);
    }
    freeaddrinfo(infoptr);
  
    // socket create for UDP
    SendSocket = socket(AF_INET, SOCK_DGRAM, 0); 
    if (SendSocket == -1) { 
        printf("socket creation for SYSLOG server failed...\n"); 
        exit(0); 
    } 
    else
        printf("Socket created for remote SYSLOG server: %s \n", host); 
	
    // assign IP, PORT 
    memset(&RecvAddr, 0, sizeof(RecvAddr)); 
    RecvAddr.sin_family = AF_INET; 
    RecvAddr.sin_addr.s_addr = inet_addr(host); 
    RecvAddr.sin_port = htons(PORT); 
  
    // function to ask user to input the test message
    func(SendSocket, &RecvAddr); 

    // shutdown the connection since no more data will be sent
    iResult = closesocket(SendSocket);
    if (iResult == SOCKET_ERROR) {
        wprintf(L"closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    //---------------------------------------------
    // Clean up and quit.
    wprintf(L"Exiting.\n");
    WSACleanup();
    return 0;

}
