#include <stdlib.h>
#include <stdio.h>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX 
#include <sys/time.h>

// Función utilizada por qsort() para comparar los elementos.
int comparar(const void *_a, const void *_b) {
    int *a, *b;
    
    a = (int *) _a;
    b = (int *) _b;
    
    return (*a - *b);
}

void imprimirArreglo(const int *arreglo, const unsigned int numElem) {
	printf("[");
	
	for (int i = 0; i < numElem; i++) {
		printf("%d, ", arreglo[i]);
	}
	
	printf("\b\b]\n");
}

// Lectura de los datos.
int read_array(char* fname, int **arr) {
  FILE *myFile;
  myFile = fopen(fname, "r");

  if (!myFile) {
    printf("ERROR: No se pudo abrir el archivo para lectura %s",fname);
    return -1;
  }

  unsigned int n;
  fscanf(myFile, "%i\n", &n); // Número de datos a leer.

  *arr = (int *) malloc(n*1.0*sizeof(int));

  if (*arr == NULL) {
    printf("Memoria insuficiente\n");
    return -1;
  }

  for (int i = 0; i < n; i++)
    fscanf(myFile, "%i\n", (*arr)+i);

  return n;
}

// Programa principal
int main(int argc, char**argv) {
  int *Arreglo;
  unsigned int n;

  struct timeval tInicial;  // <--- toma de tiempo 1
  gettimeofday(&tInicial, 0);
  
  n = read_array(argv[1], &Arreglo);

  //imprimirArreglo(Arreglo, n);

  qsort(Arreglo, n, sizeof(int), &comparar);
 
  struct timeval tFinal;  // <--- toma de tiempo 2
  gettimeofday(&tFinal, 0);

  //imprimirArreglo(Arreglo, n);

  long segundos = tFinal.tv_sec - tInicial.tv_sec;
  long microsegundos = tFinal.tv_usec - tInicial.tv_usec;
  double tTotal = segundos + (microsegundos * 1e-6);

  printf("\nCantidad de elementos = %d", n);
  printf("\nTiempo total = %f segundos\n", tTotal);

  free(Arreglo);

  return 0;
}

