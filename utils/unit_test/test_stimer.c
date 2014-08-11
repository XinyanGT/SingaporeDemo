#include "stimer.h"

int main() {
  stimer_start();
  stimer_stop();
  double interval = stimer_get_interval();
  printf("Time: %f\n", interval);

  return 0;
  
}


