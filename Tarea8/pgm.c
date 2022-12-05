#include "header.h"

int main(int argc, char **argv)
{
	pgm image, out_image;
	char *input_dir, *output_dir;
  input_dir = argv[1];
  output_dir = argv[2];
  int i,j;
	
	read_pgm_file(input_dir, &image);
  	init_out_image(&out_image, image);
  	sobel_edge_detector(image, &out_image);
 
  write_pgm_file(&out_image,output_dir, out_image.imageData);

  return 0;
}
