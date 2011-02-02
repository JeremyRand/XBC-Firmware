#ifndef _sleep_us_h_
#define _sleep_us_h_

#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif
  // define a type for using with ustime
  typedef long long utime_t;

  // wait for usecs, handling events
  void sleep_us (unsigned int usec);

  // Subtract a from b, and write difference in microseconds to result
  int usec_diff(struct timeval *a, struct timeval *b, long *result);

  // Return long long microsecond unix time.  This is from Randy
  // Sargent, 3/4/00 
  long long ustime(void);

  // Sleep  until ustime is endtime.  User beware.
  void sleep_until_us (long long endtime);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif // _sleep_us_h_
