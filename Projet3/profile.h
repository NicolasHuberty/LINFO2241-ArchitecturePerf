#ifndef C_PROFILE_H
#define C_PROFILE_H

#include <time.h>
double diff(struct timespec* t1, struct timespec* t2) {
  return ((t1->tv_sec - t2->tv_sec) * 1e9 + (t1->tv_nsec - t2->tv_nsec))/1e9;
}

struct timespec current_time() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC_RAW, &t);
  return t;  
}

#endif