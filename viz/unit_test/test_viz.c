#include "viz.h"
#include "common.h"
#include <time.h>

int main() {

  int row = 20, col = 60;
  VIZ *vp;
  vp = viz_new(row, col, 0.6, -0.6, 200, 50);

  // Generate random data
  float data[row * col];
  int i,j;
  srand(time(NULL));
  for (i = 0; i < row; i++) {
    for (j =0; j < col; j++) {
      data[i*col+j] = 1.0/ (rand() % 100 + 1);
      printf("%.2f\n", data[i*col+j]);
    }
  }

  // Visualize
  viz_viz(vp, data);

  // Free
  viz_free(vp);
  
  return 0;

  
}
