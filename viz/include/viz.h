#ifndef VIZ_H
#define VIZ_H

#ifdef __cplusplus
extern "C" {
#endif
  
void viz_init(int row, int col, float scale_high, float scale_low, int thresh_high, int thresh_low);
void viz_viz(float *data);

#ifdef __cplusplus
}
#endif

#endif
