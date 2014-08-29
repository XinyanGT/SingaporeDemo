#include "upsampler.h"

static void print_data(double *data, int row, int col) {
  int i, j;
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      printf("%.2f ", data[i*col+j]);
    }
    printf("\n");
  }
}

int main() {

  int dimy, dimx, scale, dimy2, dimx2;
  dimy = 4;
  dimx = 6;
  scale = 4;
  dimy2 = 16;
  dimx2 = 24;
  double I[4*6] =
    {1.1, 1.5, 2.2, 1.5, 1.4, 1.8,
     0.7, 2.1, 2.7, 2.0, 2.2, 1.4,
     3.5, 1.5, 0.2, 1.3, 2.6, 3.1,
     0.3, 1.7, 0.9, 3.2, 2.8, 2.6};
  double *I2 = (double *) malloc(dimy2 * dimx2 * sizeof(double));
  upsampler_upsample(dimy, dimx, I, scale, dimy2, dimx2, I2);

  /* int i, j; */
  printf("=======================================================\n");
  printf("Before upsampling: \n");
  print_data(I, dimy, dimx);
  
  printf("=======================================================\n");
  printf("After upsampling: \n");
  print_data(I2, dimy2, dimx2);
	 
  return 0;
  
}
