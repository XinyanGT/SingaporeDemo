#ifndef UPSAMPLER_H
#define UPSAMPLER_H

#include "common.h"

void upsampler_upsample(size_t dimy, size_t dimx, double X[][dimx], 
			size_t scale, size_t dimy2, size_t dimx2, double Y[][dimx2]);


#endif


