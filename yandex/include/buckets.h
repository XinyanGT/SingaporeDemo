#ifndef BUCKETS_H
#define BUCKETS_H

#include "common.h"

typedef struct BUCKETS {
  int nbuckets;
  int bucket_initial_size;
  int **buckets;
  int *bucket_nelems;  // current number of elements in each bucket
  int *bucket_size;    // the maximal number of elements each bucket can hold
  int effetive_nelems;
  int *u;   // used in bucketextract
  int *v;
} BUCKETS;


BUCKETS *buckets_new(int nbuckets);

void buckets_start(BUCKETS *bp);

void buckets_fill_range(BUCKETS *bp, int *max, int *min, int size);

void buckets_extract_in(BUCKETS *bp, int low, int high, int *result, int *count);

void buckets_extract_not_in(BUCKETS *bp, int low, int high, int *result, int *count);

void buckets_extract(BUCKETS *bp, int ranges[][2], int nranges, int *result, int *count);

void buckets_finalize(BUCKETS *bp);

void buckets_print(BUCKETS *bp);

#endif


