#include "reader.h"
#include "retriever.h"
#include "mpi.h"

int main(int argc, char **argv) {

  assert(argc > 5);
  char *filename = argv[1];          // name of the bp file to read 
  char *varname = argv[2];           // name of the variable to read
  int row_nprocs = 1;
  int col_nprocs = 1;
  int row_nchunks = atoi(argv[3]);   
  int col_nchunks = atoi(argv[4]);
  int period = atoi(argv[5]);

  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);

  // Decompose by processes, to read different portions of data
  reader_init(filename, varname, ADIOS_READ_METHOD_BP, row_nprocs, col_nprocs);  // for reader

  int row, col;
  int lrow, lcol, orow, ocol;
  reader_get_dim(&row, &col);
  printf("[%d]G: %d X %d\n", rank, row, col);
  reader_get_dim_local(&lrow, &lcol, &orow, &ocol);
  printf("[%d]L: %d X %d, O: %d, %d\n", rank, lrow, lcol, orow, ocol);  

  // Further decompose by chunks, for computing max and min to index
  DECOMP *idp;
  RETRIEVER *rp;

  idp = decomp_new(lrow, lcol, row_nchunks, col_nchunks);   // for index
  rp = retriever_new(idp, period);
  
  float *data = (float *) malloc(lrow * lcol * sizeof(float));
  float *chunk = (float *) malloc(idp->max_chunksize * period * sizeof(float));

  int i, j, size;

  // Deal with data step by step
  for (i = 0; i < period; i++) {
    // Read data
    reader_read(data);
    // Store data for later retrieval
    retriever_feed(rp, data);
  }
  
  for (i = 0; i < idp->nchunks; i++) {
    size = retriever_get_chunk(rp, i, chunk);
    printf("=== %d ===\n", i);
    for (j = 0; j < size; j++) {
      printf("%.2f ", chunk[j]);
    }
    printf("\n");
  }

  // Clear
  free(chunk);
  free(data);
  reader_finalize();
  decomp_free(idp);
  retriever_free(rp);
  MPI_Finalize();

  return 0;

}
