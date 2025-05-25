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
#include <fcntl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include "http_utils.h"



int main(int argc, char *argv[]) {
    int listenfd = 0, clientSocket = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    int clientesAtendidos = 0;

    char sendBuff[1025];
    char buffer[256];

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
    

    /* Do not wait to listener socket to be released
     */
    int yes=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    fcntl(listenfd, F_SETFL, O_NONBLOCK);

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //sudo apt install net-tool
    
    
    serv_addr.sin_port = htons(puerto);


    /* The call to the function "bind()" assigns the details specified
     * in the structure serv_addr' to the socket created in the step above
     */
    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    /* The call to the function "listen()" with second argument as 10 specifies
     * maximum number of client connections that server will queue for this listening
     * socket.
     */
    listen(listenfd, 10);
    
    printf("listening port %d\n", puerto);
    
    /* In the call to accept(), the server is put to sleep and when for an incoming
     * client request, the three way TCP handshake* is complete, the function accept()
     * wakes up and returns the socket descriptor representing the client socket.
     */
    socklen_t size = sizeof(client_addr);

    PCALLBACK callback = atenderCliente;

    fd_set readfds;
    fd_set tempreadfds;
    
    
    // A client socket is ready for writing when its connection is fully established
    fd_set writefds;
    
    
    // "Exceptional conditions” does not mean errors—errors are reported immediately when
    // an erroneous system call is executed, and do not constitute a state of the descriptor.
    // Rather, they include conditions such as the presence of an urgent message on a socket.
    // See Sockets, for information on urgent messages.
    fd_set exceptionfds;
    
  
    int fdmax = listenfd;

    
    FD_ZERO(&readfds);
    FD_SET(listenfd, &readfds);

    struct timeval tv;


    
    while (1)
    {
        tempreadfds = readfds;

        // specifies the minimum interval that select() should block
        // waiting for a file descriptor to become ready.
        // If both fields of the timeval structure are zero, then select() returns immediately.
        // If timeout is NULL (no timeout), select() can block indefinitely.
        tv.tv_sec = 2;
        tv.tv_usec = 500000;
        
        
        printf("Descriptores activos antes de select: ");
        for (int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &readfds)) {
                printf("%d ", i);
            }
        }
        printf("\n");
        int ret = select(fdmax + 1, &tempreadfds, NULL, NULL, &tv);
        
        
            

        if(ret == -1) {
            printf("El malo es %d\n", fdmax);
            perror("select");
        }
        
        
        for(int fd = 0; fd <= fdmax; fd++) {
            
            
            if(FD_ISSET(fd, &tempreadfds)) {
                
                
                
                
                if(fd == listenfd) {
                    
                    struct sockaddr_in sockaddClient;
                    socklen_t sockaddClientLength = sizeof(sockaddClient);
                    
                    int socketNewCx = accept(listenfd, (struct sockaddr *) &sockaddClient, &sockaddClientLength);
                    
                    if (socketNewCx == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // No hay conexión disponible en modo no bloqueante
                            continue;
                        } else {
                            perror("accept");
                            continue;
                        }
                    }


                    FD_SET(socketNewCx, &readfds);
                    
                    if(socketNewCx > fdmax) {
                    
                        fdmax = socketNewCx;
                    }
                    
                    
                } else {
                    memset(&buffer, 0, 256);

                    ssize_t readed = atenderClienteDesdeSelect(fd);

                    
                    if (fd > 0) {
                        close(fd);
                        FD_CLR(fd, &readfds);
                    }

                    // Actualizás fdmax de forma segura
                    if (fd == fdmax) {
                        fdmax = listenfd; // valor por defecto mínimo
                        for (int i = 0; i < FD_SETSIZE; i++) {
                            if (FD_ISSET(i, &readfds)) {
                                if (i > fdmax) fdmax = i;
                            }
                        }
                    }
                    
                }
            }
            
        }
        printf("\n");

    }

    close(listenfd);
    
}