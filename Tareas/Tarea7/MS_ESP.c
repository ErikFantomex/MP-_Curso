#include <stdlib.h>
#include "mpi.h"
#include <stdio.h>
#include <math.h> // ceil()
#include <limits.h> // INT_MAX 

#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
#define BLOCK_OWNER(j,p,n) (((p)*((j)+1)-1)/(n))


void mezcla(int* S_i, int chunksize2){
    
    int *aux = NULL;
    // Pedimos memoria al arreglo auxilar
    aux = (int*) malloc(chunksize2*sizeof(int));
    
    if(aux == NULL){printf("No hay memoria\n");   MPI_Abort(MPI_COMM_WORLD,99);  }
    
    // Hacemos la copia del arreglo
    for (int i=0; i<chunksize2/2;i++){
      aux[i]=S_i[i];
    }

    int a = 0; // Indice que recorre la primera parte del arreglo
    int b = chunksize2 /2; // Indice que recorre la segunda parte del arreglo
    int i=0;

    // Mientras algun índice no llegue a la parte final que recorren

    while (a < chunksize2/2 && b < chunksize2){
        if(aux[a] <= S_i[b]){
            S_i[i] = aux[a];
            a++;
        }else{
            S_i[i] = S_i[b];
            b++;
        }
        i++;
    }
    // Si el indice b llegó al final de su arreglo

    if(b == chunksize2){
        // Asignamos los numeros restantes
        while(i < chunksize2){
            S_i[i] = aux[a];
            a++;
            i++;
        }
    }
    free(aux);
}


int compare(const void *_a, const void *_b) {
 
        int *a, *b;
        
        a = (int *) _a;
        b = (int *) _b;
        
        return (*a - *b);
}


//  ■■■■
int main(int argc, char**argv) {
    int rank, np, root;
    unsigned int n, chunksize, i, chunk;
    double start, tiempo;
    int *S_i;
    int dummys = 0, total_dummys = 0;
    MPI_Offset  *offset, *offset2;
    MPI_Status  status;
    MPI_File fh,fh2;


    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &np); // Numero total de procesos
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Valor de nuestro identificador

    MPI_Barrier(MPI_COMM_WORLD);  // sincronización
    start = MPI_Wtime();

    // Root es el último
    root = np - 1;

    offset = (MPI_Offset*)malloc(np*sizeof(MPI_Offset));if(offset==NULL){ printf("\n ERROR: No hay memoria suficiente (offset)"); MPI_Abort(MPI_COMM_WORLD, 99); }
    MPI_File_open( MPI_COMM_WORLD, argv[1], MPI_MODE_RDONLY, MPI_INFO_NULL  , &fh );
    MPI_File_read(fh, &n, 1, MPI_INT, &status);

    offset[0] = sizeof(int);
    for( i = 1 ; i < np ; i++ ) 
        offset[i] = offset[i-1] + BLOCK_SIZE( i-1, np, n )*sizeof(int);

    chunksize = BLOCK_SIZE( rank, np, n );
    chunk = ceil(n*1.0 / np);

    S_i = (int*)malloc(2*chunk*sizeof(int)); if( S_i == NULL ){ printf("\n ERROR: No hay memoria suficiente (buffer)"); MPI_Abort(MPI_COMM_WORLD, 99); }
    
    MPI_File_read_at(fh, offset[rank], S_i, chunksize, MPI_INT, &status);        
    MPI_File_close(&fh);   

  

    // ------aqui va el procesamiento a realizar con los datos

    qsort(  S_i, chunksize,sizeof(int), &compare );



    if (chunksize < chunk) {
        S_i[chunksize] = INT_MAX;
        dummys++;
        chunksize++;
    }

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

  MPI_Reduce(&dummys, &total_dummys, 1, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD); 

  if ( rank == root){
      chunksize-=total_dummys;
  }




    offset2 = (MPI_Offset*)malloc(np*sizeof(MPI_Offset));if(offset==NULL){ printf("\n ERROR: No hay memoria suficiente (offset)"); MPI_Finalize(); return 1; }

    offset2[0] = sizeof(int);
    for(i=1;i<np;i++){
        offset2[i] = offset2[i-1] + ceil(n*1.0 / np)*sizeof(int);
    }



    MPI_File_open( MPI_COMM_WORLD, argv[2] ,MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh2);
        if(rank==0) MPI_File_write(fh2, &n, 1, MPI_INT, MPI_STATUS_IGNORE); 
      MPI_File_write_at(fh2, offset2[rank], S_i, chunksize, MPI_INT, &status);
    MPI_File_close( &fh2 );

      // <--- toma de tiempo 2
    tiempo = MPI_Wtime() - start;

    free(S_i);
    free(offset);
    free(offset2);
    if(rank==root)
       printf("%d\t%10f\n", np,",",tiempo);
    MPI_Finalize();
    return 0;
}







