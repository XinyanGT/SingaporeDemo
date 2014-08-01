#include <assert.h>
#include "quantizer.h"


int main(int argc, char **argv) {
  //  char *filename = argv[1];
  //  char *varname = argv[2];
  //  assert(argc >= 2);
 
  int level = atoi(argv[1]);
  printf("Quantization level: %d\n", level);

  int size = 10;
  float data[10] = {1, 3, 4, 4.1, 6, 6.3, 8, 9, 10, 11};
  int data_dc[10];
  float singleton = 5.4;
  int singleton_dc;
    
  quantizer_init(level);
  quantizer_restart(data, size);
  quantizer_quantize(data, data_dc, size);
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
  singleton_dc = quantizer_quantize_singleton(singleton);
  printf("Result: %d\n", singleton_dc);

  quantizer_finalize();
  return 0;

}
