#ifndef HISTQUAN_H
#define HISTQUAN_H

#include "common.h"

typedef struct HISTQUAN {
  int level;  // quantization level
  int ratio;  // finess of the histgram
  int *table;
  int table_size;  // number of elements in the table = level * number of bins in the hist
  float max, min; 
  float interval;
} HISTQUAN;


HISTQUAN *histquan_new(int level, int ratio);

void histquan_restart(HISTQUAN *hp, float *ct, int size);

int histquan_quantize_singleton(HISTQUAN *hp, float ct);

void histquan_quantize(HISTQUAN *hp, float *ct, int *data_dc, int size);

void histquan_free(HISTQUAN *hp);
		      
#endif
