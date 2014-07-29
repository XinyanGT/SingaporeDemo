#ifndef WRITER_H
#define WRITER_H

#include "decomp.h"

void writer_init(char *filename, char *varname, decomp_t *dp, int period);
void writer_start(int *pos, int count);
void writer_write(int pos, double *data);
void writer_stop();
void writer_finalize();

#endif
