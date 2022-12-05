#include <iostream>
#include <cstdlib>
#include <sys/time.h>

using std::cout;
using std::endl;

int main(int argc, char **argv) {
  int maxGap = 0, primerPrimo = 0, ultimoPrimo = 0, finRango = atoi(argv[1]);

  // Comenzamos la medición del tiempo.
  struct timeval inicioTiempo, finTiempo;
  gettimeofday(&inicioTiempo, 0);

  for (int i = 1; i <= finRango; i++) {
      bool esPrimo = true;

      for (int j = 2; j < i; j++) {
          if (i % j == 0) {
              esPrimo = false;

              break;
          }
      }
      
      if (esPrimo) {
          if (primerPrimo == 0) {
              primerPrimo = i;
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
  
  // Finalizamos la medición del tiempo.
  gettimeofday(&finTiempo, 0);

  long segundos = finTiempo.tv_sec - inicioTiempo.tv_sec;
  long microsegundos = finTiempo.tv_usec - inicioTiempo.tv_usec;
  double tiempoTotal = segundos + (microsegundos * 1e-6);

  cout << "\nRango (1, " << finRango << ")" << endl;
  cout << "Max Gap: " << maxGap << endl;
  cout << "Tiempo Total: " << tiempoTotal << endl;
}
