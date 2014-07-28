#include "stimer.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>


static struct timespec start = {0, 0}, end = {0, 0};

void stimer_start() {
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);  
}

void stimer_stop() {
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
}

double stimer_get_interval() {
  return  ((double)end.tv_sec + 1.0e-9*end.tv_nsec) -
       ((double)start.tv_sec + 1.0e-9*start.tv_nsec);
}
