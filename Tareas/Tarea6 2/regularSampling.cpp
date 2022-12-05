#include <cstdlib>
#include <stdio.h>
#include <iostream>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX
#include "mpi.h"

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n)(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

using namespace std;
// Compara dos valores para la función qSort.
int comparar(const void *_a, const void *_b) {
	int *a, *b;
	
	a = (int *) _a;
	b = (int *) _b;
	
	return (*a - *b);
}

// Comprueba si se le ha asignado memoria al puntero.
void asignarMemoria(int **arreglo, const unsigned numElem, const char nombreVar[30]) {
	*arreglo = (int *) malloc(numElem * sizeof(int));
	
	if (*arreglo == NULL) {
		printf("Memoria insuficiente %s\n", nombreVar);
		
		MPI_Abort(MPI_COMM_WORLD, 99);
	}
}

//  Lectura de archivos
int read_array(char* fname, int **arr, int np) {
	FILE *myFile;
	myFile = fopen(fname, "r");
	
	if (!myFile) {
		printf("ERROR: No se pudo abrir el archivo para lectura %s", fname);

		MPI_Abort(MPI_COMM_WORLD, 99);
	}
	
	unsigned int n;
	fscanf(myFile, "%i\n", &n); // numero de datos a leer
	
	asignarMemoria(arr, n, "(arr)");
	
	for (int i = 0; i < n; i++) {
		fscanf(myFile, "%i\n", (*arr) + i);
	}
	
	for (int i = n; i < n; i++) {
		fscanf(myFile, "%i\n", (*arr)+i);
	}
	
	return n;
}

// Imprime el arreglo completo.
void imprimirArreglo(const int *arreglo, const unsigned numElem) {
	printf("[");
	
	for (int i = 0; i < numElem; i++) {
		printf("%d, ", arreglo[i]);
	}
	
	printf("\b\b]\n");
}

// Ordena los elementos del arreglo dado.
void mezclar(int *destino, int *numDestino, int *agregado, int numAgregado) {
    int numAux = (*numDestino) + numAgregado;
    int *aux;

    asignarMemoria(&aux, numAux, "(aux)");

    int i = 0, j = 0, k = 0;
    while (j < (*numDestino) && k < numAgregado) {
        aux[i++] = (destino[j] < agregado[k]) ? destino[j++] : agregado[k++];
    }

    while (j < (*numDestino)) {
        aux[i++] = destino[j++];
    }

    while (k < numAgregado) {
        aux[i++] = agregado[k++];
    }

    for (i = 0; i < numAux; i++) {
        destino[i] = aux[i];
    }

    (*numDestino) = numAux;

    free(aux);
}

// Programa principal.
int main(int argc, char**argv) {
	int id, np, n, i, j, n_local, ndatos_recv;
	int *pivotes, *sendcounts, *sendbuf, *recvcounts, *recvbuf, *recvbuf_mezclas;
	int *muestras, *todas_las_muestras, *displs, *sdispls, *rdispls;
	double start, time;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
	MPI_Comm_rank(MPI_COMM_WORLD, &id); // Valor de nuestro identificador
	
	MPI_Barrier(MPI_COMM_WORLD);  // sincronización
	start = MPI_Wtime();
	
	// El root lee un dataset del archivo con n datos e inicia una dispersión de los datos.  <-----  (1)
	int root = np - 1;
	if (id == root) { // Lee los datos del archivo
		n = read_array(argv[1], &sendbuf, np);
 	}
	
	asignarMemoria(&sendcounts, np, "(sendcounts)");
	
	if (id == root) {
		for (i = 0; i < np; i++) {
			sendcounts[i] = BLOCK_SIZE(i, np, n);
		}
		
		asignarMemoria(&displs, np, "(displs)");
		
		displs[0] = 0;
		for (i = 1; i < np; i++) {
			displs[i] = displs[i - 1] + sendcounts[i - 1];
		}
	}
	
	// Comunica longitud de los chunks
	MPI_Bcast(&n, 1, MPI_INT, root, MPI_COMM_WORLD);
	n_local = BLOCK_SIZE(id, np, n);
	
	asignarMemoria(&recvbuf, n_local, "(recvbuf)");
	
	MPI_Scatterv(sendbuf, sendcounts, displs, MPI_INT, recvbuf, n_local, MPI_INT, root, MPI_COMM_WORLD);
	
	// Cada proceso ordena localmente sus datos con QuickSort  <-----  (2)
	qsort(recvbuf, n_local, sizeof(recvbuf[0]), &comparar);
	
	// Cada proceso selecciona los n índices 0, n/p^2, 2n/p^2, 3n/p^2,..., (p-1)n/p^2 como muestras regulares <-----  (3)
	asignarMemoria(&muestras, np, "(muestras)");
	
	for (i = 0; i < np; i++) {
		muestras[i] = recvbuf[ i*n/(np*np) ];
	}

        // Un proceso recolecta y ordena las muestras regulares y selecciona p-1 valores pivote de la lista ordenada. <-----  (4)
        // Los valores pivote están en los índices p+p/2-1, 2p+p/2-1, 3p+p/2-1, ... , (p-1)p+p/2-1.
        // Los valores pivote son dados a conocer a todos los procesos.
        if (id == root) {
		asignarMemoria(&todas_las_muestras, np*np, "(todas_las_muestras)");
	}

	MPI_Gather( muestras, np, MPI_INT, todas_las_muestras, np, MPI_INT, root, MPI_COMM_WORLD );
	
	if (id == root) {
		qsort( todas_las_muestras, np*np, sizeof(int), &comparar );
	}

	asignarMemoria(&pivotes, np-1, "(pivotes)");

	if (id == root) {
		for (i = 0; i < np - 1; i++) {
			pivotes[i] = todas_las_muestras[ (i+1)*np + np/2 -1 ];
		}
	}

	MPI_Bcast(pivotes, np-1, MPI_INT, root, MPI_COMM_WORLD);

	// Cada proceso particiona su sublista ordenada en p piezas disjuntas, usando los valores pivote como separadores. <----- (5)
	for (i = 0, j = 0; i < (np-1); i++) {
		sendcounts[i] = 0;
		
		while (recvbuf[j] <= pivotes[i] && j < n_local) {
			sendcounts[i]++;
			
			j++;
		}
	}
	
	sendcounts[np-1] = 0;
	while (j < n_local) {
		sendcounts[np-1]++;
		
		j++;
	}

	asignarMemoria(&sdispls, np, "(sdispls)");

	sdispls[0] = 0;
	for (i = 1; i < np; i++) {
		sdispls[i] = sdispls[i-1] + sendcounts[i-1];
	}

	asignarMemoria(&recvcounts, np, "(recvcounts)");

	MPI_Alltoall( sendcounts, 1, MPI_INT, recvcounts, 1, MPI_INT, MPI_COMM_WORLD );
	
	asignarMemoria(&rdispls, np, "(rdispls)");

	rdispls[0] = 0;
	for (i = 1; i < np; i++) {
		rdispls[i] = rdispls[i-1] + recvcounts[i-1];
	}

	ndatos_recv = 0;
	for (i = 0; i < np; i++) {
		ndatos_recv += recvcounts[i];
	}
	
	asignarMemoria(&recvbuf_mezclas, ndatos_recv, "(recvbuf_mezclas)");

	// Cada proceso P_i mantiene su i-ésima partición y envía la j-ésima partición al proceso P_j (alltoallv)  <----- (6)
	MPI_Alltoallv( recvbuf, sendcounts, sdispls, MPI_INT, recvbuf_mezclas, recvcounts, rdispls, MPI_INT, MPI_COMM_WORLD );

	// Cada proceso mezcla sus p particiones para obtener una sola lista.  <----- (7)
	int cantDatos = recvcounts[0];
	for (i = 1; i < np; i++) {
			mezclar(recvbuf_mezclas, &cantDatos, recvbuf_mezclas + rdispls[i], recvcounts[i]);
	}

	// Recolectar los elementos que tiene cada proceso en un solo proceso con fines de impresión <---- (8)
	MPI_Gather(&ndatos_recv, 1, MPI_INT, recvcounts, 1, MPI_INT, root, MPI_COMM_WORLD);
	
	if (id == root) {
		rdispls[0] = 0;
		for (i = 1; i < np; i++) {
			rdispls[i] = rdispls[i-1] + recvcounts[i-1];
		}
	}
	
	MPI_Gatherv( recvbuf_mezclas, ndatos_recv, MPI_INT, sendbuf, recvcounts, rdispls, MPI_INT, root, MPI_COMM_WORLD );
	
	time = MPI_Wtime() - start;

        if (id == root) {
		cout << np<< ","<< time<< endl;
		//printf("\nCantidad de elementos = %d", n);
		//printf("\nTiempo necesario = %f\n", time);
	}

	// Liberar memoria
	if (id == root) {
		free(sendcounts);
		free(displs);
		free(todas_las_muestras);
	}

	free(recvbuf);
	free(muestras);
	free(pivotes);
 
	MPI_Finalize();
	return 0;
}

