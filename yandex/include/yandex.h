#ifndef YANDEX_H
#define YANDEX_H

#include "common.h"
#include "retriever.h"
#include "decomp.h"
#include "histquan.h"
#include "buckets.h"

typedef enum {
  YANDEX_IN,
  YANDEX_NOT_IN
} yandex_query_type;


typedef struct YANDEX {
  HISTQUAN *hp;
  BUCKETS *bp;
  float *chunk_min, *chunk_max;
  float *chunk_range[2];
  int *chunk_range_dc[2];
  RETRIEVER *rp;
  DECOMP *dp;
  int first_step;
  float *verify_data;
} YANDEX;


YANDEX *yandex_new(RETRIEVER *rp, int nbuckets, int hist_ratio);
void yandex_start(YANDEX *yp);
void yandex_update(YANDEX *yp);
void yandex_stop(YANDEX *yp);
void yandex_query(YANDEX *yp, float low_bound, float high_bound, int *query_result, int *count, yandex_query_type type);
int yandex_verify(YANDEX *yp, float low_bound, float high_bound, int *query_result, int count, int *nexactp, int *nroughp, int toprint, int type);
void yandex_free(YANDEX *yp);


#endif
