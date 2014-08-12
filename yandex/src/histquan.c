#include "histquan.h"
#include <float.h>

inline int histquan_get_nelems(HISTQUAN *hp, int dv, int size) {
 int delimit = hp->level - size % hp->level; 
 int n =  size / hp->level;
 return (dv < delimit) ? n : n+1;
}

// level: the quantization level
// ratio: how fine the histogram will be
HISTQUAN *histquan_new(int level, int ratio) {
  HISTQUAN *hp = (HISTQUAN *)malloc(sizeof(HISTQUAN));
  hp->level = level;
  hp->ratio = ratio;
  hp->table_size = level * ratio;
  hp->table = (int *) malloc(hp->table_size * sizeof(int));
  return hp;
}


void histquan_restart(HISTQUAN *hp, float *data, int size) {
  int i;
  float max = FLT_MIN;
  float min = FLT_MAX;
  memset(hp->table, 0, hp->table_size*sizeof(int));  // reset table

  // Get min and max number
  for (i = 0; i < size; i++) {
    max = data[i] > max ? data[i] : max;
    min = data[i] < min ? data[i] : min;
  }
  
  max += 0.00001; // increase max a small amount so that the max value will be quantizied to the highest level
  hp->max = max; 
  hp->min = min;
  
  hp->interval = (max - min) / hp->table_size;

  // Construct quantization table based on input data distribution
  // Logically, first put all data into (ratio x level) bins,
  // in other words, build a fine histogram.
  // Then aggregate continuous bins into larger bins, so that
  // each large bin has approximately the same amount of data
  int dv = 0;  // index for next large bin
  int nelems = 0;  // number of nelems have been processed
  int desired_nelems = histquan_get_nelems(hp, 0, size);
  int is_first = 1; // indicate whether this bin is the first one in a larger bin
  int index;  // index for bin
  
  for (i = 0; i < size; i++) {
    index = (int) ((data[i] - hp->min) / hp->interval);
    if (index < 0) index = 0;
    hp->table[index]++;
  }
  
  for (i = 0; i < hp->table_size; i++) {
    // Exceed desired elements numbers
    if (nelems + hp->table[i] > desired_nelems) {
      // This bin only has too many elements
      // Have to be one discrete value no matter how big it is
      if (is_first) {
	nelems += hp->table[i];
	hp->table[i] = dv++;
	desired_nelems += histquan_get_nelems(hp, dv, size);
      // This bin together with previous ones have too many elements
      } else {
	// Better to put this bin to later large bin
	// Revisit this bin in next iteration
	if (nelems + hp->table[i] - desired_nelems > desired_nelems - nelems) {
	  i--;
        // Better to include this one in previous large bin
	} else {
 	  nelems += hp->table[i];
	  hp->table[i] = dv;
	}
	dv++;
	desired_nelems += histquan_get_nelems(hp, dv, size);
	is_first = 1;
      }
      
    // Not exceed desired elements number
    } else {
      is_first = 0;
      nelems += hp->table[i];
      hp->table[i] = dv;
    }

    // Check
    assert(dv <= hp->level);
  }
  
}

// Quantize a value to integers from 0 to level - 1
inline int histquan_quantize_singleton(HISTQUAN *hp, float ct) {
  int index;

  // Out of bound
  if (ct < hp->min) return -1;
  if (ct > hp->max) return hp->level;

  // Look up table
  index = (int) ((ct - hp->min) / hp->interval);
  return hp->table[index];
}

void histquan_quantize(HISTQUAN *hp, float *data, int *data_dc, int size) {
  int i;
  for (i = 0; i < size; i++) {
    data_dc[i] = histquan_quantize_singleton(hp, data[i]);
  }
}

void histquan_free(HISTQUAN *hp) {
  free(hp->table);
  free(hp);
}

					   
