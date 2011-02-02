#ifndef XRC_LUT_H
#define XRC_LUT_H

#include <assert.h>

void RgbToHsvOV6620(unsigned char r, unsigned char g, unsigned char b,
                    unsigned short &h, unsigned char &s, unsigned char &v);

void HsvToRgbOV6620(unsigned short h, unsigned char s, unsigned char v,
                    unsigned char &r, unsigned char &g, unsigned char &b);

void SlowRgbToHsvOV6620_555(unsigned char r, unsigned char g,
			    unsigned char b,
			    unsigned short &h, unsigned char &s,
			    unsigned char &v);

// Usage:
// Create a model, then compute the LUT from HSV cube
//
// CXRCLut model;
// model.ComputeFromHSVCube(min_h, max_h, min_s, max_s, min_v, max_v);
// See below for ranges on h, s, and v

class CXRCLut {
public:
  CXRCLut();

  const static int rBits= 5, gBits= 5, bBits= 5;
  const static int rSize= 1<<rBits, gSize= 1<<gBits, bSize= 1<<bBits;
  unsigned short lut[rSize*bSize];
  static void
  CXRCLut::CompressRange(unsigned char min_g, unsigned char max_g,
                         unsigned char &lut1, unsigned char &lut2,
                         unsigned char &lut3);

  static void
  CXRCLut::UncompressRange(unsigned char lut1, unsigned char lut2,
                           unsigned char lut3,
                           unsigned char &min_g, unsigned char &max_g);

  // HSV cube:
  //
  // h from 0 to 360
  // s from 0 to 224
  // v from 0 to 224
  //
  // Not guaranteed to always work when hue range (min_h to max_h, including
  // wrapping) is more than 120 degrees
  //
  // h can wrap;  e.g. min_h=330 max_h=30 is a valid 60 degree range
  
  void ComputeFromHSVCube(int min_h, int max_h,
                          int min_s, int max_s,
                          int min_v, int max_v);

  // Fills in part of LUT from rBegin to rEnd-1, values in range from
  // 0 to rSize-1.  
  void ComputeFromHSVCubePartial(int hMin, int hMax,
				 int sMin, int sMax,
				 int vMin, int vMax,
				 int rBegin, int rEnd);

  void print();

  void setGRange(unsigned char r, unsigned char b,
                 unsigned char gMin, unsigned char gMax) {
    assert (gMin < gMax || gMin == 0);
    unsigned char lut1, lut2, lut3;
    CompressRange(gMin, gMax, lut1, lut2, lut3);
    
    int lut_idx= (r<<bBits) | b;
    lut[lut_idx]= (lut1 << gBits) | (lut2 << 1) | lut3;
    
    bool test= true;
    if (test) {
      unsigned char testGMin, testGMax;
      getGRange(r, b, testGMin, testGMax);
      assert(testGMin == gMin && testGMax == gMax);
    }
    
  }
    
  void getGRange(unsigned char r, unsigned char b,
                 unsigned char &gMin, unsigned char &gMax) {
    int idx= (r<<CXRCLut::gBits) | b;
    int val= lut[idx];
    unsigned char lut1= (val>>CXRCLut::gBits);
    unsigned char lut2= (val>>1) & ((CXRCLut::gSize-1)>>1);
    unsigned char lut3= val & 1;
    UncompressRange(lut1, lut2, lut3, gMin, gMax);
  }

  bool test(unsigned char r, unsigned char g, unsigned char b)
	{
	unsigned char min_g, max_g;

	r >>= 3;
	g >>= 3;
	b >>= 3;
	getGRange(r, b, min_g, max_g);
    
    return min_g < g && g <= max_g;
	}


};

#endif
