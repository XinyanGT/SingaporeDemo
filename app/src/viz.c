#include "viz.h"
#include "reader.h"
#include "mpi.h"

int main(int argc, char **argv) {

  // Parameters
  assert(argc > 3);
  char *filename = argv[1];          // name of the bp file to read 
  char *varname = argv[2];           // name of the variable to read
  int period = 1;
  int steps = atoi(argv[3]);         // number of steps to read
  int row_nprocs = 1, col_nprocs = 1;

  MPI_Comm comm = MPI_COMM_WORLD;
  int rank;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(comm, &rank);

  // Decompose by processes, to read different portions of data
  decomp_t *rdp;
  rdp = reader_init(filename, varname, row_nprocs, col_nprocs);  // for reader

  int row, col, total_steps;
  int lrow, lcol, orow, ocol;
  reader_get_dim(&row, &col);
  printf("[%d]G: %d X %d\n", rank, row, col);
  reader_get_dim_local(&lrow, &lcol, &orow, &ocol);
  printf("[%d]L: %d X %d, O: %d, %d\n", rank, lrow, lcol, orow, ocol);  

  // Prepare for viz
  viz_init(lrow, lcol);
  float *data = (float *) malloc(lrow * lcol * sizeof(float));

  // Deal with data step by step
  int i = 0;
  while (i < steps || steps < 0) {
    // Read data
    reader_read(data);
    viz_viz(data);
    i++;
  }
  
  // Clearp
  free(data);
  reader_finalize();
  
  MPI_Finalize();

  return 0;
}
  
