#include <unistd.h>
#include <stdio.h>



void readKey() {
    
    
    char caracter = getchar();
    
    printf("keycode %d\n", caracter);
    
    return NULL;
}


int main(int argc, const char * argv[]) {
    
    pid_t p;

    printf("Original program, pid=%d\n", getpid());

    p = fork();

    if (p < 0) {
        perror("Error en fork");
        exit(1);
    } 
    else if (p == 0) {
        printf("Soy el proceso hijo con PID: %d\n", getpid());
        printf("El PID de mi padre es: %d\n", getppid());
        readKey();
        printf("El hijo termina\n");
    } 
    else {
        printf("Soy el proceso padre con PID: %d\n", getpid());
        printf("Mi hijo tiene PID: %d\n", p);
        printf("El padre termina\n");
    }


    // continuar la exec
    
    return 0;
}
