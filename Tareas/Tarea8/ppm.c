#include <stdlib.h>
#include <stdio.h>
#include <math.h>

//Estructura para guardar los valores de colores

void generaImagen_salida(int *img_rojo,int* img_verde,int *img_azul, int m, int n, int protocolo, char numero, char formato, char *nombre){
 	int r, g, b;
  FILE *newImagePointer;

	newImagePointer = fopen(nombre, "w");
	fprintf(newImagePointer, "%c%c \n%i %i\n%i\n", formato, numero, m, n, protocolo);

	for (int i = 0; i < m * n; i++)	
		fprintf(newImagePointer, "%i %i %i\n", img_rojo[i], img_verde[i], img_azul[i]);
	
	fclose(newImagePointer);
}

int convolucion(int *color, int m, int n, int *Gx, int *Gy, int *imgFinal){
  int kernel = 3;
  int i,j;
  int aux = 0;
  int auxC = 0;
  int multGx = 0, multGy=0;
  int sumNumGx = 0, sumNumGy = 0;
  int movDer=0, auxMovDer=0, movBajo;
  int pakBaje=0;
  int G = 0;
  int count=0;
  for(movBajo=0;abs(movBajo - n)>=kernel; movBajo++){
    for(movDer=0; abs(movDer - m)>=kernel; movDer++, count++){
      for(i=0; i < kernel; i++,aux+=m, auxC+=3){
        for(j=0; j < kernel; j++){
            //printf("%d ",color[aux+j+movDer + pakBaje]);
            multGx = color[aux+j+movDer + pakBaje] * Gx[auxC+j];
            multGy = color[aux+j+movDer + pakBaje] * Gy[auxC+j];
            sumNumGx = sumNumGx + multGx;
            sumNumGy = sumNumGy + multGy;
        }
        //printf("\n");
      }
      
      G = sqrt((pow((double)sumNumGx,2)) + (pow((double)sumNumGy,2)));
      if (G > 255) G = 255;
      //printf("sumNumGx = %d \n", sumNumGx);
      //printf("sumNumGy = %d \n", sumNumGy);
      //printf("G = %d \n\n", G);
      
      imgFinal[count] = G;

      aux = 0;
      auxC = 0;
      sumNumGx = 0;
      sumNumGy = 0;
      G = 0;
    }
    pakBaje +=m;
  }
}


int main(int argc, char ** argv){	
	char formato, numero;	//formato y numero se leen del archivo. 
	int m, n, protocolo; //m y n son las dimensiones de la imagen, protocolo se lee del archivo	
  int *img_rojo;
  int *img_verde;
  int *img_azul;
  FILE *imageFilePointer;
  
  imageFilePointer = fopen(argv[1], "r");

  if (imageFilePointer == NULL){
    printf("\n[!] ERROR: Algo salio mal al tratar de abrir el archivo %s\n", argv[1]);
    return 1;
  }

  printf("[!] Archivo %s abierto con exito!", argv[1]);

  //Se leen los dos primeros caracteres para luego imprimirlo en un nuevo archivo
  fscanf(imageFilePointer, "%c", &formato);
  fscanf(imageFilePointer, "%c", &numero);

  //Se capturan dimensiones de imagen y protocolo de colores
  fscanf(imageFilePointer, "%i", &m);
  fscanf(imageFilePointer, "%i", &n);
  fscanf(imageFilePointer, "%i", &protocolo);

  //Request de memoria para guardar imagen en memoria, separada por colores
  img_rojo  = (int*)malloc(m*n*sizeof(int));  if(img_rojo==NULL){     printf("Sin memoria (img_rojo) \n"); return 1; }
  img_verde = (int*)malloc(m*n*sizeof(int));  if(img_verde==NULL){     printf("Sin memoria (img_verde) \n"); return 1; }
  img_azul  = (int*)malloc(m*n*sizeof(int));  if(img_azul==NULL){     printf("Sin memoria (img_azul) \n"); return 1; }
  

  //Empieza la captura de colores por pixel en la imagen
  for (int i = 0; i < m * n; i++){
    fscanf(imageFilePointer, "%i ", &img_rojo[i]);
    fscanf(imageFilePointer, "%i ", &img_verde[i]);
    fscanf(imageFilePointer, "%i ", &img_azul[i]);
  }    
  fclose(imageFilePointer);

  // inicia el procesamiento de la imagen

   int Gx[] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
  int Gy[] = {1,2,1,0,0,0,-1,-2,-1};

  int *imgFinalR = (int*)malloc((m-2)*(n-2)*sizeof(int));
  int *imgFinalG = (int*)malloc((m-2)*(n-2)*sizeof(int));
  int *imgFinalB = (int*)malloc((m-2)*(n-2)*sizeof(int));

  convolucion(img_rojo, m, n, Gx, Gy, imgFinalR);
  convolucion(img_verde, m, n, Gx, Gy, imgFinalG);
  convolucion(img_azul, m, n, Gx, Gy, imgFinalB);
  generaImagen_salida(imgFinalR, imgFinalG, imgFinalB, (m-2), (n-2), protocolo, numero, formato, argv[2]);


  free(img_rojo);
  free(img_verde);
  free(img_azul);
  free(imgFinalR);
  free(imgFinalG);
  free(imgFinalB);
  return 0;

}


