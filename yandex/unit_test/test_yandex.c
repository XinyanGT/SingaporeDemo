#include "yandex.h"

int main() {

  int row = 4, col = 6;
  int nbuckets = 30, bucket_size = 100;
  int period = 2;
  int i, j;

  double data1[4 * 6] =
    {1.1, 2.5, 3.6, 1.5, 2.9, 3.5,
     0.7, 2.1, 2.7, 3.9, 2.2, 1.4,
     3.5, 1.5, 0.2, 1.3, 2.6, 3.1,
     0.3, 1.7, 0.9, 3.2, 2.8, 2.6};
  
  double data2[4 * 6] =
    {1.0, 2.5, 3.7, 1.4, 2.6, 3.7,
     0.4, 2.3, 2.8, 3.9, 2.0, 1.5,
     3.1, 1.0, 0.3, 1.3, 2.8, 3.4,
     0.6, 1.9, 0.8, 3.1, 2.4, 0.2};

  double low = 0, high = 1.0;
  int row_nchunks = 2;
  int col_nchunks = 3;
  int result[row * col];
  int count;
  double data_query[period * col * row];
  double chunk[period*col*row];
  decomp_t *dp;
  retriever_t *rp;

  // Init
  dp = decomp_init(row, col, row_nchunks, col_nchunks);
  rp = retriever_init(dp, period);
  yandex_init(rp, nbuckets, bucket_size);

  // Start a period
  yandex_start();

  // Feed data
  retriever_feed(rp, data1);
  yandex_update();
  
  retriever_feed(rp, data2);
  yandex_update();

  // Stop a period
  yandex_stop();

  // Query
  yandex_query(low, high, result, &count);
  
  printf("=======================================================\n");
  printf("Query range: %.2f -- %.2f\n", low, high);
  printf("Result(%d)\n", count);
  for (i = 0; i < count; i++) {
    printf("%2d ", result[i]);
  }
  printf("\n");


  int chunksize;
  for (i = 0; i < count; i++) {
    
    chunksize = retriever_get_chunk(rp, result[i], chunk);
    printf("Pos %2d[%d]: ", result[i], chunksize);
    for (j = 0; j < chunksize; j++) {
      if (j%(col/col_nchunks) == 0) printf("\n");
      printf("%.2f ", chunk[j]);
    }
    printf("\n\n");
  }

  int okflag, nexact, nfuzzy;
  okflag = yandex_verify(low, high, result, count, &nexact, &nfuzzy, 1);

  
  /* yandex_get(result, count, data_query); */

  /* for (i = 0; i < count * period; i++) { */
  /*   printf("%.2f ", data_query[i]); */
  /* } */
  /* printf("\n"); */

  decomp_finalize(dp);
  retriever_finalize(rp);
  yandex_finalize();

  return 0;
}
