#ifndef QUANTIZER_H
#define QUANTIZER_H

#include "common.h"

void quantizer_init(int level);

void quantizer_restart(float *ct, int size);

int quantizer_quantize_singleton(float ct);

void quantizer_quantize(float *ct, int *data_dc, int size);

void quantizer_finalize();
		      
#endif
