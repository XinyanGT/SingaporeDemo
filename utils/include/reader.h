#ifndef READER_H
#define READER_H

#include "decomp.h"
#include "common.h"
#include "adios_read.h"

DECOMP *reader_init(char *filename, char *varname, enum ADIOS_READ_METHOD method, int row_nprocs, int col_nprocs);
void reader_get_dim(int *row, int *col);
void reader_get_dim_local(int *row, int *col, int *orow, int *ocol);
void reader_read(float *data);
void reader_finalize();

#endif
