#ifndef RETRIEVER_H
#define RETRIEVER_H

#include "common.h"
#include "decomp.h"

typedef struct RETRIEVER {
  int period;
  int current_step;
  float *data;
  DECOMP *dp;
} RETRIEVER;
  
RETRIEVER *retriever_new(DECOMP *dp, int period);
void retriever_feed(RETRIEVER *rp, float *I);
float *retriever_get_step(RETRIEVER *rp, int step);
float *retriever_get_laststep(RETRIEVER *rp);
int retriever_get_chunk(RETRIEVER *rp, int pos, float *data);
void retriever_free(RETRIEVER *rp);
//void retriever_get(RETRIEVER *rp, int *pos, int pos_count, float *data);

#endif
