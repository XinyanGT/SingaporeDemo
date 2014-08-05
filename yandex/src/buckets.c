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
static int *s_bucket_size;    // the maximal number of elements each bucket can hold
static int s_bucket_initial_size = 4096;
static int s_effetive_nelems;
static int *s_a;   // used in buckets_extract
static int *s_b;

void buckets_init(int nbuckets) {
  
  // Allocate space for buckets and bucket sizes
  s_bucket_nelems = (int *) calloc(nbuckets, sizeof(int));
  s_bucket_size = (int *) malloc(nbuckets * sizeof(int));
  s_buckets = (int **) malloc(nbuckets * sizeof(int*));

  int i;
  for (i = 0; i < nbuckets; i++) {
    s_bucket_size[i] = s_bucket_initial_size;
    s_buckets[i] = (int *) malloc(s_bucket_initial_size * sizeof(int));
  }
  
  // Store information
  s_nbuckets = nbuckets;
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
  free(s_bucket_size);
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
      // Put 1 more element i to bucket j
      // If bucket j is going to overflow, enlarge the size
      if (s_bucket_nelems[j] >= s_bucket_size[j]) {
	s_bucket_size[j] *= 2;
	s_buckets[j] = (int *)realloc(s_buckets[j], s_bucket_size[j] * sizeof(int));
      }
      s_buckets[j][s_bucket_nelems[j]++] = i;
    }
  }

  s_effetive_nelems += size;
  
}

void buckets_extract_not_in(int low, int high, int *result, int *count) {
  int ranges[2][2];
  assert(low <= high);
  // No elements lower than low or higher than high
  if (low < 0 && high > s_nbuckets-1) {
    *count = 0;
    return;
  }
  // No elements lower than low
  if (low < 0) {
    ranges[0][0] = high;
    ranges[0][1] = s_nbuckets-1;
    buckets_extract(ranges, 1, result, count);
    return;
  }
  // No elements higher than high
  if (high > s_nbuckets-1) {
    ranges[0][0] = 0;
    ranges[0][1] = low;
    buckets_extract(ranges, 1, result, count);
    return;
  }

  // Otherwise
  ranges[0][0] = 0;
  ranges[0][1] = low;
  ranges[1][0] = high;
  ranges[1][1] = s_nbuckets - 1;
  buckets_extract(ranges, 2, result, count);
  
}

void buckets_extract_in(int low, int high, int *result, int *count) {
  int ranges[1][2];
  ranges[0][0] = low;
  ranges[0][1] = high;
  buckets_extract(ranges, 1, result, count);
  
}

// Assume elements in each bucket are ordered increasingly
// result will be positions ordered increasingly
// The lower bound of ranges[i] better be strictly greater than the
// higher bound of ranges[i-1], although still works if not
// However to each range, higher bound should not be less than lower bound
void buckets_extract(int ranges[][2], int nranges, int *result, int *count) {
    
  static int current_size = 0;  // size for buffer used in merging
  int high, low;
  int nelems;
  int n;
  int i, r;
  int to_alloc;

  int ia, ib;
  int *iap, *ibp;
  int *ap, *bp;

  printf("Extract buckets from %d ranges\n", nranges);

  if (current_size == 0) {
    current_size = 4096;   // initial local buffer size
    s_a = (int *) malloc(current_size * sizeof(int));
    s_b = (int *) malloc(current_size * sizeof(int));
  }
  ia = ib = 0;
  iap = &ia;
  ibp = &ib;
  ap = s_a;
  bp = s_b;
  
  for (r = 0; r < nranges; r++) {
    low = ranges[r][0];
    high = ranges[r][1];
    assert(low <= high);
    // No elements fall in this range, nothing to merge
    if (low > s_nbuckets-1 || high < 0) {
      continue;
    }
    if (high > s_nbuckets-1) high = s_nbuckets - 1;
    if (low < 0) low = 0;
    printf("[%d]Extract buckets from %d to %d\n", r, low, high);
    
    // Number of elements in those buckets
    // No greater than effective elements in all buckets so far
    n = 0;
    for (i = low; i <= high; i++) {
      n += s_bucket_nelems[i];
      if (n >= s_effetive_nelems) {
	n = s_effetive_nelems;
	break;
      }
    }

    // Allocate space for buckets merging if necessary
    to_alloc = 0;
    if (current_size < n) {
      to_alloc = 1;
      do {
	current_size *= 2;  // increase buffer size exponentially
      } while (current_size < n);
    }
    if (to_alloc) {
      s_a = (int *)realloc(s_a, current_size * sizeof(int));
      s_b = (int *)realloc(s_b, current_size * sizeof(int));
    }

    // Iteratively merge bucket i with previous merged buckets
    for (i = low; i <= high; i++) {
      buckets_merge(ap, *iap, s_buckets[i], s_bucket_nelems[i], bp, ibp);
      // Swap a and b pointers, so that a pointers correspond to previous merged buckets
      ap = (bp == s_b) ? s_b : s_a;
      bp = (bp == s_b) ? s_a : s_b;
      iap = (ibp == &ib) ? &ib : &ia;
      ibp = (ibp == &ib) ? &ia : &ib;
    }
  }
  *count = *iap;
  memcpy(result, ap, *iap * sizeof(int));
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

