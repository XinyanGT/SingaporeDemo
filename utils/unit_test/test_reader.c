#include "reader.h"
#include "mpi.h"
#include "common.h"

int main(int argc, char **argv) {

  assert(argc > 4);
  
  char *filename = argv[1];
  char *varname = argv[2];

  int row_nprocs = atoi(argv[3]);
  int col_nprocs = atoi(argv[4]);

  MPI_Comm comm= MPI_COMM_WORLD;
  int rank;
  MPI_Init(&argc, &argv);
  
  MPI_Comm_rank(comm, &rank);
  reader_init(filename, varname, row_nprocs, col_nprocs);

  int row, col, total_steps;
  int lrow, lcol, orow, ocol;

  reader_get_dim(&row, &col, &total_steps);
  printf("[%d]G: %d X %d, total_steps: %d\n", rank, row, col, total_steps);
  reader_get_dim_local(&lrow, &lcol, &orow, &ocol);
  printf("[%d]L: %d X %d, O: %d, %d\n", rank, lrow, lcol, orow, ocol);
  
  double *data = (double *) calloc(NULL, lrow * lcol * sizeof(double));
  int step;
  int i, j;  

  for (step = 0; step < 1; step++) {
    reader_read(data);
    for (i = 0; i < lrow; i++) { 
      for (j = 0; j < lcol; j++) {
	printf("%6.2f", data[i*lcol+j]);
      }
      printf("\n");
    }
    printf("\n");
  }

  reader_finalize();
  MPI_Finalize();

}
