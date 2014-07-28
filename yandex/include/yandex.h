#ifndef YANDEX_H
#define YANDEX_H

#include "common.h"
#include "retriever.h"
#include "decomp.h"

void yandex_init(retriever_t *rp, int nbuckets, int bucket_size);
void yandex_start();
void yandex_update();
void yandex_stop();
void yandex_query(double low_bound, double high_bound, int *query_result, int *count);
int yandex_verify(double low_bound, double high_bound, int *query_result, int count, int *nexactp, int *nroughp, int toprint);
void yandex_finalize();

#endif
