/************************************************************
 * Created: Jul 15, 2014
 * Author:  Xinyan Yan
 * Brief:   Parallel reading. Simulate reading data step by step.
            Underneath, it reads a 3D var from a bp file. a x b x c.
            a is treated as time step label.
 ************************************************************/

#include "reader.h"
#include "common.h"
#include "mpi.h"
#include "adios_read.h"

static ADIOS_FILE *s_file;
static ADIOS_VARINFO *s_var;
static char s_varname[256];
static char s_filename[256];
static enum ADIOS_READ_METHOD s_method = ADIOS_READ_METHOD_BP;
static MPI_Comm s_comm = MPI_COMM_WORLD;
static uint64_t s_current_step;
static int s_rank, s_nprocs;
static decomp_t *s_dp;
static uint64_t s_row, s_col, s_lrow, s_lcol, s_orow, s_ocol;


// Return decomp_t which indicates how data is decomposed
// reader will free it when finalizing
decomp_t *reader_init(char *filename, char *varname, int row_nprocs, int col_nprocs) {
  MPI_Status status; 

  // ADIOS Init
  adios_read_init_method(s_method, s_comm, "verbose=3");
  s_file = adios_read_open(filename, s_method, s_comm, ADIOS_LOCKMODE_NONE, 0);

  if (s_file == NULL) {
    printf("%s\n", adios_errmsg());
    return NULL;
  }

  s_var = adios_inq_var(s_file, varname);
  sprintf(s_varname, "%s", varname);
  sprintf(s_filename, "%s", filename);

  // Dimension decomposition
  int rank;
  MPI_Comm_rank(s_comm, &rank);
  int row = s_var->dims[1];
  int col = s_var->dims[2];
  decomp_t *dp;
  dp = decomp_init(row, col, row_nprocs, col_nprocs);
  int lr, lc, or, oc;
  decomp_get_pos(dp, rank, &lr, &lc, &or, &oc);

  // Store to static variables
  s_dp = dp;
  s_row = row;
  s_col = col;
  s_lrow = lr;
  s_lcol = lc;
  s_orow = or;
  s_ocol = oc;
  
  return dp;
}

// Get global dimension
void reader_get_dim(int *row, int *col, int *total_steps) {
  
  *total_steps = s_var->dims[0];
  *row = s_row; 
  *col = s_col;

}

// Get local dimension
void reader_get_dim_local(int *row, int *col, int *orow, int *ocol) {
  *row = s_lrow;
  *col = s_lcol;
  *orow = s_orow;
  *ocol = s_ocol;
}


// Read a new step 
void reader_read(double *data) {
  
  ADIOS_SELECTION *sel;
  int ndim = 3;
 
  uint64_t start[ndim], count[ndim];

  start[0] = s_current_step++;
  start[1] = s_orow;
  start[2] = s_ocol;
  count[0] = 1;
  count[1] = s_lrow;
  count[2] = s_lcol;

  sel = adios_selection_boundingbox(ndim, start, count);
  adios_schedule_read(s_file, sel, s_varname, 0, 1, data);
  adios_perform_reads(s_file, 1);
  
}


void reader_finalize() {
  free(s_dp);
  adios_read_close(s_file);
  MPI_Barrier(s_comm);
  adios_read_finalize_method(s_method);
    
}


