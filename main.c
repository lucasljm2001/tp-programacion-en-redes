#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef void * (* PCALLBACK) (void *);

void *readKey(void *args) {
    
    
    int* caracter = (int *) args;
    
    printf("keycode %d\n", *caracter);
    
    return NULL;
}


int main(int argc, const char * argv[]) {
    
    pthread_t thread;

    PCALLBACK callback = readKey;

    

    printf("Ha iniciado la ejecucion principal\n");

    while (1){
        printf("Ingresar caracter\n");
        int caracter = getchar();
        getchar();
        pthread_create(&thread, NULL, callback, &caracter);
        usleep(5000);

    }
        
    

    pthread_join(thread, NULL);

    printf("Finalizo el thread\n");


    // continuar la exec
    
    return 0;
}
