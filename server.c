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

int main(int argc, char *argv[]) {
    int listenfd = 0, clientSocket = 0;
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;

    char sendBuff[1025];
    

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


    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff));
    
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //sudo apt install net-tool
    
    // por que funciona igual si le doy una direccion cualquiera? no deberia fallar?
    // serv_addr.sin_addr.s_addr = inet_addr("10.0.2.15");
    
    serv_addr.sin_port = htons(3035);
    // se abre en cualquier puerto, no deberia verlo en el 3030 en la consola?
    // si uso la ip como la forma propuesta si lo abre en el port que le doy
    // sino lo abre en cualquiera

    /* The call to the function "bind()" assigns the details specified
     * in the structure serv_addr' to the socket created in the step above
     */
    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    /* The call to the function "listen()" with second argument as 10 specifies
     * maximum number of client connections that server will queue for this listening
     * socket.
     */
    listen(listenfd, 10);
    
    printf("listening port 3030\n");
    
    /* In the call to accept(), the server is put to sleep and when for an incoming
     * client request, the three way TCP handshake* is complete, the function accept()
     * wakes up and returns the socket descriptor representing the client socket.
     */
    socklen_t size = sizeof(client_addr);
    
    
    clientSocket =  accept(listenfd, (struct sockaddr*) &client_addr, &size);
}