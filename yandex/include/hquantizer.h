#ifndef HQUANTIZER_H
#define HQUANTIZER_H

#include "common.h"

void hquantizer_init(int level, int ratio);

void hquantizer_restart(float *ct, int size);

int hquantizer_quantize_singleton(float ct);

void hquantizer_quantize(float *ct, int *data_dc, int size);

void hquantizer_finalize();
		      
#endif
