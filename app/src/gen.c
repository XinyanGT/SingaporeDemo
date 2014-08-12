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
  assert(argc > 12);
  char *filename = argv[1];          // name of the bp file to read 
  char *varname = argv[2];           // name of the variable to read
  int row_nprocs = atoi(argv[3]);    // decompose one step of data to processes 
  int col_nprocs = atoi(argv[4]);
  int row_nchunks = atoi(argv[5]);   // further decompose, a chunk is a unit to compute min and mx
  int col_nchunks = atoi(argv[6]);
  int period = atoi(argv[7]);
  int steps = atoi(argv[8]);         // number of steps to read
  int nbuckets = atoi(argv[9]);      // number of buckets
  int hist_ratio = atoi(argv[10]);         // number of steps to read
  float low = strtof(argv[11], NULL);    // low bound for query
  float high = strtof(argv[12], NULL);  // high bound for query

  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);

  // Performance
  double read_time = 0;
  double build_time = 0;
  double query_time = 0;
  double retrieve_time = 0;
  double write_time = 0;
  unsigned int total_nrough = 0;
  unsigned int total_nexact = 0;
  STIMER *sp = stimer_new();

  // Decompose by processes, to read different portions of data
  DECOMP *rdp;
  stimer_start(sp);
  rdp = reader_init(filename, varname, ADIOS_READ_METHOD_BP, row_nprocs, col_nprocs);  // for reader
  stimer_stop(sp);
  read_time += stimer_get_interval(sp);

  int row, col;
  int lrow, lcol, orow, ocol;
  reader_get_dim(&row, &col);
  reader_get_dim_local(&lrow, &lcol, &orow, &ocol);

  // Further decompose by chunks, for computing max and min to index
  DECOMP *idp, *wdp;
  RETRIEVER *rp;

  idp = decomp_new(lrow, lcol, row_nchunks, col_nchunks);   // for index
  wdp = decomp_focus(rdp, rank, idp);    // for writer

  stimer_start(sp);
  rp = retriever_new(idp, period);
  stimer_stop(sp);
  retrieve_time += stimer_get_interval(sp);

  stimer_start(sp);
  writer_init("test.bp", "vol", "MPI", wdp, row, col, period);
  stimer_stop(sp);
  write_time += stimer_get_interval(sp);
  
  float *data = (float *) malloc(lrow * lcol * sizeof(float));
  float *chunk = (float *) malloc(idp->max_chunksize * period * sizeof(float));
  YANDEX *yp;
  
  stimer_start(sp);
  yp = yandex_new(rp, nbuckets, hist_ratio);
  stimer_stop(sp);
  build_time += stimer_get_interval(sp);

  int i, j;
  int result[idp->nchunks];
  int count;
  int okflag, nexact, nrough;
  yandex_query_type type = YANDEX_NOT_IN;

  // Deal with data step by step
  i = 0;
  while (i < steps || steps < 0) {
    // First step in a period
    if (i % period == 0) {
      stimer_start(sp);
      yandex_start(yp);
      stimer_stop(sp);
      build_time += stimer_get_interval(sp);
    }

    // Read data
    stimer_start(sp);
    reader_read(data);
    stimer_stop(sp);
    read_time += stimer_get_interval(sp);

    // Store data for later retrieval
    stimer_start(sp);
    retriever_feed(rp, data);
    stimer_stop(sp);
    retrieve_time += stimer_get_interval(sp);

    // Update index building
    stimer_start(sp);
    yandex_update(yp);
    stimer_stop(sp);
    build_time += stimer_get_interval(sp);

    // Last step in a period
    if ( (i+1) % period == 0) {
      // Build index
      stimer_start(sp);
      yandex_stop(yp);
      stimer_stop(sp);
      build_time += stimer_get_interval(sp);
      
      // Query
      stimer_start(sp);
      yandex_query(yp, low, high, result, &count, type);
      stimer_stop(sp);
      query_time += stimer_get_interval(sp);
	
      // Print buckets
      /* buckets_print(); */
      // Verify query results
      okflag = yandex_verify(yp, low, high, result, count, &nexact, &nrough, 0, type);
      total_nrough += nrough;
      total_nexact += nexact;
      assert(okflag);
      
      // Write
      stimer_start(sp);
      writer_start(result, count);
      stimer_stop(sp);
      write_time += stimer_get_interval(sp);
      
      for (j = 0; j < count; j++) {
	stimer_start(sp);
	retriever_get_chunk(rp, result[j], chunk);
	stimer_stop(sp);
	retrieve_time += stimer_get_interval(sp);

	stimer_start(sp);
	writer_write(result[j], chunk);
	stimer_stop(sp);
	write_time += stimer_get_interval(sp);
	
      }

      stimer_start(sp);
      writer_stop();
      stimer_stop(sp);
      write_time += stimer_get_interval(sp);
    }
    i++;
  }
  
  // Clear
  free(chunk);
  free(data);

  stimer_start(sp);
  reader_finalize();
  stimer_stop(sp);
  read_time += stimer_get_interval(sp);

  stimer_start(sp);
  writer_finalize();
  stimer_stop(sp);
  write_time += stimer_get_interval(sp);
  
  decomp_free(idp);
  decomp_free(wdp);

  stimer_start(sp);
  retriever_free(rp);
  stimer_stop(sp);
  retrieve_time += stimer_get_interval(sp);

  stimer_start(sp);
  yandex_free(yp);
  stimer_stop(sp);
  build_time += stimer_get_interval(sp);
  
  MPI_Finalize();

  stimer_free(sp);
  // Display performance information
  printf("============================================================\n");
  printf("Timing Information(in sec)\n");
  printf("Read: %.3f    Index build: %.3f    Query: %.3f    Retrieve: %.3f    Write: %.3f\n", read_time, build_time, query_time, retrieve_time, write_time);
  printf("Indexing Information\n");
  printf("Rough: %.2f%%    Exact: %.2f%%    Ratio: %.2f%%\n",
	 (float)total_nrough/(rdp->size*i)*100,
         (float)total_nexact/(rdp->size*i)*100, (float)total_nrough / total_nexact*100);
  return 0;
}
  
	
