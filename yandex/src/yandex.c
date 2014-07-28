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
#include "quantizer.h"

#define MAX         0    // index of max
#define MIN         1

static double *s_chunk_min, *s_chunk_max;
static double *s_chunk_range[2];
static int *s_chunk_range_dc[2];
static retriever_t *s_rp;
static decomp_t *s_dp;
static int s_first_step;


// Compute min max for a step of data
static void yandex_shrunk(const double *I) {
  int nchunks = s_dp->nchunks;
  int i, j, k;
  int lrow, lcol, orow, ocol;
  double max, min, elem;
  for (i = 0; i < nchunks; i++) {
    decomp_get_pos(s_dp, i, &lrow, &lcol, &orow, &ocol);
    max = I[orow *(s_dp->col) + ocol];
    min = max;
    for (j = orow; j < orow+lrow; j++) {
      for (k = ocol; k < ocol+lcol; k++) {
	elem = I[j*(s_dp->col) + k];
	max = elem > max ? elem : max;
	min = elem < min ? elem : min;
      }
    }
    s_chunk_max[i] = max;
    s_chunk_min[i] = min;
  }
}

// Update min max record
static void yandex_update_minmax(I) {
  int i;
  int nchunks = s_dp->nchunks;
  if (s_first_step) {
    memcpy(s_chunk_range[MAX], s_chunk_max, nchunks * sizeof(double));
    memcpy(s_chunk_range[MIN], s_chunk_min, nchunks * sizeof(double));
    s_first_step = 0;
  } else {
    for (i = 0; i < s_dp->nchunks; i++) {
      s_chunk_range[MAX][i] = s_chunk_max[i] > s_chunk_range[MAX][i] ? s_chunk_max[i] : s_chunk_range[MAX][i];
      s_chunk_range[MIN][i] = s_chunk_min[i] < s_chunk_range[MIN][i] ? s_chunk_min[i] : s_chunk_range[MIN][i];
    }
  }
}


void yandex_init(retriever_t *rp, int nbuckets, int bucket_size) {

  s_rp = rp;
  s_dp = rp->dp;
  
  // Allocate space
  int nchunks = s_dp->nchunks;
  s_chunk_min = (double *) malloc(nchunks * sizeof(double));
  s_chunk_max = (double *) malloc(nchunks * sizeof(double));
  double *dtemp  = (double *) malloc(2 * nchunks * sizeof(double));
  s_chunk_range[MIN] = dtemp + nchunks*MIN;
  s_chunk_range[MAX] = dtemp + nchunks*MAX;

  int *utemp = (int *) malloc(2 * nchunks * sizeof(int));
  s_chunk_range_dc[MAX] = utemp + nchunks*MAX;
  s_chunk_range_dc[MIN] = utemp + nchunks*MIN;

    // Init buckets
  buckets_init(nbuckets, bucket_size);

  // Init quantizer
  quantizer_init(nbuckets);

}


// Start a new period indexing
void yandex_start() {
  s_first_step = 1;
  // Restart buckets
  buckets_start();
}


// End of a period, build index
void yandex_stop() {
    int nchunks = s_dp->nchunks;
    quantizer_restart(s_chunk_range[0], nchunks*2);
    quantizer_quantize(s_chunk_range[0], s_chunk_range_dc[0], nchunks*2);
    buckets_fill_range(s_chunk_range_dc[MAX], s_chunk_range_dc[MIN], nchunks);
}

void yandex_update() {

  // Get min max for this step
  double *I = retriever_get_laststep(s_rp);
  yandex_shrunk(I);
  
  // Update min max record
  yandex_update_minmax();
}


void yandex_query(double low_bound, double high_bound, int *query_result, int *count) {
  int low_dc;
  int high_dc;

  assert(low_bound <= high_bound);
  
  low_dc = quantizer_quantize_singleton(low_bound);
  high_dc = quantizer_quantize_singleton(high_bound);

  //  printf("low: %d, high: %d\n", low_dc, high_dc);
  buckets_extract(low_dc, high_dc, query_result, count);		  
}


/*
 * Verify whether yandex query result is valid
 */
int yandex_verify(double low_bound, double high_bound, int *query_result, int count, int *nexactp, int *nroughp, int toprint) {
  
  // hit: the chunk contains value in the range
  // has: the chunk number appears in query result
  // nexact: number of data fall in that range
  // nrough: number of data may be in that range
  // chunksize: time X row X col

  double *data = (double *) malloc(s_dp->max_chunksize * s_rp->period * sizeof(double));
  int i, j;
  int chunksize, nexact, nrough;
  int hitflag, okflag, hasflag;
  int nchunks = s_dp->nchunks;
  nexact = nrough = 0;
  okflag = 1;
  

  // Go over all data chunk by chunk
  for (i = 0; i < nchunks; i++) {
    
    hitflag = hasflag = 0; 
    chunksize = retriever_get_chunk(s_rp, i, data);
    
    // Go over every data point
    for (j = 0; j < chunksize; j++) {
      if (data[j] >= low_bound && data[j] <= high_bound) {
	nexact++;
	hitflag = 1;
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
   	   nrough, (double)nrough/(s_rp->period*(s_dp->size))*100,
	   nexact, (double)nexact/(s_rp->period*(s_dp->size))*100,
	   (double)nrough / nexact*100);
  }

  return okflag;
}


void yandex_finalize() {
  free(s_chunk_range[0]);
  free(s_chunk_range_dc[0]);
  buckets_finalize();
  quantizer_finalize();  
}

 
