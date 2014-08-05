#include "hquantizer.h"
#include <float.h>

static int s_level;
static int *s_table;
static int s_table_size;
static float s_max, s_min;
static float s_interval;
static int s_ratio;

inline int hquantizer_get_nelems(int dv, int size) {
 int delimit = s_level - size % s_level; 
 int n =  size / s_level;
 return (dv < delimit) ? n : n+1;
}

// level: the quantization level
// ratio: how fine the histogram will be
void hquantizer_init(int level, int ratio) {
  s_level = level;
  s_ratio = ratio;
  s_table_size = level * ratio;
  s_table = (int *) malloc(s_table_size * sizeof(int));
}


void hquantizer_restart(float *data, int size) {
  int i, j;
  float max = FLT_MIN;
  float min = FLT_MAX;
  memset(s_table, 0, s_table_size*sizeof(int));  // reset table

  // Get min and max number
  for (i = 0; i < size; i++) {
    max = data[i] > max ? data[i] : max;
    min = data[i] < min ? data[i] : min;
  }
  
  max += 0.00001; // increase max a small amount so that the max value will be quantizied to the highest level
  s_max = max; 
  s_min = min;
  
  s_interval = (max - min) / s_table_size;

  // Construct quantization table based on input data distribution
  // Logically, first put all data into (ratio x level) buckets,
  // in other words, build a fine histogram.
  // Then aggregate continuous buckets into larger buckets, so that
  // each large bucket has approximately the same amount of data
  int dv = 0;  // index for next large bucket
  int nelems = 0;  // number of nelems have been processed
  int desired_nelems = hquantizer_get_nelems(0, size);
  int is_first = 1; // indicate whether this bucket is the first one in a larger bucket
  int index;  // index for bucket
  
  for (i = 0; i < size; i++) {
    index = (int) ((data[i] - s_min) / s_interval);
    s_table[index]++;
  }
  
  for (i = 0; i < s_table_size; i++) {
    // Exceed desired elements numbers
    if (nelems + s_table[i] > desired_nelems) {
      // This bucket only has too many elements
      // Have to be one discreate value no matter how big it is
      if (is_first) {
	nelems += s_table[i];
	s_table[i] = dv++;
	desired_nelems += hquantizer_get_nelems(dv, size);
      // This bucket together with previous ones have too many elements
      } else {
	// Better to put this bucket to later large bucket
	// Revisit this bucket in next iteration
	if (nelems + s_table[i] - desired_nelems > desired_nelems - nelems) {
	  i--;
        // Better to include this one in previous large bucket
	} else {
 	  nelems += s_table[i];
	  s_table[i] = dv;
	}
	dv++;
	desired_nelems += hquantizer_get_nelems(dv, size);
	is_first = 1;
      }
      
    // Not exceed desired elements number
    } else {
      is_first = 0;
      nelems += s_table[i];
      s_table[i] = dv;
    }

    // Check
    assert(dv <= s_level);
  }
  
}

// Quantize a value to integers from 0 to s_level - 1
inline int hquantizer_quantize_singleton(float ct) {
  int index;

  // Out of bound
  if (ct < s_min) return -1;
  if (ct > s_max) return s_level;

  // Look up table
  index = (int) ((ct - s_min) / s_interval);
  return s_table[index];
}

void hquantizer_quantize(float *data, int *data_dc, int size) {
  int i;
  for (i = 0; i < size; i++) {
    data_dc[i] = hquantizer_quantize_singleton(data[i]);
  }
}

void hquantizer_finalize() {
  free(s_table);
}

					   
