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

typedef void * (* PCALLBACK) (void *);

typedef struct {
    char method[16];      
    char resource[1024];  
    char protocol[16];    
    char headers[10][256]; 
    int header_count;
} HTTPRequest;

HTTPRequest parse_request(const char *buffer) {
    HTTPRequest req = {0};
    char *buffer_copy = strdup(buffer); // Crear copia modificable
    if (!buffer_copy) return req;

    char *saveptr; // Puntero para estado de strtok_r
    char *line = strtok_r(buffer_copy, "\r\n", &saveptr);

    if (line) {
        sscanf(line, "%15s %1023s %15s", req.method, req.resource, req.protocol);
    }

    // Parsear headers
    while ((line = strtok_r(NULL, "\r\n", &saveptr)) && req.header_count < 10) {
        if (strlen(line) == 0) break;
        strncpy(req.headers[req.header_count++], line, 255);
    }

    free(buffer_copy); // Liberar la copia
    return req;
}

void* atenderCliente(void* args){

    int clientSocket = *((int *) args);

    char buffer[2048];
    char response[1024];
    struct stat fileStats;
    
    // buscar imagen en filesystem
    // get image props
    int imagefd = open("./hello.png", O_RDONLY);
    fstat(imagefd, &fileStats);
    // printf("File size : %ld\n", fileStats.st_size);
    
    
        

    // receive request
    // printf("Connection established on socket %d\n", clientSocket);
    memset(buffer, 0, sizeof(buffer));
    int totalRead = 0;
    int nbytes = 0;

    do {
    // Limitar lectura para dejar espacio para el '\0'
    nbytes = recv(clientSocket, buffer + totalRead, sizeof(buffer) - totalRead - 1, 0);
        if (nbytes > 0) {
            totalRead += nbytes;
            buffer[totalRead] = '\0'; // Asegurar terminación
            if (strstr(buffer, "\r\n\r\n")) break;
        }
    } while (nbytes > 0 && totalRead < sizeof(buffer) - 1); // Evitar overflow

    HTTPRequest request = parse_request(buffer);

    printf("Método: %s\n", request.method);

    printf("Recurso: %s\n", request.resource);
    printf("Protocolo: %s\n", request.protocol);
    
    char *responseHeaders;

    if ( strcmp(request.method,"GET") == 0 && strcmp(request.resource,"/imagen.jpg") == 0)
    {
        // build response
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n";    
    } else
    {
        responseHeaders = "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n";    
        imagefd = open("./error.png", O_RDONLY);
        fstat(imagefd, &fileStats);
    }
    
    memset(response, 0, 1024);
    sprintf(response, responseHeaders, fileStats.st_size);
    


    
    // send response
    // headers first
    int sent = send(clientSocket, response, strlen(response), 0);
    
    
    // payload next
    off_t sbytes = 0;
    int ret = sendfile(clientSocket, imagefd, NULL, fileStats.st_size);
    
    if(ret < 0) {
        
        // fprintf(stderr, strerror(errno));
        exit(1);
        
    }
    
    printf("sending file... %lld\n", ret);  
    printf("--------------------------\n"); 
    close(clientSocket);
    close(imagefd);

    free(args);

    return NULL;

}

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

        // pthread_detach(thread);
        
        // clientesAtendidos++;

    }

    close(listenfd);
    
}