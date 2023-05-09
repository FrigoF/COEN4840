//  TLSsyslog.c - send message to TLS SYSLOG server 
//
//  Marquette University - COEN 4840
//  Fred J. Frigo
//  15-Feb-2020
//  09-May-2023  - Updates for RFC 5424 message format
//
//  To compile: gcc -Wall -o TLSsyslog TLSsyslog.c -lssl -lcrypto -L/usr/local/lib
//  To install OpenSSL see INSTALL at https://www.openssl.org/source/
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#define MAX 256 

#define FAIL -1

void get_syslog_message( char *tls_syslog_msg ) 
{ 
    char rfc3164_syslog_msg[4*MAX];
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
    sprintf(rfc3164_syslog_msg, "<%d>%s %s TLS: RFC3164 message from %s: %s", pri, rfc3164_time, myHost, username, myMessage );

    // RFC 5425 message with NO Structured Data
    time = localtime(&current_time.tv_sec);
    tz_hour = (int)(time->tm_gmtoff/(60*60)); 
    tz_min = (int)(labs(time->tm_gmtoff/60) - labs(tz_hour*60));
    pid = (int)getpid();
    usec = (int)current_time.tv_nsec/1000;
    sprintf(rfc5424_time,"%4.4d%c%2.2d%c%2.2dT%2.2d:%2.2d:%2.2d.%6.6d%+2.2d:%2.2d",
       time->tm_year+1900, '-', time->tm_mon+1, '-', time->tm_mday, time->tm_hour, time->tm_min, time->tm_sec, usec, tz_hour, tz_min ); 
    sprintf(tls_syslog_msg, "<%d>%d %s %s TLS %s %d %c RFC5424 message from %s: %s", 
       pri, version, rfc5424_time, myHost, "TLSsyslog", pid, '-', username,  myMessage );
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
    bzero(&addr, sizeof(addr));
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


int main(int arc, char *argv[])
{

    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char syslog_message[1024] = {0};

    char *hostname, *portnum;
    if ( arc != 3 )
    {
        printf("usage: %s <hostname> <portnum>\n", argv[0]);
        exit(0);
    }
    hostname =  argv[1];
    portnum = argv[2];

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
        SSL_shutdown(ssl);        /* release connection state */
    }
    close(server);         /* close socket */
    SSL_CTX_free(ctx);        /* release context */
    return 0;
}
