#ifndef YANDEX_H
#define YANDEX_H

typedef enum {
  YANDEX_IN,
  YANDEX_NOT_IN
} yandex_query_type;

#include "common.h"
#include "retriever.h"
#include "decomp.h"

void yandex_init(retriever_t *rp, int nbuckets, int bucket_size);
void yandex_start();
void yandex_update();
void yandex_stop();
void yandex_query(float low_bound, float high_bound, int *query_result, int *count, yandex_query_type type);
int yandex_verify(float low_bound, float high_bound, int *query_result, int count, int *nexactp, int *nroughp, int toprint, int type);
void yandex_finalize();

#endif
