/************************************************************
 * Created: Jul 14, 2014
 * Author:  Xinyan Yan
 * Brief:   Buckets that contain integers. In indexing case,
            the integers are the positions of records that
            fall in certain range.
 ************************************************************/

#include "buckets.h"

BUCKETS *buckets_new(int nbuckets) {

  BUCKETS *bp = (BUCKETS *) malloc(sizeof(BUCKETS));
  bp->nbuckets = nbuckets;
  bp->bucket_initial_size = 4096;
  
  // Allocate space for buckets and bucket sizes
  bp->bucket_nelems = (int *) calloc(nbuckets, sizeof(int));
  bp->bucket_size = (int *) malloc(nbuckets * sizeof(int));
  bp->buckets = (int **) malloc(nbuckets * sizeof(int*));

  int i;
  for (i = 0; i < nbuckets; i++) {
    bp->bucket_size[i] = bp->bucket_initial_size;
    bp->buckets[i] = (int *) malloc(bp->bucket_initial_size * sizeof(int));
  }
  return bp;
}

void buckets_start(BUCKETS *bp) {
  int i;
  for (i = 0; i < bp->nbuckets; i++) {
    bp->bucket_nelems[i] = 0;
  }
  bp->effetive_nelems = 0;
}

void buckets_finalize(BUCKETS *bp) {
  int i;
  for (i = 0; i < bp->nbuckets; i++) {
    free(bp->buckets[i]);
  }
  free(bp->buckets);
  free(bp->bucket_nelems);
  free(bp->bucket_size);
  free(bp->u);
  free(bp->v);
  free(bp);
}

// Valid values in max and min are integers from 0 to nbuckets-1
void buckets_fill_range(BUCKETS *bp, int *max, int *min, int size) {

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
      if (bp->bucket_nelems[j] >= bp->bucket_size[j]) {
	bp->bucket_size[j] *= 2;
	bp->buckets[j] = (int *)realloc(bp->buckets[j], bp->bucket_size[j] * sizeof(int));
      }
      bp->buckets[j][bp->bucket_nelems[j]++] = i;
    }
  }

  bp->effetive_nelems += size;
  
}

void buckets_extract_not_in(BUCKETS *bp, int low, int high, int *result, int *count) {
  int ranges[2][2];
  assert(low <= high);
  // No elements lower than low or higher than high
  if (low < 0 && high > bp->nbuckets-1) {
    *count = 0;
    return;
  }
  // No elements lower than low
  if (low < 0) {
    ranges[0][0] = high;
    ranges[0][1] = bp->nbuckets-1;
    buckets_extract(bp, ranges, 1, result, count);
    return;
  }
  // No elements higher than high
  if (high > bp->nbuckets-1) {
    ranges[0][0] = 0;
    ranges[0][1] = low;
    buckets_extract(bp, ranges, 1, result, count);
    return;
  }

  // Otherwise
  ranges[0][0] = 0;
  ranges[0][1] = low;
  ranges[1][0] = high;
  ranges[1][1] = bp->nbuckets - 1;
  buckets_extract(bp, ranges, 2, result, count);
  
}

// Merge bucket a and bucket b to bucket c
// Get rid of replicated elements 
static void buckets_merge(int *bucket_a, int size_a, int *bucket_b, int size_b, int *bucket_c, int *size) {
  
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


void buckets_extract_in(BUCKETS *bp, int low, int high, int *result, int *count) {
  int ranges[1][2];
  ranges[0][0] = low;
  ranges[0][1] = high;
  buckets_extract(bp, ranges, 1, result, count);
  
}

// Assume elements in each bucket are ordered increasingly
// result will be positions ordered increasingly
// The lower bound of ranges[i] better be strictly greater than the
// higher bound of ranges[i-1], although still works if not
// However to each range, higher bound should not be less than lower bound
void buckets_extract(BUCKETS *bp, int ranges[][2], int nranges, int *result, int *count) {
    
  static int current_size = 0;  // size for buffer used in merging
  int high, low;
  int n;
  int i, r;
  int to_alloc;

  int iu, iv;      // size
  int *iup, *ivp;  // pointer to size
  int *up, *vp;    // pointer to bucket

  /* printf("Extract buckets from %d ranges\n", nranges); */

  if (current_size == 0) {
    current_size = 4096;   // initial local buffer size
    bp->u = (int *) malloc(current_size * sizeof(int));
    bp->v = (int *) malloc(current_size * sizeof(int));
  }
  iu = iv = 0;
  iup = &iu;
  ivp = &iv;
  up = bp->u;
  vp = bp->v;
  
  for (r = 0; r < nranges; r++) {
    low = ranges[r][0];
    high = ranges[r][1];
    assert(low <= high);
    // No elements fall in this range, nothing to merge
    if (low > bp->nbuckets-1 || high < 0) {
      continue;
    }
    if (high > bp->nbuckets-1) high = bp->nbuckets - 1;
    if (low < 0) low = 0;
    /* printf("[%d]Extract buckets from %d to %d\n", r, low, high); */
    
    // Number of elements in those buckets
    // No greater than effective elements in all buckets so far
    n = 0;
    for (i = low; i <= high; i++) {
      n += bp->bucket_nelems[i];
      if (n >= bp->effetive_nelems) {
	n = bp->effetive_nelems;
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
      bp->u = (int *)realloc(bp->u, current_size * sizeof(int));
      bp->v = (int *)realloc(bp->v, current_size * sizeof(int));
    }

    // Iteratively merge bucket i with previous merged buckets
    for (i = low; i <= high; i++) {
      buckets_merge(up, *iup, bp->buckets[i], bp->bucket_nelems[i], vp, ivp);
      // Swap a and b pointers, so that a pointers correspond to previous merged buckets
      up = (vp == bp->v) ? bp->v : bp->u;
      vp = (vp == bp->v) ? bp->u : bp->v;
      iup = (ivp == &iv) ? &iv : &iu;
      ivp = (ivp == &iv) ? &iu : &iv;
    }
  }
  *count = *iup;
  memcpy(result, up, *iup * sizeof(int));
}

  		     
void buckets_print(BUCKETS *bp) {
  int i, j;
  printf("Here are the buckets \n");
  printf("Legend: [bucket index][#elements] elements\n");
  for (i = 0; i < bp->nbuckets; i++) {
    if (bp->bucket_nelems[i] == 0) continue; // print nothing if empty
    printf("[%2d][%3d] ", i, bp->bucket_nelems[i]);
    for (j = 0; j < bp->bucket_nelems[i]; j++) {
      printf("%2d ", bp->buckets[i][j]);
    }
    printf("\n");
  }
}

