// TLS_server.c - a simple TLS Server example  (listens on port 8080)
// 
// Marquette University  COEN 4840
// Fred J. Frigo
// 13-Mar-2021
//
// See https://wiki.openssl.org/index.php/Simple_TLS_Server
//
//  To install OpenSSL see INSTALL at https://www.openssl.org/source/
//  To compile: gcc -Wall -o TLS_server TLS_server.c -lssl -lcrypto -L/usr/local/lib
//
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#define PORT 8080 
#define MAX 256

int create_socket(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("Unable to create socket");
	exit(EXIT_FAILURE); } if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
	perror("Unable to listen");
	exit(EXIT_FAILURE);
    }

    return s;
}

void init_openssl()
{ 
    SSL_load_error_strings();	
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl()
{
    EVP_cleanup();
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	perror("Unable to create SSL context");
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    int sock;
    int status;
    SSL_CTX *ctx;
    struct sockaddr_in addr;
    uint len = sizeof(addr);
    SSL *ssl;
    char incoming_msg[1024] = {0};
    char reply[MAX*3]; 
    char myHost[MAX];
    char myMessage[] = {"Hello from TLS Server"};
    char *username;

    init_openssl();
    ctx = create_context();

    configure_context(ctx);

    sock = create_socket(PORT);  // server will listen on port 8080

    /* Handle connections */
    printf("TLS Server listening on port %d...\n", PORT);
    int client = accept(sock, (struct sockaddr*)&addr, &len);
    if (client < 0) {
         perror("Unable to accept");
         exit(EXIT_FAILURE);
    }

    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, client);

    if (SSL_accept(ssl) <= 0) {
         ERR_print_errors_fp(stderr);
    }
     else {
        printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        // Read incoming message from client
        status = SSL_read(ssl, incoming_msg, sizeof(incoming_msg));

        if(status > 0) {
            // Print message from client		
	    printf("%s\n", incoming_msg);
	    
            // Get username and local host name
            username = getenv("USER");
            gethostname(myHost, MAX);

           // Reply with message to client 
           sprintf( reply, "%s started by %s on %s\n", myMessage, username, myHost);    
           SSL_write(ssl, reply, strlen(reply));
        }

        SSL_shutdown(ssl);
        SSL_free(ssl);
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();
}
