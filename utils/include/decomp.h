#ifndef DECOMP_H
#define DECOMP_H

#include "common.h"

// Decompose a chunk
typedef struct DECOMP {
  int orowg, ocolg;      // offsets of the chunks in its superchunk
  int row, col, size;    // dimension of the chunk
  int row_nchunks, col_nchunks, nchunks;  // how the chunk is decomposed into subchunks
  int max_chunksize;    // max size of subchunks
} DECOMP;

DECOMP *decomp_new(int row, int col, int row_nchunks, int col_nchunks);

// Get offsets and positions of a subchunk in dp, related to its superchunk
extern inline void decomp_get_pos(DECOMP *dp, int pos, int *lrow, int *lcol, int *orow, int *ocol);

DECOMP *decomp_focus(DECOMP *dpA, int pos, DECOMP *dpB);

void decomp_free(DECOMP *dp);

#endif


