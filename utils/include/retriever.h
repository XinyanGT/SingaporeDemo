#ifndef RETRIEVER_H
#define RETRIEVER_H

#include "common.h"
#include "decomp.h"

typedef struct retriever_t {
  int period;
  int current_step;
  double *data;
  decomp_t *dp;
} retriever_t;
  
retriever_t *retriever_init(decomp_t *dp, int period);
void retriever_feed(retriever_t *rp, double *I);
double *retriever_get_step(retriever_t *rp, int step);
double *retriever_get_laststep(retriever_t *rp);
int retriever_get_chunk(retriever_t *rp, int pos, double *data);
void retriever_finalize(retriever_t *rp);
//void retriever_get(retriever_t *rp, int *pos, int pos_count, double *data);

#endif
