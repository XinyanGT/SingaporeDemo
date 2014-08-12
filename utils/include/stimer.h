#ifndef STIMER_H
#define STIMER_H

#include "common.h"

typedef struct STIMER {
  struct timespec start;
  struct timespec end;
} STIMER;


STIMER *stimer_new();
void stimer_start(STIMER *sp);
void stimer_stop(STIMER *sp);
double stimer_get_interval(STIMER *sp);
void stimer_free(STIMER *sp);

#endif


