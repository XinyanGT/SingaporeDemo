#ifndef BUCKETS_H
#define BUCKETS_H

#include "common.h"

void buckets_init(int nbuckets);

void buckets_start();

void buckets_fill_range(int *max, int *min, int size);

void buckets_extract_in(int low, int high, int *result, int *count);

void buckets_extract_not_in(int low, int high, int *result, int *count);

void buckets_extract(int ranges[][2], int nranges, int *result, int *count);

void buckets_finalize();

void buckets_merge(int *bucket_a, int size_a, int *bucket_b, int size_b, int *bucket_c, int *size);

void buckets_print();

#endif


