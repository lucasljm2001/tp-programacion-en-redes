#include "http_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

HTTPRequest parse_request(const char *buffer) {
    HTTPRequest req = {0};
    char *buffer_copy = strdup(buffer);
    if (!buffer_copy) return req;

    char *saveptr;
    char *line = strtok_r(buffer_copy, "\r\n", &saveptr);

    if (line) {
        sscanf(line, "%15s %1023s %15s", req.method, req.resource, req.protocol);
    }

    while ((line = strtok_r(NULL, "\r\n", &saveptr)) && req.header_count < 10) {
        if (strlen(line) == 0) break;
        strncpy(req.headers[req.header_count++], line, 255);
    }

    free(buffer_copy);
    return req;
}

void* atenderCliente(void* args) {
    int clientSocket = *((int *) args);
    char buffer[2048];
    char response[1024];
    struct stat fileStats;

    int imagefd = open("./hello.png", O_RDONLY);
    fstat(imagefd, &fileStats);

    memset(buffer, 0, sizeof(buffer));
    int totalRead = 0;
    int nbytes = 0;

    do {
        nbytes = recv(clientSocket, buffer + totalRead, sizeof(buffer) - totalRead - 1, 0);
        if (nbytes > 0) {
            totalRead += nbytes;
            buffer[totalRead] = '\0';
            if (strstr(buffer, "\r\n\r\n")) break;
        }
    } while (nbytes > 0 && totalRead < sizeof(buffer) - 1);

    HTTPRequest request = parse_request(buffer);

    printf("Método: %s\n", request.method);
    printf("Recurso: %s\n", request.resource);
    printf("Protocolo: %s\n", request.protocol);
    
    char *responseHeaders;

    if (strcmp(request.method, "GET") == 0 && strcmp(request.resource, "/imagen.jpg") == 0) {
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n";
    } else {
        responseHeaders = "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n";
        imagefd = open("./error.png", O_RDONLY);
        fstat(imagefd, &fileStats);
    }

    memset(response, 0, 1024);
    sprintf(response, responseHeaders, fileStats.st_size);

    send(clientSocket, response, strlen(response), 0);
    ssize_t sent_bytes = sendfile(clientSocket, imagefd, NULL, fileStats.st_size);

    if (sent_bytes == -1)
    {
        if (errno == EPIPE)
        {
           printf("El cliente cerro entes la conexion");
        } else{
            perror("sendfile");
        }
        
    }
    


    close(clientSocket);
    close(imagefd);
    free(args);

    return NULL;
}

ssize_t atenderClienteDesdeSelect(int clientSocket) {
    int totalRead = 0;
    char buffer[2048];
    char response[1024];
    struct stat fileStats;

    int imagefd = open("./hello.png", O_RDONLY);
    fstat(imagefd, &fileStats);

    memset(buffer, 0, sizeof(buffer));
    int nbytes = 0;

    do {
        nbytes = recv(clientSocket, buffer + totalRead, sizeof(buffer) - totalRead - 1, 0);
        if (nbytes > 0) {
            totalRead += nbytes;
            buffer[totalRead] = '\0';
            if (strstr(buffer, "\r\n\r\n")) break;
        }
    } while (nbytes > 0 && totalRead < sizeof(buffer) - 1);

    if (nbytes <= 0) {
        return nbytes;  // 0 o -1, socket cerrado o error
    }

    HTTPRequest request = parse_request(buffer);

    printf("Método: %s\n", request.method);
    printf("Recurso: %s\n", request.resource);
    printf("Protocolo: %s\n", request.protocol);

    char *responseHeaders;
    if (strcmp(request.method, "GET") == 0 && strcmp(request.resource, "/imagen.jpg") == 0) {
        responseHeaders = "HTTP/1.1 200 OK\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n";
    } else {
        responseHeaders = "HTTP/1.1 404 Not Found\r\nContent-Type: image/jpeg\r\nContent-Length: %ld\r\nConnection: close\r\n\r\n";
        imagefd = open("./error.png", O_RDONLY);
        fstat(imagefd, &fileStats);
    }

    memset(response, 0, 1024);
    sprintf(response, responseHeaders, fileStats.st_size);

    send(clientSocket, response, strlen(response), 0);
    ssize_t sent_bytes = sendfile(clientSocket, imagefd, NULL, fileStats.st_size);

    if (sent_bytes == -1 && errno != EPIPE) {
        perror("sendfile");
    }

    close(clientSocket);
    close(imagefd);
    return sent_bytes;  // Devuelvo bytes enviados para ver si cerro la conexion
}

