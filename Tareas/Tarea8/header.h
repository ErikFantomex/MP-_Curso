#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int isspace(int argument);

typedef struct {
	char version[3]; 
	int width;
	int height;
	int maxGrayLevel;
	int *imageData;
	int *gx;
	int *gy;
} pgm;

void init_out_image( pgm* out, pgm image){
	int i, j;
	strcpy(out->version, image.version);
	out->width = image.width;
	out->height = image.height;
	out->maxGrayLevel = image.maxGrayLevel;
	
	out->imageData = (int*) malloc(out->height * out->width * sizeof(int));

	
	out->gx = (int*) malloc(out->height * out->width *  sizeof(int));
	
	
	out->gy = (int*) malloc(out->height * out->width * sizeof(int));

	
	for(i = 0; i < out->height * out->width; i++) {
			out->imageData[i] = image.imageData[i];
			out->gx[i] = image.imageData[i];
			out->gy[i] = image.imageData[i];
		}
}

void read_comments(FILE *input_image) {
	char ch;
	char line[100];

	while ((ch = fgetc(input_image)) != EOF && (isspace(ch)))  {
		;
    }
	if (ch == '#') {
        fgets(line, sizeof(line), input_image);
    } 
	else {
		fseek(input_image, -2L, SEEK_CUR);
	}
}

void read_pgm_file(char* dir, pgm* image) {
	FILE* input_image; 
	int i, j, num;

	input_image = fopen(dir, "rb");
	if (input_image == NULL) {
		printf("File could not opened!");
		return;
	} 
	
	fgets(image->version, sizeof(image->version), input_image);
	read_comments(input_image);

	fscanf(input_image, "%d %d %d", &image->width, &image->height, &image->maxGrayLevel);
	
	image->imageData = (int*) malloc(image->height * image->width *  sizeof(int));
	
	if (strcmp(image->version, "P2") == 0) {
		for (i = 0; i < image->height * image->width; i++) {
				fscanf(input_image, "%d", &num);
				image->imageData[i] = num;
			
		}	
	}
	else if (strcmp(image->version, "P5") == 0) {
		char *buffer;
		int buffer_size = image->height * image->width;
		buffer = (char*) malloc( ( buffer_size + 1) * sizeof(char));
		
		if(buffer == NULL) {
			printf("Can not allocate memory for buffer! \n");
			return;
		}
		fread(buffer, sizeof(char), image->width * image-> height, input_image);
		for (i = 0; i < image->height * image ->width; i++) {
			image->imageData[i] = buffer[i];
		}
		free(buffer);
	}
	fclose(input_image);
	//printf("_______________IMAGE INFO__________________\n");
	//printf("Version: %s \nWidth: %d \nHeight: %d \nMaximum Gray Level: %d \n", image->version, image->width, image->height, image->maxGrayLevel);
}

void write_pgm_file(pgm* image, char dir[], int*matrix) {
    FILE* out_image;
    int i, j, count = 0;
    out_image = fopen(dir, "wb");
    fprintf(out_image, "%s\n", image->version);

    if (strcmp(image->version, "P2") == 0) {
        fprintf(out_image, "%d %d\n", image->width, image->height);
        fprintf(out_image, "%d\n", image->maxGrayLevel);        
        count =0;
        for(i = 0; i < image->height * image->width; i++) {
            fprintf(out_image,"%4d", matrix[i]);
            count++;
            if(count % image->width ==0) fprintf(out_image,"\n");
        }
        
        
    }
    else if (strcmp(image->version, "P5") == 0) {
        for(i = 0; i < image->height * image->width; i++) {
                char num = image->imageData[i];
                fprintf(out_image,"%c ", num);
            } 
    } 
    fclose(out_image);
}


int convolution(pgm image, int kernel[3][3], int row, int col) {
	int sum = 0;
	sum = image.imageData[(row-1)*image.width+(col-1)] * kernel[0][0] +
				image.imageData[(row-1)*image.width  + col] * kernel[0][1] +
				image.imageData[(row-1)*image.width + (col+1)] * kernel[0][2] +
				image.imageData[row*image.width +(col-1)] * kernel[1][0] +
				image.imageData[row*image.width + col] * kernel[1][1] +
				image.imageData[row*image.width + (col+1)] * kernel[1][2] +
				image.imageData[(row+1)*image.width + (col-1)] * kernel[2][0] +
				image.imageData[(row+1)*image.width + col] * kernel[2][1] +
				image.imageData[(row+1)*image.width + (col+1)] * kernel[2][2];
	
	return sum;
}



void sobel_edge_detector(pgm image, pgm* out_image) {
	int i, j, k, gx, gy;
	int mx[3][3] = {
		{-1, 0, 1},
		{-2, 0, 2},
		{-1, 0, 1}
	};
	int my[3][3] = {
		{-1, -2, -1},
		{0, 0, 0},
		{1, 2, 1}
	};
	k = image.width;
	for (i = 1; i < (image.height - 1); i++) {
		for (j = 1; j < (image.width - 1); j++) {
      if (j==1)k++;
			gx = convolution(image, mx, i, j);
			gy = convolution(image, my, i, j);
			out_image->imageData[k] = sqrt(gx*gx + gy*gy);
			out_image->gx[k] = gx;
			out_image->gy[k] = gy;
      k++;
		}
    k++;
	}
	
}

