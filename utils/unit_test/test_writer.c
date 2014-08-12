#include "writer.h"
#include "retriever.h"
#include "reader.h"
#include "mpi.h"

void naive_set(int row, int col, float offset_value, float *data) {
  int i,j; 
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      data[i*col+j] = offset_value + 0.1*i + 0.01*j;
    }
  }

}

int main(int argc, char **argv) {
  
  /* float data1[row*col], data2[row*col]; */
  /* float data[row*col]; */
  /* int period = 1; */
  
  /* naive_set(row, col, 0, data1); */
  /* naive_set(row, col, 1, data2); */

    // Parameters
  assert(argc > 8);
  char *filename = argv[1];          // name of the bp file to read 
  char *varname = argv[2];           // name of the variable to read
  int row_nprocs = atoi(argv[3]);    // decompose one step of data to processes 
  int col_nprocs = atoi(argv[4]);
  // Note: row_nchunks x col_nchunks has to be greater or equal to 10
  // in this test case
  int row_nchunks = atoi(argv[5]);   // further decompose, a chunk is a unit to compute min and mx
  int col_nchunks = atoi(argv[6]);
  int period = atoi(argv[7]);
  int steps = atoi(argv[8]);    // number of steps to read

  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);

  // Decompose by processes, to read different portions of data
  DECOMP *rdp;
  rdp = reader_init(filename, varname, ADIOS_READ_METHOD_BP, row_nprocs, col_nprocs);  // for reader

  int row, col;
  int lrow, lcol, orow, ocol;
  reader_get_dim(&row, &col);
  printf("[%d]G: %d X %d\n", rank, row, col);
  reader_get_dim_local(&lrow, &lcol, &orow, &ocol);
  printf("[%d]L: %d X %d, O: %d, %d\n", rank, lrow, lcol, orow, ocol);  

  // Further decompose by chunks, for computing max and min to index
  DECOMP *idp, *wdp;
  RETRIEVER *rp;

  idp = decomp_new(lrow, lcol, row_nchunks, col_nchunks);   // for index
  wdp = decomp_focus(rdp, rank, idp);    // for writer
  
  rp = retriever_new(idp, period);
  
  writer_init("test.bp", "vol", "MPI", wdp, row, col, period);
  
  float *data = (float *) malloc(lrow * lcol * sizeof(float));
  float *chunk = (float *) malloc(idp->max_chunksize * period * sizeof(float));

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
  decomp_free(idp);
  decomp_free(wdp);
  retriever_free(rp);
  MPI_Finalize();

  return 0;
}
