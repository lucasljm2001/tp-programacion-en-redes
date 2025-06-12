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
#include <signal.h>




int main(int argc, char *argv[]) {
    signal(SIGPIPE, SIG_IGN);
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
    
    

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    if (listenfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    


    int yes=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    fcntl(listenfd, F_SETFL, O_NONBLOCK);

    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
    
    
    serv_addr.sin_port = htons(puerto);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind");
        close(listenfd);
        exit(EXIT_FAILURE);
    }



    if (listen(listenfd, 10) < 0) {
        perror("listen");
        close(listenfd);
        exit(EXIT_FAILURE);
    } 
    
    
    printf("listening port %d\n", puerto);
    

    socklen_t size = sizeof(client_addr);


    fd_set readfds;
    fd_set tempreadfds;
    
    
    fd_set writefds;
    
    
    fd_set exceptionfds;
    
  
    int fdmax = listenfd;

    
    FD_ZERO(&readfds);
    FD_SET(listenfd, &readfds);

    struct timeval tv;


    
    while (1)
    {
        tempreadfds = readfds;

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