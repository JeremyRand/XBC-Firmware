#ifndef __TRIGTAB__
#define __TRIGTAB__
//-------------------------------------------------------------------------
// Integer sin and cos tables:  
//   Index for a given angle is according to the following formula:
//     int index = ((int)((angle/(2.0*M_PI))*512.0))%512;
//
//   Values are multipled by 256, so entries are equal to:
//     sintab[index] =  (int)(sin(angle) * 256.0);
//     costab[index] =  (int)(cos(angle) * 256.0);
#include "math.h"

#ifndef M_PI
#define M_PI		3.14159265358979323846
#endif

static inline int ttab_index(float angle) {
  return(((int)((angle/(2.0*M_PI))*512.0))%512);
}
extern const short sintab[512];
extern const short costab[512];

#endif // __TRIGTAB__
