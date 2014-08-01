/************************************************************
 * Created: July 16, 2014
 * Author:  Xinyan Yan
 * Brief:   Uniform quantization. 
 ***********************************************************/

#include "quantizer.h"
#include <float.h>

static int s_level;    // quantization level
static float s_interval;
static float s_max, s_min;


void quantizer_init(int level) {
  s_level = level;
}

void quantizer_restart(float *data, int size) {
  int i;
  float max = FLT_MIN;
  float min = FLT_MAX;

  // Get min and max number
  for (i = 0; i < size; i++) {
    max = data[i] > max ? data[i] : max;
    min = data[i] < min ? data[i] : min;
  }
  
  max += 0.00001; // increase max a small amount so that the max value will be quantizied to the highest level
  s_max = max; 
  s_min = min;
  s_interval = (max - min) / s_level;


printf("max: %.2f, min: %.2f, level: %d, intervel: %.2f\n", max, min, s_level, s_interval);

}

// Quantize a value to integers from 0 to s_level - 1
inline int quantizer_quantize_singleton(float ct) {
  if (ct < s_min) return -1;
  if (ct > s_max) return s_level;
  return (int) ((ct - s_min) / s_interval);
}

void quantizer_quantize(float *data, int *data_dc, int size) {
  int i;
  for (i = 0; i < size; i++) {
    data_dc[i] = quantizer_quantize_singleton(data[i]);
  }
}

void quantizer_finalize() {
}
