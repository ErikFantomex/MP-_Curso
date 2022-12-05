#include "header.h"
#include <mpi.h>
#include <stdio.h>
#include <math.h>


#define MIN(a,b)           ((a)<(b)?(a):(b))
#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

int main (int argc, char *argv[]) {
    pgm img;
    int i, j, k, width, height;
    int id, p;
    int *copia, *sub, low, high, local_rows, *sprimeraFila, *rprimeraFila, *sultimaFila, *rultimaFila;
    int gx, gy, g, min, max;
    int *displs, sendcount, *recvcounts;

    float sobel_x[3][3] = {{-1, 0, 1},
                           {-2, 0, 2},
                           {-1, 0, 1}};

    float sobel_y[3][3] = {{-1, -2, -1},
                           {0, 0, 0},
                           {1, 2, 1}};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    read_pgm_file(argv[1], &img);

    // -------------------------------------------------------------------------- PROCESADMIENTO DE LA IMAGEN AQUI

    width = img.width;
    height = img.height;

    sprimeraFila = (int *)malloc(width * sizeof(int));  if( sprimeraFila == NULL ){ printf("ERROR: Memoria insuficiente (sprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    rprimeraFila = (int *)malloc(width * sizeof(int));  if( rprimeraFila == NULL ){ printf("ERROR: Memoria insuficiente (rprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    sultimaFila  = (int *)malloc(width * sizeof(int));  if( sultimaFila == NULL ){ printf("ERROR: Memoria insuficiente (sultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    rultimaFila  = (int *)malloc(width * sizeof(int));  if( rultimaFila == NULL ){ printf("ERROR: Memoria insuficiente (rultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 

    low = BLOCK_LOW(id, p, height);
    high = BLOCK_HIGH(id, p, height) + 1;
    local_rows = BLOCK_SIZE(id, p, height);

    sub   = (int *)malloc(width * local_rows * sizeof(int)); if( sub   == NULL ){ printf("ERROR: Memoria insuficiente (sub)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    copia = (int *)malloc(width * local_rows * sizeof(int)); if( copia == NULL ){ printf("ERROR: Memoria insuficiente (copia)"); MPI_Abort(MPI_COMM_WORLD,99); } 

    for (i = low, k = 0; i < high; i++, k++)
    {
        for (j = 0; j < width; j++)
        {
            sub[k * width + j] = img.imageData[i * width + j];
            copia[k * width + j] = img.imageData[i * width + j];
        }
    }   

    for (i = 0; i < width; i++)      
      sultimaFila[i] = sub[(local_rows - 1) * width + i];
    if (id != p - 1)    
      MPI_Send(sultimaFila, width, MPI_INT, id + 1, 1, MPI_COMM_WORLD);
    if (id != 0 ) 
      MPI_Recv(rprimeraFila, width, MPI_INT, id - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  

    for (i = 0; i < width; i++)      
      sprimeraFila[i] = sub[i];

    if (id!=0) 
      MPI_Send(sprimeraFila, width, MPI_INT, id - 1, 2, MPI_COMM_WORLD);
    if (id!=p-1)
      MPI_Recv(rultimaFila, width, MPI_INT, id + 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      for (i = 0; i < local_rows; i++){
        for (j = 0; j < width; j++){
            gx = 0;
            gy = 0;

            /*        PRIMER RENGLON        */
            if (i == 0){ // Si me encuentro en el primero renglón
                if (id != 0){ // Si no soy el cero
                    if (j != 0){ // Si no me encuentro en la primera columna hago los de la izquierda
                        gx += (sobel_x[0][0] * rprimeraFila[j - 1]);
                        gy += (sobel_y[0][0] * rprimeraFila[j - 1]);
                    }
                    gx += (sobel_x[0][1] * rprimeraFila[j]);
                    gy += (sobel_y[0][1] * rprimeraFila[j]);
                    if (j != width - 1){ // Si no me encuentro en la última columna hago los de la derecha
                    
                        gx += (sobel_x[0][2] * rprimeraFila[j + 1]);
                        gy += (sobel_y[0][2] * rprimeraFila[j + 1]);
                    }
                }
                if (j != 0){ // Si no me encuentro en la primera columna hago los de la izquiersa
                    gx += (sobel_x[2][0] * copia[width * (i + 1) + (j - 1)]);
                    gy += (sobel_y[2][0] * copia[width * (i + 1) + (j - 1)]);
                }
                gx += (sobel_x[2][1] * copia[width * (i + 1) + j]);
                gy += (sobel_y[2][1] * copia[width * (i + 1) + j]);
                if (j != width - 1){ // Si no me encuentro en la última columna hago los de la derecha
                    gx += (sobel_x[2][2] * copia[width * (i + 1) + (j + 1)]);
                    gy += (sobel_y[2][2] * copia[width * (i + 1) + (j + 1)]);
                }
            /*   ULTIMO RENGLÓN   */
            }else if (i == local_rows - 1){
                if (id != p - 1){ // Si no soy el ultimo proceso
                    if (j!= 0)
                    {
                        gx += (sobel_x[2][0] * rultimaFila[j - 1]);
                        gy += (sobel_y[2][0] * rultimaFila[j - 1]);
                    }
                    gx += (sobel_x[2][1] * rultimaFila[j]);
                    gy += (sobel_y[2][1] * rultimaFila[j]);

                    if (j != width - 1) // Si no soy la última columna hago los de la derecha
                    {
                        gx += (sobel_x[2][2] * rultimaFila[j + 1]);
                        gy += (sobel_y[2][2] * rultimaFila[j + 1]);
                    }
                }
                if (j!= 0) // Si no soy la primera columna hago los de la izquierda
                {
                    gx += (sobel_x[0][0] * copia[width * (i - 1) + (j - 1)]);
                    gy += (sobel_y[0][0] * copia[width * (i - 1) + (j - 1)]);
                }
                gx += (sobel_x[0][1] * copia[width * (i - 1) + j]);
                gy += (sobel_y[0][1] * copia[width * (i - 1) + j]);
                if (j != width - 1){ 
                    gx += (sobel_x[0][2] * copia[width * (i - 1) + (j + 1)]);
                    gy += (sobel_y[0][2] * copia[width * (i - 1) + (j + 1)]);
                }
            } /*  EL RESTO DE LOS RENGLONES  */
            else{
                if (j != 0) { // Si no soy la primera columna
                    gx += (sobel_x[0][0] * copia[width * (i - 1) + (j - 1)]);
                    gy += (sobel_y[0][0] * copia[width * (i - 1) + (j - 1)]);
                    gx += (sobel_x[2][0] * copia[width * (i + 1) + (j - 1)]);
                    gy += (sobel_y[2][0] * copia[width * (i + 1) + (j - 1)]);
                }
                if (j != width - 1)
                {
                    gx += (sobel_x[0][2] * copia[width * (i - 1) + (j + 1)]);
                    gy += (sobel_y[0][2] * copia[width * (i - 1) + (j + 1)]);
                    gx += (sobel_x[2][2] * copia[width * (i + 1) + (j + 1)]);
                    gy += (sobel_y[2][2] * copia[width * (i + 1) + (j + 1)]);
                }
                gx += (sobel_x[0][1] * copia[width * (i - 1) + j]);
                gy += (sobel_y[0][1] * copia[width * (i - 1) + j]);
                gx += (sobel_x[2][1] * copia[width * (i + 1) + j]);
                gy += (sobel_y[2][1] * copia[width * (i + 1) + j]);
            }

            if (j != 0)
            {
                gx += (sobel_x[1][0] * copia[width * i + (j - 1)]);
                gy += (sobel_y[1][0] * copia[width * i + (j - 1)]);
            }
            gx += (sobel_x[1][1] * copia[width * i + j]);
            gy += (sobel_y[1][1] * copia[width * i + j]);
            if (j != width - 1)
            {
                gx += (sobel_x[1][2] * copia[width * i + (j + 1)]);
                gy += (sobel_y[1][2] * copia[width * i + (j + 1)]);
            }

            g = (int)sqrt((gx * gx) + (gy * gy));

            if (g < 0)
                g = 0;
            if (g > 255)
                g = 255;

            sub[i * width + j] = g;
        }
    }
 


    if (!id)
    {
        recvcounts = (int *)malloc(p * sizeof(int));
        displs = (int *)malloc(p * sizeof(int));
    }

    // Se guarda el arreglo de recvcounts
    sendcount = local_rows * width;
    MPI_Gather(&sendcount, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Se genera el arreglo de diplacements
    if (!id)
    {
        displs[0] = 0;

        for (i = 1; i < p; i++)
        {
            displs[i] = displs[i - 1] + recvcounts[i - 1];
        }
    }

    MPI_Gatherv(sub, local_rows * width, MPI_INT, img.imageData, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    free(sub);
    free(copia);
    free(sprimeraFila);
    free(rprimeraFila);
    free(sultimaFila);
    free(rultimaFila);
    if (!id)
    {
        free(recvcounts);
        free(displs);
    }
    // -------------------------------------------------------------------------- ARCHIVO DE SALIDA
    if (!id)
        write_pgm_file(&img, argv[2], img.imageData);

    MPI_Finalize();
    return 0;
}
