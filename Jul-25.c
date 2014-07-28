/*******************************************************
 * Created: Jul 25, 2014
 * Author:  Xinyan Yan
 * Brief:   An example for using utils and yandex.
            Running it will create test.bp
 *******************************************************/
 
#include "yandex.h"
#include "stimer.h"
#include "mpi.h"
#include "reader.h"
#include "writer.h"

int main(int argc, char **argv) {

  // Parameters
  assert(argc > 10);
  char *filename = argv[1];          // name of the bp file to read 
  char *varname = argv[2];           // name of the variable to read
  int row_nprocs = atoi(argv[3]);    // decompose one step of data to processes 
  int col_nprocs = atoi(argv[4]);
  int row_nchunks = atoi(argv[5]);   // further decompose, a chunk is a unit to compute min and mx
  int col_nchunks = atoi(argv[6]);
  int nbuckets = atoi(argv[7]);      // number of buckets
  int bucket_size = atoi(argv[8]);   // max buckets size
  double low = strtod(argv[9], NULL);    // low bound for query
  double high = strtod(argv[10], NULL);  // high bound for query

  int period = 1;   // have not implemented adios write temporal aggregated values yet..
  int steps = 200;    // number of steps to read

  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);

  // Decompose by processes, to read different portions of data
  decomp_t *rdp;
  rdp = reader_init(filename, varname, row_nprocs, col_nprocs);  // for reader

  int row, col, total_steps;
  int lrow, lcol, orow, ocol;
  reader_get_dim(&row, &col, &total_steps);
  printf("[%d]G: %d X %d, total_steps: %d\n", rank, row, col, total_steps);
  reader_get_dim_local(&lrow, &lcol, &orow, &ocol);
  printf("[%d]L: %d X %d, O: %d, %d\n", rank, lrow, lcol, orow, ocol);  

  // Further decompose by chunks, for computing max and min to index
  decomp_t *idp, *wdp;
  retriever_t *rp;

  idp = decomp_init(lrow, lcol, row_nchunks, col_nchunks);   // for index
  wdp = decomp_focus(rdp, rank, idp);    // for writer
  
  rp = retriever_init(idp, period);
  
  writer_init("test.bp", "vol", wdp);
  
  double *data = (double *) malloc(lrow * lcol * sizeof(double));
  double *chunk = (double *) malloc(idp->max_chunksize * period * sizeof(double));
  yandex_init(rp, nbuckets, bucket_size);

  int i, j;
  int result[idp->nchunks];
  int count;
  int okflag, nexact, nfuzzy;
  double total_build_time = 0;
  double total_query_time = 0;

  // Deal with data step by step
  for (i = 0; i < steps; i++) {

    // First step in a period
    if (i % period == 0) {
      stimer_start();
      yandex_start();
      stimer_stop();
      total_build_time += stimer_get_interval();
    }

    // Read data
    reader_read(data);

    // Store data for later retrieval
    retriever_feed(rp, data);

    // Update index building
    stimer_start();
    yandex_update();
    stimer_stop();
    total_build_time += stimer_get_interval();

    // Last step in a period
    if ( (i+1) % period == 0) {
      // Build index
      stimer_start();
      yandex_stop();
      stimer_stop();
      total_build_time += stimer_get_interval();
      
      // Query
      stimer_start();
      yandex_query(low, high, result, &count);
      stimer_stop();
      total_query_time += stimer_get_interval();
	
      // Print buckets
      //      buckets_print();
      // Verify query results
      okflag = yandex_verify(low, high, result, count, &nexact, &nfuzzy, 1);
      assert(okflag);
      // Write
      writer_start(result, count);
      for (j = 0; j < count; j++) {
	retriever_get_chunk(rp, result[j], chunk);
	writer_write(result[j], chunk);
      }
      writer_stop();
    }
  }

  // Index building time and query time
  printf("Index building time: %fs. Query time: %fs\n", total_build_time, total_query_time);
  
  // Clear
  reader_finalize();
  writer_finalize();
  decomp_finalize(idp);
  decomp_finalize(wdp);
  retriever_finalize(rp);
  yandex_finalize();
  
  MPI_Finalize();

  return 0;
}
  
	
