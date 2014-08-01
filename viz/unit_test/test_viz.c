#include "viz.h"
#include "common.h"
#include <time.h>

int main() {

  int row = 256, col = 768;
  viz_init(row, col);
  float data[row * col];
  int i,j;
  srand(time(NULL));
  for (i = 0; i < row; i++) {
    for (j =0; j < col; j++) {
      data[i*col+j] = 1.0/ (rand() % 100 + 1);
      printf("%.2f\n", data[i*col+j]);
    }
  }

  viz_viz(data);
  
  return 0;

  
}
