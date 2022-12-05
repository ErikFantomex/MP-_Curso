#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)
#define BLOCK_OWNER(j,p,n) (((p)*((j)+1)-1)/(n))
//  ■■■■■■■■■■■■■■■■■
int main( int argc, char *argv[] ){
    int i, size, n, rank, count, n_local;
    int *buffer;
    MPI_File fh;
    MPI_Comm comm;
    MPI_Status status;
    MPI_Offset *offset;

    MPI_Init( &argc, &argv );

    comm = MPI_COMM_WORLD;
    MPI_Comm_size( comm, &size );
    MPI_Comm_rank( comm, &rank );

    n = atoi(argv[1]) ;

    n_local = BLOCK_SIZE( rank, size, n );
    printf("\n %d) n=%d n_local = %d\n",rank, n, n_local);

    srand(rank + time(0));
    buffer = (int*)malloc(n_local*sizeof(int)); if(buffer==NULL){ printf("\n ERROR: No hay memoria suficiente (buffer)"); MPI_Finalize(); return 1; }
    for(i=0;i<n_local;i++) 
        buffer[i] = rand()%100;

    offset = (MPI_Offset*)malloc(size*sizeof(MPI_Offset));if(offset==NULL){ printf("\n ERROR: No hay memoria suficiente (offset)"); MPI_Finalize(); return 1; }

    offset[0] = sizeof(int);
    for(i=1;i<size;i++)
        offset[i] = offset[i-1] + BLOCK_SIZE( i-1, size, n )*sizeof(int);
 
    MPI_File_open( comm, argv[2] ,MPI_MODE_WRONLY | MPI_MODE_CREATE, MPI_INFO_NULL, &fh);
        if(rank==0) MPI_File_write(fh, &n, 1, MPI_INT, MPI_STATUS_IGNORE); 
       MPI_File_write_at(fh, offset[rank], buffer, n_local, MPI_INT, &status);
    MPI_File_close( &fh );


        for(i=0;i<n_local;i++)
        printf("[%d]%d\t ", rank,buffer[i]);
    printf("\n");    

    MPI_Finalize();
    return 0;
}