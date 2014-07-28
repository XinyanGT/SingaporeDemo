/************************************************************
 * Created: Jul 14, 2014
 * Author:  Xinyan Yan
 * Brief:   Buckets that contain integers. In indexing case,
            the integers are the positions of records that
            fall in certain range.
 ************************************************************/

#include "buckets.h"

static int **s_buckets;
static int *s_bucket_nelems;  // current number of elements in each bucket
static int s_nbuckets;
static int s_bucket_size;    // the maximal number of elements each bucket can hold
static int s_effetive_nelems;
static int *s_a;   // used in buckets_extract
static int *s_b;

void buckets_init(int nbuckets, int size) {
  
  // Allocate space for buckets and bucket sizes
  s_bucket_nelems = (int *) calloc(nbuckets, sizeof(int));
  s_buckets = (int **) malloc(nbuckets * sizeof(int*));
  int i;
  for (i = 0; i < nbuckets; i++) {
    s_buckets[i] = (int *) malloc(size * sizeof(int));
  }
  
  // Store information
  s_nbuckets = nbuckets;
  s_bucket_size = size;
}

void buckets_start() {
  int i;
  for (i = 0; i < s_nbuckets; i++) {
    s_bucket_nelems[i] = 0;
  }
  s_effetive_nelems = 0;
}

void buckets_finalize() {
  int i;
  for (i = 0; i < s_nbuckets; i++) {
    free(s_buckets[i]);
  }
  free(s_buckets);
  free(s_bucket_nelems);
  free(s_a);
  free(s_b);
}

// Valid values in max and min are integers from 0 to s_nbuckets-1
void buckets_fill_range(int *max, int *min, int size) {

  int i, j;
  
  /* printf("size: %d\n", size); */
  /* printf("max: \n"); */


  /* for (i = 0; i < size; i++) { */
  /*   printf("%d ", max[i]); */
  /* } */
  /* printf("\n"); */
  /* printf("min: \n"); */
  /* for (i = 0; i < size; i++) { */
  /*   printf("%d ", min[i]); */
  /* } */
  /* printf("\n"); */
  
  
  for (i = 0; i < size; i++) {
    for (j = min[i]; j <= max[i]; j++) {
      assert(s_bucket_nelems[j] < s_bucket_size);  // assert bucket is not overflown
      s_buckets[j][s_bucket_nelems[j]++] = i;
    }
  }

  s_effetive_nelems += size;
  
}

// Assume elements in each bucket are ordered increasingly
// result will be positions ordered increasingly
void buckets_extract(int low, int high, int *result, int *count) {
  
  static int current_size = 0;  // size for buffer used in merging
  
  int nelems = 0;
  int i;
  
  if (low > s_nbuckets-1 || high < 0) {
    *count = 0;
    return;
  }

  if (high > s_nbuckets-1) high = s_nbuckets - 1;
  if (low < 0) low = 0;

  printf("Extract buckets from %d to %d\n", low, high);
      
  // Number of elements in those buckets
  // No greater than effective elements in all buckets so far
  for (i = low; i <= high; i++) {
    nelems += s_bucket_nelems[i];
    if (nelems >= s_effetive_nelems) {
      nelems = s_effetive_nelems;
      break;
    }
  }

  // Allocate space for buckets merging if necessary
  int to_alloc = 0;
  if (current_size == 0) {
    to_alloc = 1;
    current_size = 4096;  // init size
  } else if (current_size < nelems) {
    to_alloc = 1;
    do {
      current_size *= 2;  // increas buffer size exponentially
    } while (current_size < nelems);
  }
  if (to_alloc) {
    s_a = (int *)realloc(s_a, current_size * sizeof(int));
    s_b = (int *)realloc(s_b, current_size * sizeof(int));
  }

  // Just one bucket
  if (low == high) {
    memcpy(result, s_buckets[low], s_bucket_nelems[low] * sizeof(int));
    *count = s_bucket_nelems[low];
    // Return
    return;
  }
  
  // Merge buckets
  int ia=0, ib=0;
  int *iap = &ia, *ibp = &ib;
  int *ap = s_a, *bp = s_b;

  *iap = s_bucket_nelems[low];
  ap = s_buckets[low];


  // Iteratively merge buckets up to i with bucket i+1
  for (i = low; i < high; i++) {
    // Last round
    if (i == high -1) {
      buckets_merge(ap, *iap, s_buckets[i+1], s_bucket_nelems[i+1], result, count);
    } else {
      buckets_merge(ap, *iap, s_buckets[i+1], s_bucket_nelems[i+1], bp, ibp);
      // Swap a and b
      ap = (bp == s_b) ? s_b : s_a;
      bp = (bp == s_b) ? s_a : s_b;
      iap = (ibp == &ib) ? &ib : &ia;
      ibp = (ibp == &ib) ? &ia : &ib;
    }
  }
}

// Merge bucket a and bucket b to bucket c
// Get rid of replicated elements 
void buckets_merge(int *bucket_a, int size_a, int *bucket_b, int size_b, int *bucket_c, int *size) {
  
  int i = 0, j = 0, k = 0;
  
  while (i < size_a && j < size_b) {
    // a <= b
    if (bucket_a[i] <= bucket_b[j]) {
      bucket_c[k] = bucket_a[i];
      // a == b, increment j also
      if (bucket_a[i] == bucket_b[j]) j++;
      i++;
      k++;
    } else {
      bucket_c[k] = bucket_b[j];
      j++;
      k++;
    }
  }

  // If bucket a runs out first, memcpy from bucket b
  if (i >= size_a && j < size_b) {
    memcpy(bucket_c+k, bucket_b+j, (size_b-j) * sizeof(int));
    k += size_b - j;
  } else if (j <= size_b && i < size_a) {
    memcpy(bucket_c+k, bucket_a+i, (size_a-i) * sizeof(int));
    k += size_a - i;
  }

  
  // Set size of merged buckets
  *size = k;
 
}

void buckets_print() {
  int i, j;
  printf("Here are the buckets \n");
  for (i = 0; i < s_nbuckets; i++) {
    if (s_bucket_nelems[i] == 0) continue; // print nothing if empty
    printf("[%2d][%3d] ", i, s_bucket_nelems[i]);
    for (j = 0; j < s_bucket_nelems[i]; j++) {
      printf("%2d ", s_buckets[i][j]);
    }
    printf("\n");
  }
}

