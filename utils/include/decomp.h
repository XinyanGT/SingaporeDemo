#ifndef DECOMP_H
#define DECOMP_H

#include "common.h"

// Decompose a chunk
typedef struct decomp_t {
  int orowg, ocolg;      // offsets of the chunks in its superchunk
  int row, col, size;    // dimension of the chunk
  int row_nchunks, col_nchunks, nchunks;  // how the chunk is decomposed into subchunks
  int max_chunksize;    // max size of subchunks
} decomp_t;

decomp_t *decomp_init(int row, int col, int row_nchunks, int col_nchunks);

// Get offsets and positions of a subchunk in dp, related to its superchunk
extern inline void decomp_get_pos(decomp_t *dp, int pos, int *lrow, int *lcol, int *orow, int *ocol);

decomp_t *decomp_focus(decomp_t *dpA, int pos, decomp_t *dpB);

void decomp_finalize(decomp_t *dp);

#endif


