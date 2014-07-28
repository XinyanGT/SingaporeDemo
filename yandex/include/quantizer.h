#ifndef QUANTIZER_H
#define QUANTIZER_H

#include "common.h"

void quantizer_init(int level);

void quantizer_restart(double *ct, int size);

int quantizer_quantize_singleton(double ct);

void quantizer_quantize(double *ct, int *data_dc, int size);

void quantizer_finalize();
		      
#endif
