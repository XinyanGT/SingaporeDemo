#include "writer.h"
#include "retriever.h"
#include "reader.h"
#include "mpi.h"

void naive_set(int row, int col, double offset_value, double *data) {
  int i,j; 
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      data[i*col+j] = offset_value + 0.1*i + 0.01*j;
    }
  }

}

int main(int argc, char **argv) {
  
  /* double data1[row*col], data2[row*col]; */
  /* double data[row*col]; */
  /* int period = 1; */
  
  /* naive_set(row, col, 0, data1); */
  /* naive_set(row, col, 1, data2); */

    // Parameters
  assert(argc > 8);
  char *filename = argv[1];          // name of the bp file to read 
  char *varname = argv[2];           // name of the variable to read
  int row_nprocs = atoi(argv[3]);    // decompose one step of data to processes 
  int col_nprocs = atoi(argv[4]);
  int row_nchunks = atoi(argv[5]);   // further decompose, a chunk is a unit to compute min and mx
  int col_nchunks = atoi(argv[6]);
  int period = atoi(argv[7]);
  int steps = atoi(argv[8]);    // number of steps to read

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
  
  writer_init("test.bp", "vol", wdp, period);
  
  double *data = (double *) malloc(lrow * lcol * sizeof(double));
  double *chunk = (double *) malloc(idp->max_chunksize * period * sizeof(double));

  int i, j;
  int result[3][idp->nchunks];
  result[0][0] = 7;
  result[0][1] = 8;
  result[1][0] = 1;
  result[1][1] = 4;
  result[2][0] = 9;
  int count[3] = {2, 2, 1};
  int adios_step = 0;

  // Deal with data step by step
  for (i = 0; i < steps; i++) {

    // Read data
    reader_read(data);

    // Store data for later retrieval
    retriever_feed(rp, data);

    // Last step in a period
    if ( (i+1) % period == 0) {
      adios_step = i / period;
      // Write
      writer_start(result[adios_step], count[adios_step]);
      for (j = 0; j < count[adios_step]; j++) {
	retriever_get_chunk(rp, result[adios_step][j], chunk);
	writer_write(result[adios_step][j], chunk);
      }
      writer_stop();
    }
  }

  
  // Clear
  free(chunk);
  free(data);
  reader_finalize();
  writer_finalize();
  decomp_finalize(idp);
  decomp_finalize(wdp);
  retriever_finalize(rp);
  MPI_Finalize();

  return 0;
}
