#include "xrc_lut.h"
#include "hsv.h"
#include "lut_rgb2hsv_ov6620.h"
#include <algorithm>
#include <string.h>

#if !defined(USE_HOST) && !defined(HOST)
#include <textdisp.h>
#else 
#include <stdio.h>
#endif

using namespace std;

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

CXRCLut::CXRCLut()
	{
	memset((void *)lut, 0, sizeof(lut));
	}

// Inputs:
// 16 <= {r,g,b} <= 240
// Outputs:
// 0 <= h <= 360
// 0 <= s <= 224
// 0 <= v <= 224
// if r,g,b outside of normal range, h is returned as -1

void RgbToHsvOV6620(unsigned char r, unsigned char g, unsigned char b,
                    unsigned short &h, unsigned char &s, unsigned char &v)
{
  if (r < 16 || g < 16 || b < 16 ||r > 240 || g > 240 || b > 240) {
    h=(unsigned short)-1;
    s=v=0;
    return;
  }
  r -= 16;
  g -= 16;
  b -= 16;
  unsigned char minval= min(min(r,g),b);
  v= max(max(r,g),b);
  unsigned char delta= v-minval;
  
  // Saturation is the ratio of max-min to max. 0 if max is 0
  s = v ? 224*delta/v : 0;

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

#include "lut_rgb2hsv_ov6620.h"
inline void FastRgbToHsvOV6620_555(unsigned char r, unsigned char g, unsigned char b,
                               unsigned short &h, unsigned char &s, unsigned char &v)
{
				       
  int idx = ((int)r<<10)+((int)b<<5)+g;
  h= lut_rgb2h_ov6620_555[idx];
  h= (h==255) ? (unsigned short)-1 : h<<1;
  s= lut_rgb2s_ov6620_555[idx];
  v= lut_rgb2v_ov6620_555[idx];
}

void SlowRgbToHsvOV6620_555(unsigned char r, unsigned char g, unsigned char b,
			    unsigned short &h, unsigned char &s, unsigned char &v)
{
  r <<= 3;
  g <<= 3;
  b <<= 3;
  if (r < 16 || g < 16 || b < 16 ||r > 240 || g > 240 || b > 240) {
    h=(unsigned short)-1;
    s=v=0;
    return;
  }
  r -= 16;
  g -= 16;
  b -= 16;
  unsigned char minval= min(min(r,g),b);
  v= max(max(r,g),b);
  unsigned char delta= v-minval;
  
  // Saturation is the ratio of max-min to max. 0 if max is 0
  s = v ? 224*delta/v : 0;

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

// Inputs:
// 0 <= h <= 360
// 0 <= s <= 224
// 0 <= v <= 224
// Outputs:
// 16 <= {r,g,b} <= 240

void HsvToRgbOV6620(unsigned short h, unsigned char s, unsigned char v,
                    unsigned char &r, unsigned char &g, unsigned char &b)
{
  int i = h/60;
  int f = h-i*60; // 0 to 60
  if (!(i & 1)) f = 60 - f; // if i is even
  int m = v * (224-s) / 224;  // 0 to 224
  int n = v * (224*60 - s * f)/(224*60);  // 0 to 224
  
  switch (i) {
  case 1:  r=n+16; g=v+16; b=m+16; break;
  case 2:  r=m+16; g=v+16; b=n+16; break;
  case 3:  r=m+16; g=n+16; b=v+16; break;
  case 4:  r=n+16; g=m+16; b=v+16; break;
  case 5:  r=v+16; g=m+16; b=n+16; break;
  default: r=v+16; g=n+16; b=m+16; break;
  }
}

void CXRCLut::CompressRange(unsigned char min_g, unsigned char max_g,
                            unsigned char &lut1, unsigned char &lut2,
                            unsigned char &lut3)
{
  // Save one bit in the lut by encoding min_g's LSB in the ordering
  // of min and max.
  // lut1 and lut2 are each one bit smaller than min_g,max_g.
  // lut3 is one bit (max_g's LSB)
  // Note that min_g == max_g can only be encoded if min_g is even
  
  if (min_g & 1) {
    lut2= min_g >> 1;
    lut1= max_g >> 1;
    assert(lut1>lut2);
  } else {
    lut1= min_g >> 1;
    lut2= max_g >> 1;
  }
  lut3= max_g & 1;
}

void CXRCLut::UncompressRange(unsigned char lut1, unsigned char lut2,
                              unsigned char lut3,
                              unsigned char &gMin, unsigned char &gMax)
{
  if (lut1 > lut2) {
    gMin= (lut2<<1) | 1;
    gMax= (lut1<<1) | lut3;
  } else { // (lut1 <= lut2)
    gMin= (lut1<<1) | 0;
    gMax= (lut2<<1) | lut3;
  }
}

void CXRCLut::ComputeFromHSVCube(int hMin, int hMax,
                                 int sMin, int sMax,
                                 int vMin, int vMax)
{
  ComputeFromHSVCubePartial(hMin, hMax,
			    sMin, sMax,
			    vMin, vMax,
			    0, rSize);
}

void CXRCLut::ComputeFromHSVCubePartial(int hMin, int hMax,
					int sMin, int sMax,
					int vMin, int vMax,
					int rBegin, int rEnd)
{
  int debug= 0;
  if (debug) {
    printf("Setting partial model r: %d-%d:\n", rBegin, rEnd);
    printf("h: %d-%d s: %d-%d v: %d-%d\n", hMin, hMax, sMin, sMax,
           vMin, vMax);
  }
  
  // Create and fill min_g and max_g lookup tables
  // min_g < g <= max_g
  // This means we cannot include g=0 in the color model, which is fine
  // because the OV6620 outputs 16 <= g <= 240
  assert(rBits == 5);
  assert(gBits == 5);
  assert(bBits == 5);

  
  for (int r= rBegin; r< rEnd; r++) {
    for (int b= 0; b< bSize; b++) {
      bool prev=0;
      int thisGMin= 0;  // start with inclusive range
      int thisGMax= gSize-1;
      if (lut_rgb2hsv_ov6620_555_valid) {
	for (int g= 0; g< gSize; g++) {
	  unsigned short h; unsigned char s,v;
	  FastRgbToHsvOV6620_555(r, g, b, h, s, v);
	  
	  bool ok=
	    h != (unsigned short)-1 &&
	    in_hsv_cube(h,s,v, hMin,hMax, sMin,sMax, vMin,vMax);
	  
	  if (g && ok != prev) {
	    if (ok && thisGMin == 0) {
	      // 0 to 1 transition (rising): min
	      // If more than one rising edge, take the first
	      thisGMin= g-1; // min is exclusive
	    } else if (!ok) {
	      // 1 to 0 transition (falling): max
	      // If more than one falling edge, take the last
	      thisGMax= g-1; // max is inclusive
	    }
	  }
	  prev= ok;
	}
      } else {
	for (int g= 0; g< gSize; g++) {
	  unsigned short h; unsigned char s,v;
	  SlowRgbToHsvOV6620_555(r, g, b, h, s, v);
	  
	  bool ok=
	    h != (unsigned short)-1 &&
	    in_hsv_cube(h,s,v, hMin,hMax, sMin,sMax, vMin,vMax);
	  
	  if (g && ok != prev) {
	    if (ok && thisGMin == 0) {
	      // 0 to 1 transition (rising): min
	      // If more than one rising edge, take the first
	      thisGMin= g-1; // min is exclusive
	    } else if (!ok) {
	      // 1 to 0 transition (falling): max
	      // If more than one falling edge, take the last
	      thisGMax= g-1; // max is inclusive
	    }
	  }
	  prev= ok;
	}
      }

      if (thisGMin == 0 && thisGMax == gSize-1 && !prev) {
        // All zeros.  Empty range.
        thisGMin= thisGMax= 0;
      }

      //if (debug) {
      //printf("r=%d, b=%d, g= %d-%d\n", r, b, thisGMin, thisGMax);
      //}
      setGRange(r, b, thisGMin, thisGMax);
    }
  }
}

void CXRCLut::print()
{
  for (int r= -1; r< rSize; r++) {
    for (int l= 0; l< 2; l++) {
      for (int b= -1; b< bSize; b++) {
        if (r < 0 && b < 0) printf(!l ? " b->" : "rv  ");
        else if (r < 0) printf(!l ? " %2d " : " -- ", b);
        else if (b < 0) printf(!l ? "%2d: " : "    ", r);
        else {
          unsigned char gMin, gMax;
          getGRange(r, b, gMin, gMax);
          printf(" %2d ", !l ? gMin : gMax);
        }
      }
      printf("\n");
    }
    printf("\n");
  }
}

