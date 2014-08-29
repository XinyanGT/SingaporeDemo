#include "yandex.h"

int main() {

  int row = 4, col = 6;
  int nbuckets = 40;
  int hist_ratio = 5;
  int period = 2;
  int i, j;

  float data1[4 * 6] =
    {1.1, 2.5, 3.6, 1.5, 2.9, 3.5,
     0.7, 2.1, 2.7, 3.9, 2.2, 1.4,
     3.5, 1.5, 0.2, 1.3, 2.6, 3.1,
     0.3, 1.7, 0.9, 3.2, 2.8, 2.6};
  
  float data2[4 * 6] =
    {1.0, 2.5, 3.7, 1.4, 2.6, 3.7,
     0.4, 2.3, 2.8, 3.9, 2.0, 1.5,
     3.1, 1.0, 0.3, 1.3, 2.8, 3.4,
     0.6, 1.9, 0.8, 3.1, 2.4, 0.2};

  float low = 0, high = 1.0;
  int row_nchunks = 2;
  int col_nchunks = 6;
  int result[row * col];
  int count;
  float chunk[period*col*row];
  DECOMP *dp;
  RETRIEVER *rp;
  YANDEX *yp;

  // Init
  dp = decomp_new(row, col, row_nchunks, col_nchunks);
  rp = retriever_new(dp, period);
  yp = yandex_new(rp, nbuckets, hist_ratio, NULL);
  
  // Start a period
  yandex_start(yp);

  // Feed data
  retriever_feed(rp, data1);
  yandex_update(yp);
  
  retriever_feed(rp, data2);
  yandex_update(yp);

  // Stop a period
  yandex_stop(yp);

  // Query
  int chunksize;
  int nexact, nfuzzy;
  int okflag;
  
  yandex_query(yp, low, high, result, &count, YANDEX_IN);
  buckets_print(yp->bp);
  
  printf("=======================================================\n");
  printf("Query range: %.2f -- %.2f\n", low, high);
  printf("Result(%d)\n", count);
  for (i = 0; i < count; i++) {
    printf("%2d ", result[i]);
  }
  printf("\n");



  for (i = 0; i < count; i++) {
    
    chunksize = retriever_get_chunk(rp, result[i], chunk);
    printf("Pos %2d[%d]: ", result[i], chunksize);
    for (j = 0; j < chunksize; j++) {
      if (j%(col/col_nchunks) == 0) printf("\n");
      printf("%.2f ", chunk[j]);
    }
    printf("\n\n");
  }


  okflag = yandex_verify(yp, low, high, result, count, &nexact, &nfuzzy, 1, YANDEX_IN);


  low = -1;
  high = 3;
  yandex_query(yp, low, high, result, &count, YANDEX_NOT_IN);
  printf("=======================================================\n");
  printf("Query not in range: %.2f -- %.2f\n", low, high);
  printf("Result(%d)\n", count);
  for (i = 0; i < count; i++) {
    printf("%2d ", result[i]);
  }
  printf("\n");



  
  for (i = 0; i < count; i++) {
    
    chunksize = retriever_get_chunk(rp, result[i], chunk);
    printf("Pos %2d[%d]: ", result[i], chunksize);
    for (j = 0; j < chunksize; j++) {
      if (j%(col/col_nchunks) == 0) printf("\n");
      printf("%.2f ", chunk[j]);
    }
    printf("\n\n");
  }


  okflag = yandex_verify(yp, low, high, result, count, &nexact, &nfuzzy, 1, YANDEX_NOT_IN);
  
  
  decomp_free(dp);
  retriever_free(rp);
  yandex_free(yp);

  return 0;
}
