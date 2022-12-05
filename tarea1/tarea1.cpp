#include "mpi.h"
#include <iostream>
#include <cstring>
 
using namespace std;
 
int main( ) {
    MPI_Init(NULL, NULL);
 
    int cpu_ID, total_CPU;
    char salida[100], entrada[100];
 
    MPI_Comm_size(MPI_COMM_WORLD, &total_CPU);  // Número total de procesos.
    MPI_Comm_rank(MPI_COMM_WORLD, &cpu_ID);     // Valor de nuestro identificador.
 
    int root = 0;
    int last = total_CPU - 1;
    int next = cpu_ID + 1;
    int prev = cpu_ID - 1;
 
    if ( cpu_ID == root ) {
        // Enviando el mensaje.
        sprintf(salida, "HOLA PROCESO [%d], ATENTAMENTE PROCESO [%d].\n", next, root);
        MPI_Send(salida, strlen(salida) + 1, MPI_CHAR, next, root * 10 + 1, MPI_COMM_WORLD);
 
        // Recibiendo el mensaje.
        MPI_Recv(entrada, 100, MPI_CHAR, last, last * 10 + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        cout << "Soy el proceso [" << root << "], recibí el mensaje:\n" << entrada;
 
    } else if ( cpu_ID == last ) {
        // Enviando el mensaje.
        sprintf(salida, "HOLA PROCESO [%d], ATENTAMENTE PROCESO [%d].\n", root, last);
        MPI_Send(salida, strlen(salida) + 1, MPI_CHAR, root, last * 10 + 1, MPI_COMM_WORLD);
 
        // Recibiendo el mensaje.
        MPI_Recv(entrada, 100, MPI_CHAR, prev, prev * 10 + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        cout << "Soy el proceso [" << last << "], recibí el mensaje:\n" << entrada;
 
    } else {
        // Enviando el mensaje.
        sprintf(salida, "HOLA PROCESO [%d], ATENTAMENTE PROCESO [%d].\n", next, cpu_ID);
        MPI_Send(salida, strlen(salida) + 1, MPI_CHAR, next, cpu_ID * 10 + 1, MPI_COMM_WORLD);
 
        // Recibiendo el mensaje.
        MPI_Recv(entrada, 100, MPI_CHAR, prev, prev * 10 + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
        cout << "Soy el proceso [" << cpu_ID << "], recibí el mensaje:\n" << entrada;
 
    }
                            
    MPI_Finalize(); 
    return 0;
}
