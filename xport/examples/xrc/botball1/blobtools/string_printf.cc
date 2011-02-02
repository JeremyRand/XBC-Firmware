#include <stdio.h>
#include "string_printf.h"

using namespace std;

// The following are from Randy Sargent
string
string_vprintf(const char *fmt, va_list args)
{
  string ret;
  int size= 100;

  while (1) {
    ret.resize(size);
#if defined(_WIN32)
    int nwritten= _vsnprintf(&ret[0], size-1, fmt, args);
#else
    int nwritten= vsnprintf(&ret[0], size-1, fmt, args);
#endif
    // Some c libraries return -1 for overflow, some return
    // a number larger than size-1
    if (nwritten >= 0 && nwritten < size-2) {
      if (ret[nwritten-1] == 0) nwritten--;
      ret.resize(nwritten);
      return ret;
    }
    size *= 2;
  }
}

string
string_printf(const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  string ret= string_vprintf(fmt, args);
  va_end(args);
  return ret;
}
