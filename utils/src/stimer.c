#include "stimer.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


STIMER *stimer_new() {
  
  STIMER *sp = (STIMER *) malloc(sizeof(STIMER));
  sp->start.tv_sec = 0;
  sp->start.tv_nsec = 0;
  sp->end.tv_sec = 0;
  sp->end.tv_nsec = 0;

  return sp;
}

void stimer_start(STIMER *sp) {
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sp->start);  
}

void stimer_stop(STIMER *sp) {
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &sp->end);
}

double stimer_get_interval(STIMER *sp) {
  return  ((double)sp->end.tv_sec + 1.0e-9*sp->end.tv_nsec) -
       ((double)sp->start.tv_sec + 1.0e-9*sp->start.tv_nsec);
}

void stimer_free(STIMER *sp) {
  free(sp);
}
