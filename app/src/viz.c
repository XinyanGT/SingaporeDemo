#include "viz.h"
#include "reader.h"
#include "mpi.h"

int main(int argc, char **argv) {

  // Parameters
  assert(argc > 3);
  char *filename = argv[1];          // name of the bp file to read 
  char *varname = argv[2];           // name of the variable to read
  int steps = atoi(argv[3]);         // number of steps to read
  int row_nprocs = 1, col_nprocs = 1;
  float scale_high = 0.92;
  float scale_low = 0.62;
  int thresh_high = 160;
  int thresh_low = 70;

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

  // Prepare for viz
  VIZ *vp = viz_new(lrow, lcol, scale_high, scale_low, thresh_high, thresh_low);
  float *data = (float *) malloc(lrow * lcol * sizeof(float));

  // Deal with data step by step
  int i = 0;
  float mid = (scale_high + scale_low) / 2;
  while (i < steps || steps < 0) {
    // Read data
    reader_read(data);
    for (i = 0; i < lrow * lcol; i++) {
      if ( (data[i] == 0 ) ||
	   (0 < data[i] && data[i] < 0.00001) ||
	   (data[i] < 0 && data[i] > -0.00001) ) {
	data[i] = mid;
      }
    }
    viz_viz(vp, data);
    i++;
  }
  
  // Clear
  free(data);
  reader_finalize();
  viz_free(vp);
  
  MPI_Finalize();

  return 0;
}
  
