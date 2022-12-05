//  ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦

int funcionQueCompara(const void *a, const void *b) {
  // Castear a enteros
  int aInt = *(int *) a;
  int bInt = *(int *) b;
  // Al restarlos, se debe obtener un número mayor, menor o igual a 0
  // Con esto ordenamos de manera ascendente
  return aInt - bInt;
}
//  ¦¦¦¦¦¦¦¦¦¦
int read_array(char* fname, int **arr) {
    FILE *myFile;
    unsigned int i, n;

    myFile = fopen(fname, "r");           if(   !myFile   ) {   printf("\nERROR: No se pudo abrir el archivo para lectura %s\n\n",fname);  MPI_Abort(MPI_COMM_WORLD, 99); }

    fscanf(myFile, "%i\n", &n); // numero de datoss a leer

    *arr = (int *) malloc(n*sizeof(int)); if (*arr == NULL) { printf("Memoria insuficiente\n"); MPI_Abort(MPI_COMM_WORLD, 99); return 0; }

    for ( i=0; i < n; i++)
        fscanf(myFile, "%i\n", (*arr)+i);

    fclose(myFile);

    return n;
}

//  ¦¦¦¦¦¦¦¦¦¦¦¦¦¦
void separaDatos_Pivote(int *bloque,unsigned int *cuantosMn,int **bloqueMenor, unsigned int *cuantosMy,int **bloqueMayor, int pivote, unsigned int n, int iteracion, int id){
	int i;
    *bloqueMenor = NULL;
    *bloqueMayor = NULL;

    *cuantosMn = 0;
    i=0;
	while(bloque[i]<pivote && i<n){
		(*cuantosMn) ++;		
        i++;
    }   

    if(*cuantosMn){
        *bloqueMenor =(int *)malloc((*cuantosMn)*sizeof(int));  if(*bloqueMenor==NULL){printf("\n(%d) ERROR: No hay memoria (*bloqueMenor)\n\n",iteracion);  MPI_Abort(MPI_COMM_WORLD, 99);}
    

        for(i=0;i<*cuantosMn;i++)
            (*bloqueMenor)[i] = bloque[i];
    }
    
    *cuantosMy = n-(*cuantosMn);
    if(*cuantosMy){
        *bloqueMayor =(int *)malloc((*cuantosMy)*sizeof(int));  if(*bloqueMayor==NULL){printf("\n(%d) ERROR: No hay memoria (*bloqueMayor)\n\n",iteracion);  MPI_Abort(MPI_COMM_WORLD, 99);}

        for(i=*cuantosMn;i<n;i++)
            (*bloqueMayor)[i-*cuantosMn]=bloque[i];
    }

	return;
}

//   ¦¦¦¦¦¦¦¦¦¦¦¦¦¦¦
int* mezclar_bloques(int *bloque1,int *bloque2,unsigned int n1,unsigned int n2){ // JOSÉ ALBERTO 
    int *aux;

    aux = (int *)malloc((n1+n2)*sizeof(int));   if(aux==NULL){printf("\n ERROR: No hay memoria (aux)\n\n");  MPI_Abort(MPI_COMM_WORLD, 99);}
    
    unsigned int a = 0; // Indice que recorre el bloque1
    unsigned int b = 0; // Indice que recorre el bloque2
    unsigned int i = 0;

    while (a < n1 && b < n2){
        if(bloque1[a] <= bloque2[b]){
            aux[i] = bloque1[a];
            a++;
        }else{
            aux[i] = bloque2[b];
            b++;
        }
        i++;
    }

    // Si el indice b llegó al final de su arreglo
    if(a == n1){
        // Asignamos los numeros restantes en el bloque2
        while(b < n2){
            aux[i] = bloque2[b];
            b++;
            i++;
        }
    }else   // Si el indice b llegó al final de su arreglo
        // Asignamos los numeros restantes en el bloque1
        while(a < n1){
            aux[i] = bloque1[a];
            a++;
            i++;
        }       

    return aux;
}
