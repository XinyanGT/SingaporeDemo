/*****************************************************************
 * Created: Jul 20, 2014
 * Author:  Xinyan Yan
 * Brief:   Write to a bp file.
 *****************************************************************/

#include "writer.h"
#include "mpi.h"
#include "adios.h"

static MPI_Comm s_comm = MPI_COMM_WORLD;
static int buffer_size = 10;
static int64_t s_adios_group, s_adios_file;
static int64_t *s_idp;
static decomp_t *s_dp;
static char s_varname[256];
static char s_filename[256];

void writer_init(char *filename, char *varname, decomp_t *dp) {
  
  adios_init_noxml(s_comm);
  adios_allocate_buffer(ADIOS_BUFFER_ALLOC_NOW, 10);
  adios_declare_group(&s_adios_group, "restart", "", adios_flag_no);
  adios_select_method(s_adios_group, "MPI", "", "");
  
  sprintf(s_varname, "%s", varname);
  sprintf(s_filename, "%s", filename);
  int nchunks = dp->row_nchunks * dp->col_nchunks;
  s_idp = (int64_t *) malloc(nchunks * sizeof(int64_t));
  s_dp = dp;

  int i;
  int lrow, lcol, orow, ocol;
  char g[256], l[256], o[256];
  sprintf(g, "%d,%d", dp->row, dp->col);
  for (i = 0; i < nchunks; i++) {
    decomp_get_pos(s_dp, i, &lrow, &lcol, &orow, &ocol);
    sprintf(l, "%d,%d", lrow, lcol);
    sprintf(o, "%d,%d", orow, ocol);
    s_idp[i] = adios_define_var(s_adios_group, s_varname, "", adios_double, l, g, o);
  }		     

}

void writer_start(int *pos, int count) {
  adios_open(&s_adios_file, "restart", s_filename, "a", s_comm);
  int data_size = 0;
  uint64_t group_size, total_size;
  int lrow, lcol, orow, ocol;
  int i;
  for (i = 0; i < count; i++) {
    decomp_get_pos(s_dp, pos[i], &lrow, &lcol, &orow, &ocol);
    data_size += lrow * lcol;
  }
  group_size = data_size * sizeof(double);
  adios_group_size(s_adios_file, group_size, &total_size);
}

void writer_write(int pos, double *data) {
  adios_write_byid(s_adios_file, s_idp[pos], data);
}

void writer_stop() {
  adios_close(s_adios_file);
  MPI_Barrier(s_comm);
}

void writer_finalize() {
  int rank;
  MPI_Comm_rank(s_comm, &rank);
  adios_finalize(rank);
}
