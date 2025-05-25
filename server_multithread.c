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
#include "threadpool.h"


#define DEFAULT_PORT 3030
#define DEFAULT_POOL_SIZE 4
#define DEFAULT_QUEUE_SIZE 16


int main(int argc, char *argv[]) {
    int listenfd = 0, clientSocket = 0;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t size = sizeof(client_addr);
    int puerto = DEFAULT_PORT;
    int pool_size = DEFAULT_POOL_SIZE;

    if (argc > 1) puerto = atoi(argv[1]);
    if (argc > 2) pool_size = atoi(argv[2]);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(puerto);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listenfd, 10);

    printf("Servidor escuchando en puerto %d con pool de %d threads\n", puerto, pool_size);

    // Crear el thread pool
    threadpool_t *pool = threadpool_create(pool_size, DEFAULT_QUEUE_SIZE, 0);
    if (!pool) {
        fprintf(stderr, "Error al crear el pool de threads\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        int *nuevoSocket = malloc(sizeof(int));
        if (!nuevoSocket) {
            perror("malloc");
            continue;
        }

        *nuevoSocket = accept(listenfd, (struct sockaddr *)&client_addr, &size);
        if (*nuevoSocket < 0) {
            perror("accept");
            free(nuevoSocket);
            continue;
        }

        printf("Conexión aceptada desde %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // PREGUNTAR SOBRE PONER EN ESPERA COMO SERIA, ACA SOLO RECHAZA
        int err = threadpool_add(pool, atenderCliente, nuevoSocket, 0);
        if (err) {
            fprintf(stderr, "Fallo al agregar tarea al pool: código %d\n", err);
            close(*nuevoSocket);
            free(nuevoSocket);
        }
    }

    threadpool_destroy(pool, 0); // nunca se llega aquí en este servidor, pero es lo correcto
    close(listenfd);

    return 0;
}
