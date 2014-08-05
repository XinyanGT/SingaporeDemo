#include <assert.h>
#include "hquantizer.h"


int main(int argc, char **argv) {
  //  char *filename = argv[1];
  //  char *varname = argv[2];
  //  assert(argc >= 2);

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
    
  hquantizer_init(level, ratio);
  hquantizer_restart(data, size);
  hquantizer_quantize(data, data_dc, size);
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
  singleton_dc = hquantizer_quantize_singleton(singleton);
  printf("Result: %d\n", singleton_dc);

  hquantizer_finalize();
  return 0;
}
