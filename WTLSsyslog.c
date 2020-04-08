//  WTLSsyslog.c - send message to TLS SYSLOG server  (Windows version)
//
//  Marquette University - COEN 4840
//  21-Feb-2020
//
//  Install OpenSSL from https://www.openssl.org/source/
//  To compile: 
//      set LIB=%LIB%;"C:/Program Files/OpenSSL/lib"
//      CL WTLSsyslog.c /I "C:/Program Files/OpenSSL/include" 
//  
//
//  > WTLSsyslog.exe  bloomcounty.eng.mu.edu  6514
//
// Windows 10 OpenSSL installation Notes (see INSTALL file):
// ------------------------------------------------------------------------------
// 1. Download Strawberry Perl - https://www.perl.org/get.html & install in C:\Perl64
// 2. Download NASM - https://www.nasm.us  
// 3. Install OpenSSL - https://www.openssl.org/source/   (use cygwin for gunzip and un-tar )
// 4. Open Visual Studio Native Tools Command window AS Administrator
// 5. Change directory to OPENSSL installation directory
//		set PATH=%PATH%;C:\Perl64\bin
//		perl Configure VC-WIN64A no-asm no-shared
//		nmake
//		nmake test
//		nmake install
//		
//  The "no-shared" option disables the dynamic library engine
//  OpenSSL is installed in C:\Program Files\OpenSSL
//

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>

// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
// Required for OpenSSL 
#pragma comment (lib, "GDI32.lib")
#pragma comment (lib, "CRYPT32.lib")
#pragma comment (lib, "USER32.lib")
#pragma comment (lib, "libcrypto.lib")
#pragma comment (lib, "libssl.lib")


#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "10514"

#define MAX 256




#define FAIL -1

void get_syslog_message( char *tls_syslog_msg ) 
{ 
    char syslog_time[MAX];
    char host_name[MAX];
    char buff[MAX]; 
    int n, status; 
    int pri = (13*8)+6;  // RFC 5424: priority 13 = log audit, priority = 6 info
    time_t current_time;
    char* c_time_string;

    memset(buff, 0, MAX);
    memset(syslog_time, 0, MAX);

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

    sprintf(tls_syslog_msg, "<%d> %s %s TLS:test_message %s", pri, syslog_time, host_name, buff );
    printf("Secure SYSLOG message: %s\n",tls_syslog_msg);

} 

 
int OpenConnection(const char *hostname, int port)
{
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    if ( (host = gethostbyname(hostname)) == NULL )
    {
        perror(hostname);
        abort();
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}

SSL_CTX* InitCTX(void)
{
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = (SSL_METHOD *)TLS_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}


void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}



int __cdecl main(int argc, char **argv) 
{
    WSADATA wsaData;
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char syslog_message[1024] = {0};
	int iResult;

    char *hostname, *portnum;
    if ( argc != 3 )
    {
        printf("usage: %s <hostname> <portnum>\n", argv[0]);
        exit(0);
    }
    hostname =  argv[1];
    portnum = argv[2];
	
    /* Initialize Winsock */
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }
	long res = 1;

    SSL_library_init();
    ctx = InitCTX();
    server = OpenConnection(hostname, atoi(portnum));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */
    if ( SSL_connect(ssl) == FAIL )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else
    {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        ShowCerts(ssl);        /* get any certs */

        get_syslog_message( syslog_message );

        SSL_write(ssl, syslog_message, strlen(syslog_message));   /* encrypt & send message */
		SSL_shutdown(ssl);  /* shut down TLS connection */
    }
    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
	WSACleanup();             /* Winsock */
    return 0;
}