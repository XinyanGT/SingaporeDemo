#include "writer.h"
#include "retriever.h"

void naive_set(int row, int col, double offset_value, double *data) {
  int i,j; 
  for (i = 0; i < row; i++) {
    for (j = 0; j < col; j++) {
      data[i*col+j] = offset_value + 0.1*i + 0.01*j;
    }
  }

}

int main(int argc, char **argv) {
  
  MPI_Init(&argc, &argv);
  
  int row = 4, col = 6;
  int row_nchunks = 2, col_nchunks = 2;
  char *varname = "vol";
  char *filename = "test.bp";

  double data1[row*col], data2[row*col];
  double data[row*col];
  int period = 1;
  
  naive_set(row, col, 0, data1);
  naive_set(row, col, 1, data2);

  decomp_t *dp = decomp_init(row, col, row_nchunks, col_nchunks);
  retriever_t *rp = retriever_init(dp, period);

  int i;
  writer_init(filename, varname, dp);
  
  int pos[] = {0,3};
  int count = 2;
  
  writer_start(pos, count);
  retriever_feed(rp, data1);
  
    
  for (i = 0; i < count; i++) {
    retriever_get_chunk(rp, pos[i], data);
    writer_write(pos[i], data);
  }


  writer_stop();

  
  pos[0] = 1;
  count = 1;
  writer_start(pos, count);
  retriever_feed(rp, data2);

  for (i = 0; i < count; i++) {
    retriever_get_chunk(rp, pos[i], data);
    writer_write(pos[i], data);
  }

  writer_stop();

  decomp_finalize(dp);
  retriever_finalize(rp);
  writer_finalize();
  MPI_Finalize();
}
