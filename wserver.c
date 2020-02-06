// wserver.c - Example of TCP/IP Server using sockets (Windows)
//            COEN 4840
//            05-Feb-2020
//
// See:  https://docs.microsoft.com/en-us/windows/win32/winsock/complete-server-code
// and   https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
//
// To compile with Visual Studio:  CL wserver.c 
//

#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "8080"

#define MAX 80

// Function designed for chat between client and server. 
void func(SOCKET sockfd) 
{ 
    char buff[MAX]; 
    int iResult, sResult;	
    // infinite loop for chat 
	do 
	{

        iResult = recv(sockfd, buff, sizeof(buff), 0);
        if (iResult > 0) 
		{
           // print buffer which contains the client contents 
           printf("From client: %s\t To client : ", buff); 
		   scanf("%s", buff);
		   sResult = send( sockfd, buff, sizeof(buff), 0);
		   
           // if msg contains "Exit" then server exit and chat ended. 
           if (strncmp("exit", buff, 4) == 0) 
		   { 
               printf("Server Exit...\n"); 
               break; 
           }
        }		   
    } while(1);  
} 


int __cdecl main(void) 
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo *result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Create a SOCKET for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
	else 
		printf("Server Socket successfully created..\n"); 

    // Setup the TCP listening socket
    iResult = bind( ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	else
		printf("Server Socket successfully binded..\n");

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	else
		printf("Server listening..\n"); 

    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
	else
        printf("server acccept the client...\n"); 
	
    // Function for chatting between client and server 
    func(ClientSocket); 
	
    // No longer need server socket
    closesocket(ListenSocket);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    closesocket(ClientSocket);
    WSACleanup();

    return 0;
}
