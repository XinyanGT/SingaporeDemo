/************************************************************
 * Created: Jul 20, 2014
 * Author:  Xinyan Yan
 * Brief:   Store data which spans a certain time period.
 *          Automatically overwrite pervious period data.
 ************************************************************/

#include "retriever.h"
#include "decomp.h"

RETRIEVER *retriever_new(DECOMP *dp, int period) {
  RETRIEVER *rp = (RETRIEVER *) malloc(1 * sizeof(RETRIEVER));
  rp->period = period;
  rp->current_step = 0;
  rp->data = (float *) malloc(period * dp->size * sizeof(float));
  rp->dp = dp;
  return rp;
}

void retriever_feed(RETRIEVER *rp, float *I) {
  // Store data so that they can be queried
  // Data can be compressed or reorganized here
  memcpy(rp->data + rp->dp->size*rp->current_step, I, rp->dp->size*sizeof(float));
  // Update step
  rp->current_step = (rp->current_step + 1) % rp->period;
}

// Return pointer to that step data
float *retriever_get_step(RETRIEVER *rp, int step) {
  return rp->data + rp->dp->size*step;
}

float *retriever_get_laststep(RETRIEVER *rp) {
  int step;
  step = rp->current_step == 0 ? rp->period-1 : rp->current_step-1;
  return retriever_get_step(rp, step);
}

// Return size
int retriever_get_chunk(RETRIEVER *rp, int pos, float *data) {
  int i, j, k;
  int lrow, lcol, orow, ocol;

  decomp_get_pos(rp->dp, pos, &lrow, &lcol, &orow, &ocol);
  
  for (i = 0; i < rp->period; i++) {
    for (j = 0; j < lrow; j++) {
      for (k = 0; k < lcol; k++) {
	data[i*lrow*lcol+j*lcol+k] = rp->data[i*(rp->dp->size)+(orow+j)*(rp->dp->col)+(ocol+k)];
      }
    }
  }
  return rp->period * lrow * lcol;
}

/* void retriever_get(RETRIEVER *rp, int *pos, int pos_count, float *data) { */
/*   int i; */
/*   for (i = 0; i < pos_count; i++) { */
/*     retriever_get_chunk(pos[i], data+i*rp->period); */
/*   } */
/* } */

void retriever_free(RETRIEVER *rp) {
  free(rp->data);
  free(rp);
}

