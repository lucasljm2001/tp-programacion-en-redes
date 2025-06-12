#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include "http_utils.h"



int main(int argc, char *argv[]) {
    int listenfd = 0, clientSocket = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    int clientesAtendidos = 0;

    char sendBuff[1025];

    int puerto = 3030;

    if (argc>1)
    {
        puerto = atoi(argv[1]);
    }
    
    

    /* creates an UN-named socket inside the kernel and returns
     * an integer known as socket descriptor
     * This function takes domain/family as its first argument.
     * For Internet family of IPv4 addresses we use AF_INET
     */
    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    

    /* Do not wait to listener socket to be released
     */
    int yes=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));


    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //sudo apt install net-tool
    
    
    serv_addr.sin_port = htons(puerto);


    /* The call to the function "bind()" assigns the details specified
     * in the structure serv_addr' to the socket created in the step above
     */
    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    {
        /* code */
    }
    

    /* The call to the function "listen()" with second argument as 10 specifies
     * maximum number of client connections that server will queue for this listening
     * socket.
     */
    if (listen(listenfd, 10) < 0) {
        perror("listen");
        close(listenfd);
        exit(EXIT_FAILURE);
    }
    {
        /* code */
    }
    
    
    
    printf("listening port %d\n", puerto);
    
    /* In the call to accept(), the server is put to sleep and when for an incoming
     * client request, the three way TCP handshake* is complete, the function accept()
     * wakes up and returns the socket descriptor representing the client socket.
     */
    socklen_t size = sizeof(client_addr);

    PCALLBACK callback = atenderCliente;


    
    while (1)
    {
        int* nuevoSocket = malloc(sizeof(int));

        clientSocket =  accept(listenfd, (struct sockaddr*) &client_addr, &size);

        printf("Acepto conexion\n");

        *nuevoSocket = clientSocket;

        pthread_t clienteThread;
        pthread_attr_t threadAttrs;


        pthread_attr_init(&threadAttrs);
        pthread_attr_setdetachstate(&threadAttrs, PTHREAD_CREATE_DETACHED);
        pthread_attr_setschedpolicy(&threadAttrs, SCHED_FIFO);  
    

        pthread_create(&clienteThread, &threadAttrs,callback, nuevoSocket);


    }

    close(listenfd);
    
}