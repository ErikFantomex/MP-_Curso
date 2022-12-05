#include <iostream>
#include <cstdlib>
#include "mpi.h"

using std::cout;
using std::endl;

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)

int main(int argc, char **argv ) {      
  MPI_Init( &argc , &argv); 
  int id, i, j, np, n, gap, inicio, fin, root = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &id); 
  MPI_Comm_size(MPI_COMM_WORLD, &np); 

  n      = atoi(argv[1]);
  inicio = BLOCK_LOW(id, np, n) + 1; // para que empiece en 1
  fin    = BLOCK_HIGH(id, np, n) + 1;

  int numPrimGemelos = 0, primerPrimAbs = 0, primerPrimo = 0, ultimoPrimo = 0;

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

              if (ultimoPrimo - 2 == primerPrimo) {
                  numPrimGemelos++;
              }
          } else {
              primerPrimo = ultimoPrimo;
              ultimoPrimo = i;

              if (ultimoPrimo - 2 == primerPrimo) {
                  numPrimGemelos++;
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

      if (primoRecibido > 0 && primoRecibido < primerPrimAbs) {
          if (primerPrimAbs - 2 == primoRecibido) {
              numPrimGemelos++;
          }
      }
      
  }
  
  int numPrimGemTotal;
  MPI_Reduce(&numPrimGemelos, &numPrimGemTotal, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);

  if(id == root) {
    // Finalizamos la medición del tiempo.
    double finTiempo = MPI_Wtime();
    double tiempoTotal = finTiempo - inicioTiempo;  
    
  //cout << "\nRango (1, " << n << ")" << endl;
    cout << np <<',' << tiempoTotal <<  endl;
  //cout << "Cantidad de Primos Gemelos: " << numPrimGemTotal << endl;
  //  cout << "Tiempo Total: " << tiempoTotal << endl;
  }
 
  MPI_Finalize(); 
}
