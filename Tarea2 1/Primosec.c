#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

float time_diff(struct timeval *start, struct timeval *end){
    return (end->tv_sec - start->tv_sec) + 1e-6*(end->tv_usec - start->tv_usec);
}

int main(int argc, char **argv) {
    int i,j,flag;
    int n = atoi(argv[1]);
    time_t Inicio = time(NULL);

    struct timeval start;
    struct timeval end;    

    gettimeofday(&start, NULL);// <----- tiempo1
    
    int valoracion = 0;
    for ( i = 2; i <=n;i++){
        flag = 0;
        for (j = 2; j <= i/2; j++){
            if(i % j == 0){
              flag = 1;
                break;
            }
        }
        if(flag == 0){
          //printf("%d es un numero primo\n",i);
          valoracion++;
        }        
    }
    printf("\nTotal de Numeros primos: %d\n", valoracion);
    gettimeofday(&end, NULL); // <----- tiempo2
    printf("Tiempo ejecutado en: %f segundos",time_diff(&start, &end));
    
    return 0;
}
