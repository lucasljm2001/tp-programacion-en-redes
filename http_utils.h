#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <stddef.h> // para size_t
#include <sys/types.h> // para off_t

// Callback genérico
typedef void * (* PCALLBACK) (void *);

// Estructura de la request HTTP
typedef struct {
    char method[16];      
    char resource[1024];  
    char protocol[16];    
    char headers[10][256]; 
    int header_count;
} HTTPRequest;

// Funciones exportadas
HTTPRequest parse_request(const char *buffer);
void* atenderCliente(void* args);

#endif // HTTP_UTILS_H
