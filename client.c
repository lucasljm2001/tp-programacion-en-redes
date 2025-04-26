#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <netdb.h>


int main(int argc, char * argv[]) {

    int clientfd = 0;
    struct sockaddr_in client_addr;


    int clientesAtendidos = 0;

    char sendBuff[1025];
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    

    /* Do not wait to listener socket to be released
     */
    int yes=1;
    setsockopt(clientfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));


    memset(&client_addr, '0', sizeof(client_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr("10.0.2.15");//htonl(INADDR_ANY); 
    
    
    client_addr.sin_port = htons(3030);

    socklen_t size = sizeof(client_addr);

    int ret = connect(clientfd, (struct sockaddr*) &client_addr, size);
    
    if (ret==-1)
    {
        close(clientfd);
        exit(1);
    }

    char message[] = "PING";

    int cxBytes = send(clientfd, message, strlen(message), 0);

    char buffer[5];

    int req = recv(clientfd, buffer, sizeof(buffer)-1, 0);

    buffer[req] = '\0';

    printf("Respuesta recibida: %s\n", buffer);

    close(clientfd);
    

}

