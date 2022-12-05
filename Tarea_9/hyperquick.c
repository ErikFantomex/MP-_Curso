#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "cabecera.h"

#define BLOCK_LOW(id,p,n) ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n)(BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)


//  ¦¦¦¦
int main(int argc,char *argv[]){
    double elapsed_time;	
    int          *Arreglo, *bloque, *bloqueMenor, *bloqueMayor, *bloqueAux, pivote, partner, nprocesos_minimo;
    int          idCart, idCart2, p, pCart, i, j, id, root;	    
    int          ND, *dimensiones, *periodicos;
    int          *cambiosDedimension, *cambiosDedimension2; 
    unsigned int nt,cuantosMn, cuantosMy, cuantosMn2, cuantosMy2, ndatos_local;
    unsigned int *sendcounts,*sdispls, *recvcounts, *displs;
    MPI_Comm     comm_topologia_hipercubo, comm_sub_hipercubo, comm_sub_hipercubo2;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&id);
    MPI_Comm_size(MPI_COMM_WORLD,&p);
    elapsed_time = - MPI_Wtime();

    if(argc!=3){
		if(id==0) printf("ERROR: mpirun -np <NP> ./hiperquicksort nombre_archivo <NumDimensiones> \n");
		MPI_Abort(MPI_COMM_WORLD,1);
        return EXIT_SUCCESS;
	}

    ND = atoi(argv[2]);

    dimensiones = (int*) malloc(ND*sizeof(int));    if( dimensiones == NULL ){ printf("ERROR: Memoria insuficiente! (dimensiones)\n");  MPI_Abort(MPI_COMM_WORLD, 99); }
    periodicos = (int*) malloc(ND*sizeof(int));     if( periodicos  == NULL ){ printf("ERROR: Memoria insuficiente! (periodicos)\n");   MPI_Abort(MPI_COMM_WORLD, 99); }

    nprocesos_minimo = 1; 
    for(i=0;i<ND;i++){
        dimensiones[i]    = 2;
        periodicos[i]     = 0;
        nprocesos_minimo *= 2;
    }

    if( p < nprocesos_minimo ){
        printf( "\nERROR: Se requieren al menos %d procesos \n\n", nprocesos_minimo );
        MPI_Abort( MPI_COMM_WORLD, 99 ); 
    }

    MPI_Cart_create(MPI_COMM_WORLD,ND,dimensiones,periodicos,1,&comm_topologia_hipercubo);

    if(comm_topologia_hipercubo!=MPI_COMM_NULL){
        MPI_Comm_rank(comm_topologia_hipercubo, &idCart);
        MPI_Comm_size(comm_topologia_hipercubo, &pCart);
        root = 0;
        if (idCart == root){ // Lee los datos del archivo
            nt = read_array(argv[1], &Arreglo);      
        }

        MPI_Bcast(&nt, 1, MPI_UNSIGNED, root, comm_topologia_hipercubo); 

        // ----  El root envía los datos que le corresponden a cada proceso
        // ----   No necesariamente le va a enviar el mismo numero de datos a cada proceso, por eso se requiere MPI_Scatterv  

        ndatos_local = BLOCK_SIZE(idCart,pCart,nt); 

        bloque     = (int*) malloc(ndatos_local*sizeof(int));               if( bloque     == NULL ){ printf("ERROR: Memoria insuficiente! (bloque)\n");        MPI_Abort(comm_topologia_hipercubo, 99); } 
        sendcounts = (unsigned int*) malloc(pCart*sizeof(unsigned int));    if( sendcounts == NULL ){ printf("ERROR: Memoria insuficiente! (sendcounts)\n");    MPI_Abort(comm_topologia_hipercubo, 99);  } 
        sdispls    = (unsigned int*) malloc(pCart*sizeof(unsigned int));    if( sdispls    == NULL ){ printf("ERROR: Memoria insuficiente! (sdispls)\n");       MPI_Abort(comm_topologia_hipercubo, 99); } 

        if( idCart == root ){
            for( i = 0 ; i < pCart; i++ )
                sendcounts[i]=BLOCK_SIZE(i,pCart,nt);
            
            sdispls[0] = 0;
            for( i = 1 ; i < pCart; i++ )  
                sdispls[i] = sdispls[i-1] + sendcounts[i-1];
        }

        MPI_Scatterv( Arreglo, sendcounts, sdispls, MPI_INT, bloque, ndatos_local, MPI_INT, root, comm_topologia_hipercubo );

        // Cada proceso ordena localmente su bloque
        qsort(bloque, ndatos_local, sizeof(int), funcionQueCompara);


        // seleccionar un pivote y mandarlo a todo el comunicador
        if( idCart == root ){
            if( ndatos_local % 2 == 0 ) pivote = (bloque[ndatos_local/2]+bloque[(ndatos_local-2)/2])/2; // el promedio de los dos valores medios
            else                        pivote = bloque[ndatos_local/2];
        }

        MPI_Bcast (&pivote, 1, MPI_INT, root, comm_topologia_hipercubo );
        separaDatos_Pivote( bloque, &cuantosMn, &bloqueMenor, &cuantosMy, &bloqueMayor, pivote, ndatos_local, 0 , idCart);      

        cambiosDedimension  = (int*) malloc(ND*sizeof(int));  if( cambiosDedimension  == NULL ){ printf("ERROR: Memoria insuficiente! (cambiosDedimension)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }
        cambiosDedimension2 = (int*) malloc(ND*sizeof(int));  if( cambiosDedimension2 == NULL ){ printf("ERROR: Memoria insuficiente! (cambiosDedimension2)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }

        for( i = 0 ; i < ND; i++ ){
            for(j=0;j<ND;j++){
                if(j==i){
                    cambiosDedimension[j]=1;				
                    cambiosDedimension2[j]=0;
                }else{
                    cambiosDedimension[j]=0;
                    if(i>j) cambiosDedimension2[j]=0;				
                    else    cambiosDedimension2[j]=1;
                }
            }

            MPI_Cart_sub(comm_topologia_hipercubo, cambiosDedimension, &comm_sub_hipercubo);	// se determina el partner	
            MPI_Comm_rank(comm_sub_hipercubo, &idCart);           

            if(idCart==0){	// envio los mayores al partner y recibo los menores del partner
                partner = 1;			
                MPI_Send(  &cuantosMy, 1, MPI_UNSIGNED, partner, 0, comm_sub_hipercubo          );                
                if( cuantosMy != 0 ){
                    MPI_Send(bloqueMayor, cuantosMy, MPI_INT, partner, 3, comm_sub_hipercubo);
                    free(bloqueMayor);
                }

                MPI_Recv( &cuantosMn2, 1, MPI_UNSIGNED, partner, 1, comm_sub_hipercubo, MPI_STATUS_IGNORE );				
                if( cuantosMn2 != 0 ){				
                    bloqueAux=(int *)malloc(cuantosMn2*sizeof(int));        if( bloqueAux  == NULL ){ printf("ERROR: Memoria insuficiente! (bloqueAux)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }		
                    MPI_Recv(bloqueAux, cuantosMn2, MPI_INT, partner, 4, comm_sub_hipercubo, MPI_STATUS_IGNORE);
                }

                if( ndatos_local != 0 )	free(bloque);

                bloque = mezclar_bloques( bloqueMenor, bloqueAux, cuantosMn, cuantosMn2);
                ndatos_local = cuantosMn + cuantosMn2;

                if( cuantosMn  != 0 ) free(bloqueMenor);				
                if( cuantosMn2 != 0 ) free(bloqueAux);
            }else{			
                partner = 0;	
                MPI_Send(&cuantosMn,  1, MPI_UNSIGNED, partner, 1, comm_sub_hipercubo);                
                if(cuantosMn!=0){
                    MPI_Send(bloqueMenor, cuantosMn, MPI_INT, partner, 4, comm_sub_hipercubo);
                    free(bloqueMenor);
                }

                MPI_Recv(&cuantosMy2, 1, MPI_UNSIGNED, partner, 0, comm_sub_hipercubo,  MPI_STATUS_IGNORE);				
                if(cuantosMy2!=0){				
                    bloqueAux=(int *)malloc(cuantosMy2*sizeof(int));        if( bloqueAux  == NULL ){ printf("ERROR: Memoria insuficiente! (bloqueAux)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }	
                    MPI_Recv(bloqueAux, cuantosMy2, MPI_INT, partner, 3, comm_sub_hipercubo,  MPI_STATUS_IGNORE);
                }			
                if( ndatos_local != 0 )	free(bloque);	
                						
                bloque = mezclar_bloques(bloqueMayor, bloqueAux, cuantosMy, cuantosMy2);
                ndatos_local = cuantosMy + cuantosMy2;

                if( cuantosMy  != 0 ) free(bloqueMayor);
                if( cuantosMy2 != 0 ) free(bloqueAux);
            }
        
            if(i!=ND-1){ // se divide en subcomunicadores
                MPI_Cart_sub( comm_topologia_hipercubo, cambiosDedimension2, &comm_sub_hipercubo2 );		
                MPI_Comm_rank( comm_sub_hipercubo2, &idCart2 );
                if(idCart2==0){
                    if( ndatos_local > 0 ){
                        if(ndatos_local%2==0) pivote = (bloque[ndatos_local/2]+bloque[(ndatos_local-2)/2])/2; // el promedio de los dos valores medios
                        else                  pivote = bloque[ndatos_local/2];
                    }else{
                        printf("\n ERROR: El root se quedó sin datos :( <==== \n\n");
                        MPI_Abort(MPI_COMM_WORLD, 99);
                    }
                }				
                MPI_Bcast(&pivote,1,MPI_INT,0,comm_sub_hipercubo2);	
                separaDatos_Pivote( bloque, &cuantosMn, &bloqueMenor, &cuantosMy, &bloqueMayor, pivote, ndatos_local, i , idCart);
            }	       
        }

        if(ND>1){
            MPI_Comm_free(&comm_sub_hipercubo);
            MPI_Comm_free(&comm_sub_hipercubo2);
        }

        MPI_Comm_rank(comm_topologia_hipercubo, &idCart);
        MPI_Comm_size(comm_topologia_hipercubo, &pCart);
        
        if(idCart==root){
            recvcounts = (unsigned int*) malloc(pCart*sizeof(unsigned int));  if( recvcounts  == NULL ){ printf("ERROR: Memoria insuficiente! (recvcounts)\n"); MPI_Abort(MPI_COMM_WORLD, 99); }
        }

        MPI_Gather( &ndatos_local, 1,   MPI_UNSIGNED, recvcounts, 1, MPI_UNSIGNED, root , comm_topologia_hipercubo );
        
        if(idCart==root){
            displs = (unsigned int*)malloc(pCart*sizeof(unsigned int));       if( displs     == NULL ){ printf("ERROR: Memoria insuficiente (displs)");         MPI_Abort(MPI_COMM_WORLD,99);  } 

            displs[0] = 0; 
            for( i = 1 ; i < pCart ; i++ ) 
                displs[i] = displs[i-1] + recvcounts[i-1]; 
        }    

        MPI_Gatherv(bloque, ndatos_local, MPI_INT, Arreglo, recvcounts, displs, MPI_INT, root, comm_topologia_hipercubo); 
	elapsed_time += MPI_Wtime();
 	if(idCart==root){
		printf (",SIEVE_ODD (%d) ,%10.6f, segs\n", p, elapsed_time);
    for(i = 0; i < nt; i++){
        printf("%d, ",Arreglo[i]);
    }
	}

        // NOTA: Falta liberar memoria (pendiente) <--------

        MPI_Comm_free(&comm_topologia_hipercubo);
    }

    MPI_Finalize();
    return 0;
}
