/************************************************************
 * Created: Jul 15, 2014
 * Author:  Xinyan Yan
 * Brief:   Parallel reading. Simulate reading data step by step.
            Underneath, it reads a 3D var from a bp file. a x b x c.
            a is treated as time step label.
 ************************************************************/

#include "reader.h"
#include "adios_error.h"
#include "common.h"
#include "mpi.h"
#include "adios_read.h"

static ADIOS_FILE *s_file;
static ADIOS_VARINFO *s_var;
static char s_varname[256];
static char s_filename[256];
static enum ADIOS_READ_METHOD s_method = ADIOS_READ_METHOD_BP;
static MPI_Comm s_comm = MPI_COMM_WORLD;
static uint64_t s_current_step;          // current logical step
static uint64_t s_steps_per_adios_step;  // how many logical steps contained in an adios step
static float s_timeout_sec = 1.0;
static int s_is_adios_3d;                // whether in adios it's a 3d var
static int s_rank, s_nprocs;
static decomp_t *s_dp;
static uint64_t s_row, s_col, s_lrow, s_lcol, s_orow, s_ocol;


// Return decomp_t which indicates how data is decomposed
// reader will free it when finalizing
decomp_t *reader_init(char *filename, char *varname, enum ADIOS_READ_METHOD method, int row_nprocs, int col_nprocs) {
  MPI_Status status; 

  // ADIOS Init
  s_method = method;
  adios_read_init_method(s_method, s_comm, "verbose=3");
  s_file = adios_read_open(filename, s_method, s_comm, ADIOS_LOCKMODE_NONE, s_timeout_sec);

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

  int row, col;
  if (s_var->ndim == 3) {
    s_is_adios_3d = 1;
    row = s_var->dims[1];
    col = s_var->dims[2];
    s_steps_per_adios_step = s_var->dims[0];
  } else {
    s_is_adios_3d = 0;
    row = s_var->dims[0];
    col = s_var->dims[1];
    s_steps_per_adios_step = 1;
  }
  
  
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
// Number logical steps
void reader_get_dim(int *row, int *col) {
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
void reader_read(float *data) {
  
  ADIOS_SELECTION *sel;
  uint64_t start[3], count[3];
  
  int adios_step = s_current_step / s_steps_per_adios_step;

  // Set data to zero
  memset(data, 0, s_lrow*s_lcol*sizeof(float));
  if (adios_step != 0) {
    adios_advance_step(s_file, 0, s_timeout_sec);
    if (adios_errno == err_step_notready) {
      printf("No new step arrived within the timeout(%.2fsec). Quit. %s\n", s_timeout_sec, adios_errmsg());
      exit(0);
    }
    if (adios_errno == err_end_of_stream) {
            printf("End of stream. Not new step to read. Quit. %s\n", adios_errmsg());
      exit(0);
    }
  }
    
  start[0] = s_current_step % s_steps_per_adios_step;
  start[1] = s_orow;
  start[2] = s_ocol;
  
  count[0] = 1;
  count[1] = s_lrow;
  count[2] = s_lcol;

  if (s_is_adios_3d) {
    sel = adios_selection_boundingbox(3, start, count);
  } else {
    sel = adios_selection_boundingbox(2, start+1, count+1);
  }
  adios_schedule_read(s_file, sel, s_varname, 0, 1, data);
  adios_perform_reads(s_file, 1);
  
  s_current_step++; // increment current logical step
  
}


void reader_finalize() {
  free(s_dp);
  adios_free_varinfo(s_var);
  adios_read_close(s_file);
  MPI_Barrier(s_comm);
  adios_read_finalize_method(s_method);
    
}


