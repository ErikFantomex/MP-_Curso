#include <iostream>
#include <cstdlib>
#include "mpi.h"

using std::cout;
using std::endl;

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n)(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

int main(int argc, char **argv ) {      
  MPI_Init( &argc , &argv); 
  int id, i, j, np, n, ind, gap, inicio, fin, num_elems_rango, root = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &id); 
  MPI_Comm_size(MPI_COMM_WORLD, &np); 

  n      = atoi(argv[1]);
  inicio = BLOCK_LOW(id, np, n) + 1; // para que empiece en 1
  fin    = BLOCK_HIGH(id, np, n) + 1;
  num_elems_rango = BLOCK_SIZE(id, np, n);

  int maxGap = 0, primerPrimAbs = 0, primerPrimo = 0, ultimoPrimo = 0;

  // Comenzamos la medición del tiempo.
  MPI_Barrier(MPI_COMM_WORLD);
  double inicioTiempo = MPI_Wtime();

  for (int i = (inicio == 1 ? 2 : inicio); i <= fin; i++) {
      bool esPrimo = true;

      for (int j = 2; j < i; j++) {
          if (i % j == 0) {
              esPrimo = false;

              break;
          }
      }
      
      if (esPrimo) {
          if (primerPrimo == 0) {
              primerPrimAbs = primerPrimo = i;
          } else if (ultimoPrimo == 0) {
              ultimoPrimo = i;

              maxGap = ultimoPrimo - primerPrimo;
          } else {
              primerPrimo = ultimoPrimo;
              ultimoPrimo = i;

              if (maxGap < ultimoPrimo - primerPrimo) {
                  maxGap = ultimoPrimo - primerPrimo;
              }
          }
      }
  }

  if (ultimoPrimo == 0) {
      ultimoPrimo = primerPrimo;
  }

  if (id != np - 1 && ultimoPrimo != 0) {
      // Enviar ultimoPrimo con SEND().
      MPI_Send(&ultimoPrimo, 1, MPI_INT, id + 1, id * 10 + 1, MPI_COMM_WORLD);   
  }
  
  if (id != root) {
      int primoRecibido;
      MPI_Recv(&primoRecibido, 1, MPI_INT, id - 1, (id - 1) * 10 + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      if (primoRecibido > 1 && primoRecibido < primerPrimAbs) {
          if (maxGap < primerPrimAbs - primoRecibido) {
              maxGap = primerPrimAbs - primoRecibido;
          }
      }
      
  }

  int maxGapFinal;
  MPI_Reduce(&maxGap, &maxGapFinal, 1, MPI_INT, MPI_MAX, root, MPI_COMM_WORLD);

  if(id == root) {
    // Finalizamos la medición del tiempo.
    double finTiempo = MPI_Wtime();
    double tiempoTotal = finTiempo - inicioTiempo;

    //cout << "\nRango (1, " << n << ")" << endl;
    cout << np <<","<< tiempoTotal <<","<< maxGapFinal << endl;
    //cout << "Max Gap: " << maxGapFinal << endl;
    //cout << "Tiempo Total: " << tiempoTotal << endl;
  }
 
  MPI_Finalize(); 
}
