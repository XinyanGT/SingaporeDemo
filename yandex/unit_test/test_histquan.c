#include <assert.h>
#include "histquan.h"


int main(int argc, char **argv) {
  //  char *filename = argv[1];
  //  char *varname = argv[2];
  
  assert(argc >= 2);

  HISTQUAN *hp;
  
  int level = atoi(argv[1]);
  printf("Quantization level: %d\n", level);
  
  int ratio = atoi(argv[2]);
  printf("Histogram finess: %d\n", ratio);

  int size = 20;
  float data[20] = {1.0, 3.1, 4.0, 4.1, 4.2, 4.8, 4.9, 5.1, 5.4, 5.5,
		    6.1, 6.3, 8.1, 8.2, 8.2, 8.3, 8.4, 9.9, 10.1, 11.2};
  int data_dc[20];
  float singleton = 5.4;
  int singleton_dc;
    
  hp = histquan_new(level, ratio);
  histquan_restart(hp, data, size);
  histquan_quantize(hp, data, data_dc, size);
  int i;
  printf("Before quantization: \n");
  for (i = 0; i < size; i++) {
    printf("%.2f ", data[i]);
  }
  printf("\n");
  printf("After quantization: \n");
  for (i = 0; i < size; i++) {
    printf("%d ", data_dc[i]);
  }
  printf("\n");
  
  printf("Quantize a singleton: %.3f\n", singleton);
  singleton_dc = histquan_quantize_singleton(hp, singleton);
  printf("Result: %d\n", singleton_dc);

  histquan_free(hp);
  return 0;
}
