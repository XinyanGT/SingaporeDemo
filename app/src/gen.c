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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct PARAMS {
  char filename[256];
  char varname[256];
  int row_nprocs;
  int col_nprocs;
  int row_nchunks;
  int col_nchunks;
  int period;
  int steps;
  int nbuckets;
  int hist_ratio;
} PARAMS;

int main(int argc, char **argv) {

  // Parameters
  char *index_dir = "Index";
  
  assert(argc > 13);
  int mode = atoi(argv[1]);
  char *filename = argv[2];          // name of the bp file to read 
  char *varname = argv[3];           // name of the variable to read
  // mode:
  //   1 -- generate index and query
  //   2 -- generate index and save to file
  //   3 -- read index from file and query

  int row_nprocs = atoi(argv[4]);    // decompose one step of data to processes 
  int col_nprocs = atoi(argv[5]);
  int row_nchunks = atoi(argv[6]);   // further decompose, a chunk is a unit to compute min and mx
  int col_nchunks = atoi(argv[7]);
  int period = atoi(argv[8]);
  int steps = atoi(argv[9]);         // number of steps to read
  int nbuckets = atoi(argv[10]);      // number of buckets
  int hist_ratio = atoi(argv[11]);         // number of steps to read
  float low = strtof(argv[12], NULL);    // low bound for query
  float high = strtof(argv[13], NULL);  // high bound for query

  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);
  PARAMS params;
  char meta_filename[256];
  char index_filename[256];
  FILE *meta_file, *index_file;
  char rank_str[256];
  sprintf(rank_str, "%d", rank);
  strcpy(meta_filename, index_dir);
  strcat(meta_filename, "/meta");
  strcpy(index_filename, index_dir);
  strcat(index_filename, "/index-");
  strcat(index_filename, rank_str);  
    

  // If master process and mode is generating index and saving to local file
  // write meta information
  if (rank == 0 && mode == 2) {
    struct stat st = {0};
    if (stat(index_dir, &st) == -1) {
      mkdir(index_dir, 0775);
    }

    strncpy(params.filename, filename, 256);
    strncpy(params.varname, varname, 256);
    params.row_nprocs = row_nprocs;
    params.col_nprocs = col_nprocs;
    params.row_nchunks = row_nchunks;
    params.col_nchunks = col_nchunks;
    params.period = period;
    params.steps = steps;
    params.nbuckets = nbuckets;
    params.hist_ratio = hist_ratio;
    meta_file = fopen(meta_filename, "w");
    fwrite(&params, sizeof(PARAMS), 1, meta_file);
    fclose(meta_file);
  }


  // mode is reading previous built index and doing query
  // read parameters from meta file
  if (mode == 3) {
    meta_file = fopen(meta_filename, "r");
    fread(&params, sizeof(PARAMS), 1, meta_file);
    assert(row_nprocs == params.row_nprocs);
    assert(col_nprocs == params.col_nprocs);
    row_nchunks = params.row_nchunks;
    col_nchunks = params.col_nchunks;
    period = params.period;
    steps = params.steps;
    nbuckets = params.nbuckets;
    hist_ratio = params.hist_ratio;
    fclose(meta_file);
  }


  if (mode == 1) {
    index_file = NULL;
  } else if (mode == 2) {
    index_file = fopen(index_filename, "w");
  } else {
    index_file = fopen(index_filename, "r");
  }
		  
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
  if (mode == 3)
    yp = yandex_new(rp, nbuckets, hist_ratio, index_file);
  else
   yp = yandex_new(rp, nbuckets, hist_ratio, NULL);
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


      if (mode != 2) {
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
      } else {
	yandex_save(yp, index_file);
      }
    }
    i++;
  }
  
  // Clear
  free(chunk);
  free(data);

  if (index_file)
    fclose(index_file);

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
  
	
