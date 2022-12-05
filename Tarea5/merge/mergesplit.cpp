#include <stdlib.h>
#include <iostream>
#include "mpi.h"
#include <stdio.h>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX 

using namespace std;
// Función utilizada por qsort() para comparar los elementos.
int compara(const void *_a, const void *_b) {
    int *a, *b;
    
    a = (int *) _a;
    b = (int *) _b;
    
    return (*a - *b);
}

// Ordena los subarreglos resultantes.
void mezcla(int *S_i, int chunksize) {
    int *aux;

    aux = (int*)malloc(chunksize*sizeof(int));

    int i = 0, j = 0, k =  chunksize/2;

    while (j < chunksize/2 && k < chunksize) {
        aux[i++] = (S_i[j] < S_i[k]) ? S_i[j++] : S_i[k++];
    }

    while (j < chunksize/2) {
        aux[i++] = S_i[j++];
    }

    while (k < chunksize) {
        aux[i++] = S_i[k++];
    }

    for (i = 0; i < chunksize; i++) {
        S_i[i] = aux[i];
    }

    free(aux);
}

//  Lectura de archivos
int read_array(char* fname, int **arr, int np) {
  FILE *myFile;
  unsigned int i, n, chunksize, faltantes;

  myFile = fopen(fname, "r");
  if(!myFile){
    printf("ERROR: No se pudo abrir el archivo para lectura %s",fname);
    MPI_Abort(MPI_COMM_WORLD, 99);
  }

  fscanf(myFile, "%i\n", &n); // numero de datoss a leer

  chunksize = ceil(n*1.0 / np);
  faltantes = np*chunksize - n;

  *arr = (int *) malloc((chunksize*np)*sizeof(int));
  if (*arr == NULL) {
    printf("Memoria insuficiente\n");
    MPI_Abort(MPI_COMM_WORLD, 99);
  }

  for ( i=0; i < n; i++)
    fscanf(myFile, "%i\n", (*arr)+i);

  for ( i=n; i < n + faltantes; i++) //<--- Dummys
    (*arr)[i] = INT_MAX;

  return n;
}

// Programa principal
int main(int argc, char**argv) {
  int rank, np, root, faltantes;
  unsigned int n, chunksize, i;
  double start, time;
  int *Arreglo, *S_i;

  MPI_Init(&argc, &argv);

  MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
  MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Valor de nuestro identificador

  root = np - 1;  // Root es el último

  MPI_Barrier(MPI_COMM_WORLD);  // sincronización
  double tInicial = MPI_Wtime();  // <--- toma de tiempo 1

  if (rank == root){ // Lee los datos del archivo
    n = read_array(argv[1], &Arreglo, np);
    chunksize = ceil(n*1.0 / np);
    faltantes = np*chunksize - n;
  }

  // Comunica longitud de los chunks.
  MPI_Bcast(&chunksize, 1, MPI_UNSIGNED, root, MPI_COMM_WORLD);
  
  // Pide memoria del doble de dicha longitud para poder ordenar.
  S_i = (int *) malloc(2*chunksize*sizeof(int));

  if (S_i == NULL) {
    printf("Memoria insuficiente\n");
    MPI_Abort(MPI_COMM_WORLD, 99);
  }

  MPI_Scatter(Arreglo, chunksize, MPI_INT, S_i, chunksize, MPI_INT, root, MPI_COMM_WORLD);

  qsort(  S_i, chunksize, sizeof(int), &compara );

  for(i=1;i<=ceil(np/2.0);i++){
    if(rank%2==1) {
         MPI_Send( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD );
         MPI_Recv( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }

    if(rank%2==0 && rank!=np-1){
      MPI_Recv( S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );   
      mezcla(S_i, 2*chunksize); // <---- 
      MPI_Send(S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD);  
    }

    if(rank%2==0 && rank!=0) {
         MPI_Send( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD );
         MPI_Recv( S_i, chunksize, MPI_INT, rank-1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
    }

    if(rank%2==1 && rank!=np-1){
      MPI_Recv( S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE );   
      mezcla(S_i, 2*chunksize); // <---- 
      MPI_Send(S_i+chunksize, chunksize, MPI_INT, rank+1, 10, MPI_COMM_WORLD);  
    }
  }
  
  // Recolectar de nuevo los datos en el root e imprimir.
  MPI_Gather(S_i, chunksize, MPI_INT, Arreglo, chunksize, MPI_INT, root, MPI_COMM_WORLD );
  
  double tFinal = MPI_Wtime(); // <--- toma de tiempo 2

  if(rank==root) {
      double tTotal = tFinal - tInicial; 

      cout << np << ","<<tTotal<<endl;
      //printf("\nCantidad de elementos = %d", n);
      //printf("\nTiempo total = %f segundos", tTotal);
      //printf(" \n");
  }
  
  free(S_i);
  if (rank == root) free(Arreglo);

  MPI_Finalize();
  return 0;
}
