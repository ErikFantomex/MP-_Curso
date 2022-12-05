#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define MIN(a,b)           ((a)<(b)?(a):(b))
#define BLOCK_LOW(id,p,n)  ((id)*(n)/(p))
#define BLOCK_HIGH(id,p,n) (BLOCK_LOW((id)+1,p,n)-1)
#define BLOCK_SIZE(id,p,n) (BLOCK_HIGH(id,p,n)-BLOCK_LOW(id,p,n)+1)

//Estructura para guardar los valores de colores

void generaImagen_salida(int *img_rojo,int* img_verde,int *img_azul, int m, int n, int protocolo, char numero, char formato, char *nombre){
 	int r, g, b;
   int i;
  FILE *newImagePointer;

	newImagePointer = fopen(nombre, "w");
	fprintf(newImagePointer, "%c%c \n%i %i\n%i\n", formato, numero, n, m, protocolo);

	for (i = 0; i < m * n; i++){	
    if (i %n == 0) fprintf(newImagePointer,"\n");
		fprintf(newImagePointer, "%i %i %i  ", img_rojo[i], img_verde[i], img_azul[i]);
    
  }

	
	fclose(newImagePointer);
}

int main (int argc, char *argv[]) {
  char formato, numero;	//formato y numero se leen del archivo. 
	int height, width, protocolo; //m y n son las dimensiones de la imagen, protocolo se lee del archivo	
  int *img_rojo,*copia_img_rojo;
  int *img_verde,*copia_img_verde;
  int *img_azul,*copia_img_azul;
  int id, p;
	int *sub_r, *sub_g, *sub_b;
  int *sprimeraFila_r, *sprimeraFila_g,*sprimeraFila_b;
  int *sultimaFila_r,*sultimaFila_g,*sultimaFila_b;
  int *rultimaFila_r,*rultimaFila_g,*rultimaFila_b;
  int *rprimeraFila_r,*rprimeraFila_g,*rprimeraFila_b;
  int g_x_r, g_x_g, g_x_b;
  int g_y_r, g_y_g, g_y_b;
  int G_r,G_g,G_b;
	int *recvcounts, *displs, sendcount;
  int i,j,k;
  FILE *imageFilePointer;
  int local_rows, low,high;

    float sobel_x[3][3] = {{-1, 0, 1},
                           {-2, 0, 2},
                           {-1, 0, 1}};

    float sobel_y[3][3] = {{-1, -2, -1},
                           {0, 0, 0},
                           {1, 2, 1}};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);

    imageFilePointer = fopen(argv[1], "r");

  if (imageFilePointer == NULL){
    printf("\n[!] ERROR: Algo salio mal al tratar de abrir el archivo %s\n", argv[1]);
    return 1;
  }

  //printf("[!] Archivo %s abierto con exito!", argv[1]);

  //Se leen los dos primeros caracteres para luego imprimirlo en un nuevo archivo
  fscanf(imageFilePointer, "%c", &formato);
  fscanf(imageFilePointer, "%c", &numero);

  //Se capturan dimensiones de imagen y protocolo de colores
  fscanf(imageFilePointer, "%i", &width);
  fscanf(imageFilePointer, "%i", &height);
  fscanf(imageFilePointer, "%i", &protocolo);

  //Request de memoria para guardar imagen en memoria, separada por colores
  img_rojo  = (int*)malloc(height*width*sizeof(int));  if(img_rojo==NULL){     printf("Sin memoria (img_rojo) \n"); return 1; }
  img_verde = (int*)malloc(height*width*sizeof(int));  if(img_verde==NULL){     printf("Sin memoria (img_verde) \n"); return 1; }
  img_azul  = (int*)malloc(height*width*sizeof(int));  if(img_azul==NULL){     printf("Sin memoria (img_azul) \n"); return 1; }


  //Empieza la captura de colores por pixel en la imagen
  for (i = 0; i < height * width; i++){
    fscanf(imageFilePointer, "%i ", &img_rojo[i]);
    fscanf(imageFilePointer, "%i ", &img_verde[i]);
    fscanf(imageFilePointer, "%i ", &img_azul[i]);
  }    
  fclose(imageFilePointer);

    // -------------------------------------------------------------------------- PROCESADMIENTO DE LA IMAGEN AQUI
    sprimeraFila_r = (int *)malloc(width * sizeof(int));  if( sprimeraFila_r == NULL ){ printf("ERROR: Memoria insuficiente (sprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); }     
    sprimeraFila_g = (int *)malloc(width * sizeof(int));  if( sprimeraFila_g == NULL ){ printf("ERROR: Memoria insuficiente (sprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); }     
    sprimeraFila_b = (int *)malloc(width * sizeof(int));  if( sprimeraFila_b == NULL ){ printf("ERROR: Memoria insuficiente (sprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); }    

    rprimeraFila_r = (int *)malloc(width * sizeof(int));  if( rprimeraFila_r == NULL ){ printf("ERROR: Memoria insuficiente (rprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    rprimeraFila_g = (int *)malloc(width * sizeof(int));  if( rprimeraFila_g == NULL ){ printf("ERROR: Memoria insuficiente (rprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    rprimeraFila_b = (int *)malloc(width * sizeof(int));  if( rprimeraFila_b == NULL ){ printf("ERROR: Memoria insuficiente (rprimeraFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 

    sultimaFila_r  = (int *)malloc(width * sizeof(int));  if( sultimaFila_r == NULL ){ printf("ERROR: Memoria insuficiente (sultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    sultimaFila_g  = (int *)malloc(width * sizeof(int));  if( sultimaFila_g == NULL ){ printf("ERROR: Memoria insuficiente (sultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    sultimaFila_b  = (int *)malloc(width * sizeof(int));  if( sultimaFila_b == NULL ){ printf("ERROR: Memoria insuficiente (sultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 

    rultimaFila_r  = (int *)malloc(width * sizeof(int));  if( rultimaFila_r == NULL ){ printf("ERROR: Memoria insuficiente (rultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    rultimaFila_g  = (int *)malloc(width * sizeof(int));  if( rultimaFila_g == NULL ){ printf("ERROR: Memoria insuficiente (rultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    rultimaFila_b  = (int *)malloc(width * sizeof(int));  if( rultimaFila_b == NULL ){ printf("ERROR: Memoria insuficiente (rultimaFila)"); MPI_Abort(MPI_COMM_WORLD,99); } 

    low = BLOCK_LOW(id, p, height);
    high = BLOCK_HIGH(id, p, height) + 1;
    local_rows = BLOCK_SIZE(id, p, height);
    
    sub_r   = (int *)malloc(width * local_rows * sizeof(int)); if( sub_r   == NULL ){ printf("ERROR: Memoria insuficiente (sub)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    sub_g   = (int *)malloc(width * local_rows * sizeof(int)); if( sub_g   == NULL ){ printf("ERROR: Memoria insuficiente (sub)"); MPI_Abort(MPI_COMM_WORLD,99); } 
    sub_b   = (int *)malloc(width * local_rows * sizeof(int)); if( sub_b   == NULL ){ printf("ERROR: Memoria insuficiente (sub)"); MPI_Abort(MPI_COMM_WORLD,99); } 

  copia_img_rojo = (int *)malloc(width * local_rows * sizeof(int)); if( copia_img_rojo == NULL ){ printf("ERROR: Memoria insuficiente (copia)"); MPI_Abort(MPI_COMM_WORLD,99); } 
  copia_img_verde = (int *)malloc(width * local_rows * sizeof(int)); if( copia_img_verde == NULL ){ printf("ERROR: Memoria insuficiente (copia)"); MPI_Abort(MPI_COMM_WORLD,99); } 
  copia_img_azul = (int *)malloc(width * local_rows * sizeof(int)); if( copia_img_azul == NULL ){ printf("ERROR: Memoria insuficiente (copia)"); MPI_Abort(MPI_COMM_WORLD,99); } 

  for (i = low, k = 0; i < high; i++, k++){
        for (j = 0; j < width; j++){
            sub_r[k * width + j] = img_rojo[i * width + j];
            sub_g[k * width + j] = img_verde[i * width + j];
            sub_b[k * width + j] = img_azul[i * width + j];

            copia_img_rojo[k * width + j] = img_rojo[i * width + j];
            copia_img_verde[k * width + j] = img_verde[i * width + j];
            copia_img_azul[k * width + j] = img_azul[i * width + j];
        }
    }


    for (i = 0; i < width; i++){      
      sultimaFila_r[i] = sub_r[(local_rows - 1) * width + i];
      sultimaFila_g[i] = sub_g[(local_rows - 1) * width + i];
      sultimaFila_b[i] = sub_b[(local_rows - 1) * width + i];
    }

    
    if (id != p - 1){ 
      MPI_Send(sultimaFila_r, width, MPI_INT, id + 1, 1, MPI_COMM_WORLD);
      MPI_Send(sultimaFila_g, width, MPI_INT, id + 1, 2, MPI_COMM_WORLD);
      MPI_Send(sultimaFila_b, width, MPI_INT, id + 1, 3, MPI_COMM_WORLD);
    }
    if (id != 0 ){ 
      MPI_Recv(rprimeraFila_r, width, MPI_INT, id - 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  
      MPI_Recv(rprimeraFila_g, width, MPI_INT, id - 1, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);  
      MPI_Recv(rprimeraFila_b, width, MPI_INT, id - 1, 3, MPI_COMM_WORLD, MPI_STATUS_IGNORE); 
    }
    
    
    for (i = 0; i < width; i++){      
      sprimeraFila_r[i] = sub_r[i];
      sprimeraFila_g[i] = sub_g[i];
      sprimeraFila_b[i] = sub_b[i];
    }

    if (id!=0){ 
      MPI_Send(sprimeraFila_r, width, MPI_INT, id - 1, 4, MPI_COMM_WORLD);
      MPI_Send(sprimeraFila_g, width, MPI_INT, id - 1, 5, MPI_COMM_WORLD);
      MPI_Send(sprimeraFila_b, width, MPI_INT, id - 1, 6, MPI_COMM_WORLD);
    }
    if (id!=p-1){
      MPI_Recv(rultimaFila_r, width, MPI_INT, id + 1, 4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(rultimaFila_g, width, MPI_INT, id + 1, 5, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(rultimaFila_b, width, MPI_INT, id + 1, 6, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

   

      for (i = 0; i < local_rows; i++){
        for (j = 0; j < width; j++){
            g_x_r = 0;
            g_y_r = 0;

            g_x_g = 0;
            g_y_g = 0;

            g_x_b = 0;
            g_y_b = 0;

            /*        PRIMER RENGLON        */
            if (i == 0){ // Si me encuentro en el primero renglón
                if (id != 0){ // Si no soy el cero
                    if (j != 0){ // Si no me encuentro en la primera columna hago los de la izquierda
                        g_x_r += (sobel_x[0][0] * rprimeraFila_r[j - 1]);
                        g_x_g += (sobel_x[0][0] * rprimeraFila_g[j - 1]);
                        g_x_b += (sobel_x[0][0] * rprimeraFila_b[j - 1]);

                        g_y_r += (sobel_y[0][0] * rprimeraFila_r[j - 1]);
                        g_y_g += (sobel_y[0][0] * rprimeraFila_g[j - 1]);
                        g_y_b += (sobel_y[0][0] * rprimeraFila_b[j - 1]);
                    }
                    g_x_r += (sobel_x[0][1] * rprimeraFila_r[j]);
                    g_x_g += (sobel_x[0][1] * rprimeraFila_g[j]);
                    g_x_b += (sobel_x[0][1] * rprimeraFila_b[j]);

                    g_y_r += (sobel_y[0][1] * rprimeraFila_r[j]);
                    g_y_g += (sobel_y[0][1] * rprimeraFila_g[j]);
                    g_y_b += (sobel_y[0][1] * rprimeraFila_b[j]);

                    if (j != width - 1){ // Si no me encuentro en la última columna hago los de la derecha
                    
                        g_x_r += (sobel_x[0][2] * rprimeraFila_r[j + 1]);
                        g_x_g += (sobel_x[0][2] * rprimeraFila_g[j + 1]);
                        g_x_b += (sobel_x[0][2] * rprimeraFila_b[j + 1]);

                        g_y_r += (sobel_y[0][2] * rprimeraFila_r[j + 1]);
                        g_y_g += (sobel_y[0][2] * rprimeraFila_g[j + 1]);
                        g_y_b += (sobel_y[0][2] * rprimeraFila_b[j + 1]);
                    }
                }
                if (j != 0){ // Si no me encuentro en la primera columna hago los de la izquiersa
                    g_x_r += (sobel_x[2][0] * copia_img_rojo[width * (i + 1) + (j - 1)]);
                    g_x_g += (sobel_x[2][0] * copia_img_verde[width * (i + 1) + (j - 1)]);
                    g_x_b += (sobel_x[2][0] * copia_img_azul[width * (i + 1) + (j - 1)]);

                    g_y_r += (sobel_y[2][0] * copia_img_rojo[width * (i + 1) + (j - 1)]);
                    g_y_g += (sobel_y[2][0] * copia_img_verde[width * (i + 1) + (j - 1)]);
                    g_y_b += (sobel_y[2][0] * copia_img_azul[width * (i + 1) + (j - 1)]);
                }
                g_x_r += (sobel_x[2][1] * copia_img_rojo[width * (i + 1) + j]);
                g_x_g += (sobel_x[2][1] * copia_img_verde[width * (i + 1) + j]);
                g_x_b += (sobel_x[2][1] * copia_img_azul[width * (i + 1) + j]);

                g_y_r += (sobel_y[2][1] * copia_img_rojo[width * (i + 1) + j]);
                g_y_g += (sobel_y[2][1] * copia_img_verde[width * (i + 1) + j]);
                g_y_b += (sobel_y[2][1] * copia_img_azul[width * (i + 1) + j]);

                if (j != width - 1){ // Si no me encuentro en la última columna hago los de la derecha
                    g_x_r += (sobel_x[2][2] * copia_img_rojo[width * (i + 1) + (j + 1)]);
                    g_x_g += (sobel_x[2][2] * copia_img_verde[width * (i + 1) + (j + 1)]);
                    g_x_b += (sobel_x[2][2] * copia_img_azul[width * (i + 1) + (j + 1)]);

                    g_y_r += (sobel_y[2][2] * copia_img_rojo[width * (i + 1) + (j + 1)]);
                    g_y_g += (sobel_y[2][2] * copia_img_verde[width * (i + 1) + (j + 1)]);
                    g_y_b += (sobel_y[2][2] * copia_img_azul[width * (i + 1) + (j + 1)]);
                }
            /*   ULTIMO RENGLÓN   */
            }else if (i == local_rows - 1){
                if (id != p - 1){ // Si no soy el ultimo proceso
                    if (j!= 0)
                    {
                        g_x_r += (sobel_x[2][0] * rultimaFila_r[j - 1]);
                        g_x_g += (sobel_x[2][0] * rultimaFila_g[j - 1]);
                        g_x_b += (sobel_x[2][0] * rultimaFila_b[j - 1]);
                     
                        g_y_r += (sobel_y[2][0] * rultimaFila_r[j - 1]);
                        g_y_g += (sobel_y[2][0] * rultimaFila_g[j - 1]);
                        g_y_b += (sobel_y[2][0] * rultimaFila_b[j - 1]);
                    }
                    g_x_r += (sobel_x[2][1] * rultimaFila_r[j]);
                    g_x_g += (sobel_x[2][1] * rultimaFila_g[j]);
                    g_x_b += (sobel_x[2][1] * rultimaFila_b[j]);

                    g_y_r += (sobel_y[2][1] * rultimaFila_r[j]);
                    g_y_g += (sobel_y[2][1] * rultimaFila_g[j]);
                    g_y_b += (sobel_y[2][1] * rultimaFila_b[j]);

                    if (j != width - 1) // Si no soy la última columna hago los de la derecha
                    {
                        g_x_r += (sobel_x[2][2] * rultimaFila_r[j + 1]);
                        g_x_g += (sobel_x[2][2] * rultimaFila_g[j + 1]);
                        g_x_b += (sobel_x[2][2] * rultimaFila_b[j + 1]);
                     
                        g_y_r += (sobel_y[2][2] * rultimaFila_r[j + 1]);
                        g_y_g += (sobel_y[2][2] * rultimaFila_g[j + 1]);
                        g_y_b += (sobel_y[2][2] * rultimaFila_b[j + 1]);                     
                    }
                }
                if (j!= 0) // Si no soy la primera columna hago los de la izquierda
                {
                    g_x_r += (sobel_x[0][0] * copia_img_rojo[width * (i - 1) + (j - 1)]);
                    g_x_g += (sobel_x[0][0] * copia_img_verde[width * (i - 1) + (j - 1)]);
                    g_x_b += (sobel_x[0][0] * copia_img_azul[width * (i - 1) + (j - 1)]);
                 
                    g_y_r += (sobel_y[0][0] * copia_img_rojo[width * (i - 1) + (j - 1)]);
                    g_y_g += (sobel_y[0][0] * copia_img_verde[width * (i - 1) + (j - 1)]);
                    g_y_b += (sobel_y[0][0] * copia_img_azul[width * (i - 1) + (j - 1)]);
                }
                g_x_r += (sobel_x[0][1] * copia_img_rojo[width * (i - 1) + j]);
                g_x_g += (sobel_x[0][1] * copia_img_verde[width * (i - 1) + j]);
                g_x_b += (sobel_x[0][1] * copia_img_azul[width * (i - 1) + j]);

                g_y_r += (sobel_y[0][1] * copia_img_rojo[width * (i - 1) + j]);
                g_y_g += (sobel_y[0][1] * copia_img_verde[width * (i - 1) + j]);
                g_y_b += (sobel_y[0][1] * copia_img_azul[width * (i - 1) + j]);
                if (j != width - 1){ 
                    g_x_r += (sobel_x[0][2] * copia_img_rojo[width * (i - 1) + (j + 1)]);
                    g_x_g += (sobel_x[0][2] * copia_img_verde[width * (i - 1) + (j + 1)]);
                    g_x_b += (sobel_x[0][2] * copia_img_azul[width * (i - 1) + (j + 1)]);

                    g_y_r += (sobel_y[0][2] * copia_img_rojo[width * (i - 1) + (j + 1)]);
                    g_y_g += (sobel_y[0][2] * copia_img_verde[width * (i - 1) + (j + 1)]);
                    g_y_b += (sobel_y[0][2] * copia_img_azul[width * (i - 1) + (j + 1)]);
                }
            } /*  EL RESTO DE LOS RENGLONES  */
            else{
                if (j != 0) { // Si no soy la primera columna
                    g_x_r += (sobel_x[0][0] * copia_img_rojo[width * (i - 1) + (j - 1)]);
                    g_x_g += (sobel_x[0][0] * copia_img_verde[width * (i - 1) + (j - 1)]);
                    g_x_b += (sobel_x[0][0] * copia_img_azul[width * (i - 1) + (j - 1)]);

                    g_y_r += (sobel_y[0][0] * copia_img_rojo[width * (i - 1) + (j - 1)]);
                    g_y_g += (sobel_y[0][0] * copia_img_verde[width * (i - 1) + (j - 1)]);
                    g_y_b += (sobel_y[0][0] * copia_img_azul[width * (i - 1) + (j - 1)]);

                    g_x_r += (sobel_x[2][0] * copia_img_rojo[width * (i + 1) + (j - 1)]);
                    g_x_g += (sobel_x[2][0] * copia_img_verde[width * (i + 1) + (j - 1)]);
                    g_x_b += (sobel_x[2][0] * copia_img_azul[width * (i + 1) + (j - 1)]);

                    g_y_r += (sobel_y[2][0] * copia_img_rojo[width * (i + 1) + (j - 1)]);
                    g_y_g += (sobel_y[2][0] * copia_img_verde[width * (i + 1) + (j - 1)]);
                    g_y_b += (sobel_y[2][0] * copia_img_azul[width * (i + 1) + (j - 1)]);
                }
                if (j != width - 1)
                {
                    g_x_r += (sobel_x[0][2] * copia_img_rojo[width * (i - 1) + (j + 1)]);
                    g_x_g += (sobel_x[0][2] * copia_img_verde[width * (i - 1) + (j + 1)]);
                    g_x_b += (sobel_x[0][2] * copia_img_azul[width * (i - 1) + (j + 1)]);
                 
                    g_y_r += (sobel_y[0][2] * copia_img_rojo[width * (i - 1) + (j + 1)]);
                    g_y_g += (sobel_y[0][2] * copia_img_verde[width * (i - 1) + (j + 1)]);
                    g_y_b += (sobel_y[0][2] * copia_img_azul[width * (i - 1) + (j + 1)]);
                 
                    g_x_r += (sobel_x[2][2] * copia_img_rojo[width * (i + 1) + (j + 1)]);
                    g_x_g += (sobel_x[2][2] * copia_img_verde[width * (i + 1) + (j + 1)]);
                    g_x_b += (sobel_x[2][2] * copia_img_azul[width * (i + 1) + (j + 1)]);
                 
                    g_y_r += (sobel_y[2][2] * copia_img_rojo[width * (i + 1) + (j + 1)]);
                    g_y_g += (sobel_y[2][2] * copia_img_verde[width * (i + 1) + (j + 1)]);
                    g_y_b += (sobel_y[2][2] * copia_img_azul[width * (i + 1) + (j + 1)]);
                }
                g_x_r += (sobel_x[0][1] * copia_img_rojo[width * (i - 1) + j]);
                g_x_g += (sobel_x[0][1] * copia_img_verde[width * (i - 1) + j]);
                g_x_b += (sobel_x[0][1] * copia_img_azul[width * (i - 1) + j]);
                 
                g_y_r += (sobel_y[0][1] * copia_img_rojo[width * (i - 1) + j]);
                g_y_g += (sobel_y[0][1] * copia_img_verde[width * (i - 1) + j]);
                g_y_b += (sobel_y[0][1] * copia_img_azul[width * (i - 1) + j]);
                 
                g_x_r += (sobel_x[2][1] * copia_img_rojo[width * (i + 1) + j]);
                g_x_g += (sobel_x[2][1] * copia_img_verde[width * (i + 1) + j]);
                g_x_b += (sobel_x[2][1] * copia_img_azul[width * (i + 1) + j]);
                 
                g_y_r += (sobel_y[2][1] * copia_img_rojo[width * (i + 1) + j]);
                g_y_g += (sobel_y[2][1] * copia_img_verde[width * (i + 1) + j]);
                g_y_b += (sobel_y[2][1] * copia_img_azul[width * (i + 1) + j]);
            }

            if (j != 0)
            {
                g_x_r += (sobel_x[1][0] * copia_img_rojo[width * i + (j - 1)]);
                g_x_g += (sobel_x[1][0] * copia_img_verde[width * i + (j - 1)]);
                g_x_b += (sobel_x[1][0] * copia_img_azul[width * i + (j - 1)]);
             
                g_y_r += (sobel_y[1][0] * copia_img_rojo[width * i + (j - 1)]);
                g_y_g += (sobel_y[1][0] * copia_img_verde[width * i + (j - 1)]);
                g_y_b += (sobel_y[1][0] * copia_img_azul[width * i + (j - 1)]);
            }
            g_x_r += (sobel_x[1][1] * copia_img_rojo[width * i + j]);
            g_x_g += (sobel_x[1][1] * copia_img_verde[width * i + j]);
            g_x_b += (sobel_x[1][1] * copia_img_azul[width * i + j]);

            g_y_r += (sobel_y[1][1] * copia_img_rojo[width * i + j]);
            g_y_g += (sobel_y[1][1] * copia_img_verde[width * i + j]);
            g_y_b += (sobel_y[1][1] * copia_img_azul[width * i + j]);
            if (j != width - 1)
            {
                g_x_r += (sobel_x[1][2] * copia_img_rojo[width * i + (j + 1)]);
                g_x_g += (sobel_x[1][2] * copia_img_verde[width * i + (j + 1)]);
                g_x_b += (sobel_x[1][2] * copia_img_azul[width * i + (j + 1)]);
             
                g_y_r += (sobel_y[1][2] * copia_img_rojo[width * i + (j + 1)]);
                g_y_g += (sobel_y[1][2] * copia_img_verde[width * i + (j + 1)]);
                g_y_b += (sobel_y[1][2] * copia_img_azul[width * i + (j + 1)]);
            }

            G_r = (int)sqrt((g_x_r * g_x_r) + (g_y_r * g_y_r));
            G_g = (int)sqrt((g_x_g * g_x_g) + (g_y_g * g_y_g));
            G_b = (int)sqrt((g_x_b * g_x_b) + (g_y_b * g_y_b));

            if (G_r < 0)
                G_r = 0;
            if (G_r > 255)
                G_r = 255;
            if (G_g < 0)
                G_g = 0;
            if (G_g > 255)
                G_g = 255;
            if (G_b < 0)
                G_b = 0;
            if (G_b > 255)
                G_b = 255;

            sub_r[i * width + j] = G_r;
            //printf("[%d]%d\t",id,sub_r[i*width+j]);
            sub_g[i * width + j] = G_g;
            //printf("[%d]%d\t",id,sub_g[i*width+j]);
            sub_b[i * width + j] = G_b;
        }
        //printf("\n");
    }

    // Generamos el arreglo de recvcounts
		if(!id){
			recvcounts = (int*) malloc(p * sizeof(int));
			displs  = (int*) malloc(p * sizeof(int));	
		}

    sendcount = local_rows * width;
		MPI_Gather(&sendcount, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);	

		// Generamos el arreglo de diplacements
		if(!id){
			displs[0]=0; 	

			for(i=1;i<p;i++){ 
				displs[i]=displs[i-1]+recvcounts[i-1];
			}
		}


		MPI_Gatherv(sub_r, local_rows*width, MPI_INT, img_rojo, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Gatherv(sub_g, local_rows*width, MPI_INT, img_verde, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Gatherv(sub_b, local_rows*width, MPI_INT, img_azul, recvcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);

    if(!id){
        int *imgFinalR = (int*)malloc((width)*(height)*sizeof(int));
        int *imgFinalG = (int*)malloc((width)*(height)*sizeof(int));
        int *imgFinalB = (int*)malloc((width)*(height)*sizeof(int));
			for(i = 0; i < width*height;i++){
				imgFinalR[i] = img_rojo[i]; 
				imgFinalG[i] = img_verde[i]; 
				imgFinalB[i] = img_azul[i]; 
			}
      generaImagen_salida(imgFinalR, imgFinalG, imgFinalB, height, width, protocolo, numero, formato, argv[2]);

      free(imgFinalR);
      free(imgFinalG);
      free(imgFinalB);
		}

    free(img_rojo);free(copia_img_rojo);
    free(img_verde);free(copia_img_verde);
    free(img_azul);free(copia_img_azul);
    free(sub_r);free(sub_g); free(sub_b);
		if(!id)
		{free(recvcounts);
			free(displs);
		}




  MPI_Finalize();
  return 0;
}
