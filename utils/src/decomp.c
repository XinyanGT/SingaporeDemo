#include "decomp.h"

decomp_t *decomp_init(int row, int col, int row_nchunks, int col_nchunks) {
  decomp_t *dp = (decomp_t *) malloc(1 * sizeof(decomp_t));
  dp->orowg = 0;
  dp->ocolg = 0;
  dp->row = row;
  dp->col = col;
  dp->size = row * col;
  dp->row_nchunks = row_nchunks;
  dp->col_nchunks = col_nchunks;
  dp->nchunks = row_nchunks * col_nchunks;

  // Get max chunk size
  int pos = row_nchunks*col_nchunks - 1;
  int lr, lc, or, oc;
  decomp_get_pos(dp, pos, &lr, &lc, &or, &oc);
  dp->max_chunksize = lr * lc;
  
  return dp;
}

// Get offsets and positions of a subchunk in dp, related to its superchunk
inline void decomp_get_pos(decomp_t *dp, int pos, int *lrow, int *lcol, int *orow, int *ocol) {

  int lr, lc, or, oc;
  lr = dp->row / dp->row_nchunks;
  or = lr * (pos / dp->col_nchunks);
  if (pos / dp->col_nchunks == dp->row_nchunks - 1) {
    lr += dp->row % dp->row_nchunks;
  }

  lc = dp->col / dp->col_nchunks;
  oc = lc * (pos % dp->col_nchunks);
  if ( (pos+1) % dp->col_nchunks == 0) {
    lc += dp->col % dp->col_nchunks;
  }
  
  *lrow = lr;
  *lcol = lc;
  *orow = or + dp->orowg;
  *ocol = oc + dp->ocolg;
}

// Focus on a certain chunk, superchunk of dpB is in the position pos in dpA
// Generate a new decomp_t dp
decomp_t *decomp_focus(decomp_t *dpA, int pos, decomp_t *dpB) {

  // Dimension of dp and way to decompose is the same as dpB
  decomp_t *dp = decomp_init(dpB->row, dpB->col, dpB->row_nchunks, dpB->col_nchunks);

  // Compute new offsets
  int lr, lc, or, oc;
  decomp_get_pos(dpA, pos, &lr, &lc, &or, &oc);

  // New offset =
  //    offset of dpA in its superchunk +
  //    offset of dpB's superchunk in dpB +
  //    offset of dpB in its superchunk
  dp->orowg = dpA->orowg + or + dpB->orowg;
  dp->ocolg = dpA->ocolg + oc + dpB->ocolg;
  
  return dp;
}


void decomp_finalize(decomp_t *dp) {
  free(dp);
}


