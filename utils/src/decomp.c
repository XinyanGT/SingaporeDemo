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

/*
decomp_t *decomp_copy(decomp_t *dp) {
  decompt_t *dp_new = (decomp_t *) malloc(sizeof(decomp_t));
  memcpy(dp_new, dp, sizeof(decomp_t));
  return dp_new;
}
*/
		     
// Get offsets and positions of a subchunk in dp, related to its superchunk
// For example, row = 2400, row_nchunks = 500
// Then first part (0-99) should have 4 elements, and second part (100-499) should have 5
// elements, instead of first part(0-498) having 4 elements and second part(499) having 404 elements
inline void decomp_get_pos(decomp_t *dp, int pos, int *lrow, int *lcol, int *orow, int *ocol) {
  
  // lr: number of rows in this chunk
  // or: row offset
  // ir: index in row for this chunk (0 - row_nchunks-1)
  // dr: delimit index in row, from this index, second part chunks start
  
  int lr, lc, or, oc;
  int dr, dc, ir, ic;
  
  ir = pos / dp->col_nchunks;  
  lr = dp->row / dp->row_nchunks;
  dr = dp->row_nchunks - dp->row % dp->row_nchunks;
  if (ir < dr) {  // first part
    or = lr * ir;
  } else {
    or = lr * dr + (ir-dr) * (lr+1);
    lr++;
  }

  ic = pos % dp->col_nchunks;
  lc = dp->col / dp->col_nchunks;
  dc = dp->col_nchunks - dp->col % dp->col_nchunks;
  if (ic < dc) {
    oc = lc * ic;
  } else {
    oc = lc * dc + (ic-dc) * (lc+1);
    lc++;
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


