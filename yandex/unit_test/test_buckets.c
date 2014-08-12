#include <assert.h>
#include "buckets.h"

int main(int argc, char **argv) {

  assert(argc > 3);
  int nbuckets = atoi(argv[1]);
  int low = atoi(argv[2]);
  int high = atoi(argv[3]);

  BUCKETS *bp;
  bp = buckets_new(nbuckets);

  // Fill buckets
  int size = 10;
  int min[10] = {0, 1, 5, 2, 8, 3, 3, 4, 6, 5};
  int max[10] = {1, 1, 5, 2, 8, 5, 4, 6, 6, 8};
  int result[10];
  int count;

  buckets_fill_range(bp, max, min, size);

  int i;
  printf("max:   ");
  for (i = 0; i < size; i++) {
    printf("%d ", max[i]);
  }
  printf("\n");
  
  printf("min:   ");
  for (i = 0; i < size; i++) {
    printf("%d ", min[i]);
  }
  printf("\n");

  printf("=======================================================\n");
  buckets_print(bp);

  // Extract from buckets
  printf("=======================================================\n");
  printf("Extract from buckets IN [%d, %d]\n", low, high);
  buckets_extract_in(bp, low, high, result, &count);
  printf("Result(%d): \n", count);
  for (i = 0; i < count; i++) {
    printf("%d ", result[i]);
  }
  printf("\n");

  printf("=======================================================\n");
  printf("Extract from buckets NOT IN [%d, %d]\n", low, high);
  buckets_extract_not_in(bp, low, high, result, &count);
  printf("Result(%d): \n", count);
  for (i = 0; i < count; i++) {
    printf("%d ", result[i]);
  }
  printf("\n");

   
  // Clear 
  buckets_free(bp);

  return 0;
}
  
