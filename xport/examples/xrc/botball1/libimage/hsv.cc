#include "hsv.h"
#include <algorithm>
using namespace std;

// adapted from http://www.acm.org/jgt/papers/SmithLyons96/hsv_rgb.html
// h from 0 to 360
// s from 0 to 255
// v from 0 to 255
void hsv_to_rgb(unsigned short h, unsigned char s,  unsigned char v,
                unsigned char &r, unsigned char &g, unsigned char &b)
{
  int i = h/60;
  int f = h-i*60; // 0 to 60
  if (!(i & 1)) f = 60 - f; // if i is even
  int m = v * (255-s) / 255;  // 0 to 255
  int n = v * (255*60 - s * f)/(255*60);  // 0 to 255
  
  switch (i) {
  case 1:  r=n; g=v; b=m; break;
  case 2:  r=m; g=v; b=n; break;
  case 3:  r=m; g=n; b=v; break;
  case 4:  r=n; g=m; b=v; break;
  case 5:  r=v; g=m; b=n; break;
  default: r=v; g=n; b=m; break;
  }
}

// Hue from 0 to 360
// 0:   red
// 60:  yellow
// 120: green
// 180: cyan
// 240: blue
// 300: magenta

// adapted from http://www.mediamacros.com/item/item-1006687249/

void rgb_to_hsv(unsigned char r, unsigned char g, unsigned char b,
                unsigned short &h, unsigned char &s, unsigned char &v)
{
  unsigned char minval= min(min(r,g),b);
  v= max(max(r,g),b);
  unsigned char delta= v-minval;
  
  // Saturation is the ratio of max-min to max. 0 if max is 0
  s = v ? 255*delta/v : 0;

  if (!s) {
    h= 0; // If no saturation, arbitrarily select hue=0
  } else if (r == v) {
    // between -60 (magenta) and +60 (yellow)
    int htmp=60*(g-b)/delta;
    if (htmp < 0) htmp += 360;
    h= htmp;
  } else if (g == v) {
    // between +60 (yellow) and +180 (cyan)
    h=120+60*(b-r)/delta;
  } else {
    // between +180 (cyan) and +300 (magenta)
    h=240+60*(r-g)/delta;
  }
}
