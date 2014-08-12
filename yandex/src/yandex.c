/*****************************************************************
 * Created: Jul 15, 2014
 * Author:  Xinyan Yan
 * Brief:   Use min and max as descriptions for data chunks aggregated
            in spatial and temporal dimension. On top of that, other
            indexing methods can be applied, such as Bitmap and simple
            integer buckets.
 ****************************************************************/

#include "yandex.h"
#include "buckets.h"
#include "histquan.h"

static int MAX = 0;    // index of max
static int MIN = 1;

// Compute min max for a step of data
static void yandex_shrunk(YANDEX *yp, const float *I) {
  int nchunks = yp->dp->nchunks;
  int i, j, k;
  int lrow, lcol, orow, ocol;
  float max, min, elem;
  for (i = 0; i < nchunks; i++) {
    decomp_get_pos(yp->dp, i, &lrow, &lcol, &orow, &ocol);
    max = I[orow *(yp->dp->col) + ocol];
    min = max;
    for (j = orow; j < orow+lrow; j++) {
      for (k = ocol; k < ocol+lcol; k++) {
	elem = I[j*(yp->dp->col) + k];
	max = elem > max ? elem : max;
	min = elem < min ? elem : min;
      }
    }
    yp->chunk_max[i] = max;
    yp->chunk_min[i] = min;
  }
}

// Update min max record
static void yandex_update_minmax(YANDEX *yp) {
  int i;
  int nchunks = yp->dp->nchunks;
  if (yp->first_step) {
    memcpy(yp->chunk_range[MAX], yp->chunk_max, nchunks * sizeof(float));
    memcpy(yp->chunk_range[MIN], yp->chunk_min, nchunks * sizeof(float));
    yp->first_step = 0;
  } else {
    for (i = 0; i < yp->dp->nchunks; i++) {
      yp->chunk_range[MAX][i] = yp->chunk_max[i] > yp->chunk_range[MAX][i] ? yp->chunk_max[i] : yp->chunk_range[MAX][i];
      yp->chunk_range[MIN][i] = yp->chunk_min[i] < yp->chunk_range[MIN][i] ? yp->chunk_min[i] : yp->chunk_range[MIN][i];
    }
  }
}


YANDEX *yandex_new(RETRIEVER *rp, int nbuckets, int hist_ratio) {

  YANDEX *yp = (YANDEX *) malloc(sizeof(YANDEX));
  yp->rp = rp;
  yp->dp = rp->dp;
  yp->verify_data = NULL;
  
  // Allocate space
  int nchunks = yp->dp->nchunks;
  yp->chunk_min = (float *) malloc(nchunks * sizeof(float));
  yp->chunk_max = (float *) malloc(nchunks * sizeof(float));
  float *dtemp  = (float *) malloc(2 * nchunks * sizeof(float));
  yp->chunk_range[MIN] = dtemp + nchunks*MIN;
  yp->chunk_range[MAX] = dtemp + nchunks*MAX;

  int *utemp = (int *) malloc(2 * nchunks * sizeof(int));
  yp->chunk_range_dc[MAX] = utemp + nchunks*MAX;
  yp->chunk_range_dc[MIN] = utemp + nchunks*MIN;

  // Init buckets
  yp->bp = buckets_new(nbuckets);

  // Init histquan
  yp->hp = histquan_new(nbuckets, hist_ratio);

  return yp;
}


// Start a new period indexing
void yandex_start(YANDEX *yp) {
  yp->first_step = 1;
  // Restart buckets
  buckets_start(yp->bp);
}


// End of a period, build index
void yandex_stop(YANDEX *yp) {
    int nchunks = yp->dp->nchunks;
    histquan_restart(yp->hp, yp->chunk_range[0], nchunks*2);
    histquan_quantize(yp->hp, yp->chunk_range[0], yp->chunk_range_dc[0], nchunks*2);
    buckets_fill_range(yp->bp, yp->chunk_range_dc[MAX], yp->chunk_range_dc[MIN], nchunks);
}

void yandex_update(YANDEX *yp) {

  // Get min max for this step
  float *I = retriever_get_laststep(yp->rp);
  yandex_shrunk(yp, I);
  
  // Update min max record
  yandex_update_minmax(yp);
}


void yandex_query(YANDEX *yp, float low_bound, float high_bound, int *query_result, int *count, yandex_query_type type) {

  int low_dc;
  int high_dc;

  assert(low_bound <= high_bound);
  
  low_dc = histquan_quantize_singleton(yp->hp, low_bound);
  high_dc = histquan_quantize_singleton(yp->hp, high_bound);

  //  printf("low: %d, high: %d\n", low_dc, high_dc);
  switch (type) {
  case YANDEX_IN:    
    buckets_extract_in(yp->bp, low_dc, high_dc, query_result, count);
    break;
  case YANDEX_NOT_IN:
    buckets_extract_not_in(yp->bp, low_dc, high_dc, query_result, count);
    break;
  }
}
    

/*
 * Verify whether yandex query result is valid
 */
int yandex_verify(YANDEX *yp, float low_bound, float high_bound, int *query_result, int count, int *nexactp, int *nroughp, int toprint, int type) {
  
  // hit: the chunk contains value in the range
  // has: the chunk number appears in query result
  // nexact: number of data fall in that range
  // nrough: number of data may be in that range
  // chunksize: time X row X col

  float *data;
  int i, j;
  int chunksize, nexact, nrough;
  int hitflag, okflag, hasflag;
  int nchunks = yp->dp->nchunks;
  nexact = nrough = 0;
  okflag = 1;
  if (yp->verify_data == NULL) {
    yp->verify_data = (float *) malloc(yp->dp->max_chunksize * yp->rp->period * sizeof(float));
  }
  data = yp->verify_data;

  // Go over all data chunk by chunk
  for (i = 0; i < nchunks; i++) {
    
    hitflag = hasflag = 0; 
    chunksize = retriever_get_chunk(yp->rp, i, data);
    
    // Go over every data point
    for (j = 0; j < chunksize; j++) {
      switch (type) {
      case YANDEX_IN:
	if (data[j] >= low_bound && data[j] <= high_bound) {
	  nexact++;
	  hitflag = 1;
	}
	break;
      case YANDEX_NOT_IN:
	if (data[j] <= low_bound || data[j] >= high_bound) {
 	  nexact++;
	  hitflag = 1;
	}
      }
    }
    
    // Check yandex query result
    for (j = 0; j < count; j++) {
      if (query_result[j] == i) {
	hasflag = 1;
	nrough += chunksize;
	break;
      }
    }

    // data in the range but yandex missed it!!
    if (hitflag && !hasflag) {
      okflag = 0;
    }
  }

  *nexactp = nexact;
  *nroughp = nrough;

  if (toprint) {
    if (okflag) {
      printf("[SUCCESS]Query result passed verification\n");
    } else {
      printf("ERROR. Query result failed verification\n");
    }
    printf("Rough: %d[%.2f%%]. Exact: %d[%.2f%%]. Ratio: %.2f%%\n",
   	   nrough, (float)nrough/(yp->rp->period*(yp->dp->size))*100,
	   nexact, (float)nexact/(yp->rp->period*(yp->dp->size))*100,
	   (float)nrough / nexact*100);
  }

  return okflag;
}


void yandex_free(YANDEX *yp) {
  free(yp->verify_data);
  free(yp->chunk_range[0]);
  free(yp->chunk_range_dc[0]);
  buckets_free(yp->bp);
  histquan_free(yp->hp);
  free(yp);
}

 
