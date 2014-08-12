#include "decomp.h"

int main() {
  
  DECOMP *dpA, *dpB, *dpC;
  int lrow, lcol, orow, ocol;
  int row, col, row_nchunks, col_nchunks;
  int i;
  
  row = 14;
  col = 24 ;
  row_nchunks = 3;
  col_nchunks = 5;
  dpA = decomp_new(row, col, row_nchunks, col_nchunks);
  printf("dpA g: %d X %d, chunks: %d X %d\n", row, col, row_nchunks, col_nchunks);

  for (i = 0; i < row_nchunks * col_nchunks; i++) {
    decomp_get_pos(dpA, i, &lrow, &lcol, &orow, &ocol);
    printf("dpA [%d] l: %d X %d, o: %d %d\n", i, lrow, lcol, orow, ocol);
  }
  

  row = 4;
  col = 4 ;
  row_nchunks = 3;
  col_nchunks = 2;
  dpB = decomp_new(row, col, row_nchunks, col_nchunks);
  printf("dpB g: %d X %d, chunks: %d X %d\n", row, col, row_nchunks, col_nchunks);
  for (i = 0; i < row_nchunks * col_nchunks; i++) {
    decomp_get_pos(dpB, i, &lrow, &lcol, &orow, &ocol);
    printf("dpB [%d] l: %d X %d, o: %d %d\n", i, lrow, lcol, orow, ocol);
  }

  int pos = 8;
  dpC = decomp_focus(dpA, pos, dpB);
  printf("dpC g: %d X %d, chunks: %d X %d\n", dpC->row, dpC->col, dpC->row_nchunks, dpC->col_nchunks);
  for (i = 0; i < dpC->row_nchunks * dpC->col_nchunks; i++) {
    decomp_get_pos(dpC, i, &lrow, &lcol, &orow, &ocol);
    printf("dpC [%d] l: %d X %d, o: %d %d\n", i, lrow, lcol, orow, ocol);
  }

  decomp_free(dpA);
  decomp_free(dpB);
  decomp_free(dpC);
  return 0;
}
