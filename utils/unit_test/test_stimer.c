#include "stimer.h"

int main() {
  STIMER *sp = stimer_new();
  stimer_start(sp);
  stimer_stop(sp);
  double interval = stimer_get_interval(sp);
  printf("Time: %f\n", interval);
  return 0;
}


