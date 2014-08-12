#ifndef VIZ_H
#define VIZ_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VIZ {
  void *cpp_p;
} VIZ;
    
VIZ *viz_new(int row, int col, float scale_high, float scale_low, int thresh_high, int thresh_low);
void viz_viz(VIZ *viz_p, float *data);
void viz_free(VIZ *viz_p);
  
#ifdef __cplusplus
}
#endif

#endif
