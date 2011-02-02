#include "colormodel.h"
#if !defined(USE_HOST) && !defined(HOST)
#include <textdisp.h>
#else 
#include <stdio.h>
#endif

//////////////////////////////////////////////////////////////////////
// Color model HSV
CColorModelHSV::CColorModelHSV(short ihMin, short ihMax,
			       short isMin, short isMax,
			       short ivMin, short ivMax):
  hMin(ihMin), hMax(ihMax),
  sMin(isMin), sMax(isMax),
  vMin(ivMin), vMax(ivMax)
{
}

void
CColorModelHSV::Set(short ihMin, short ihMax,
		    short isMin, short isMax,
		    short ivMin, short ivMax)
{
  hMin = ihMin;
  sMin = isMin;
  vMin = ivMin;
  hMax = ihMax;
  sMax = isMax;
  vMax = ivMax;
}

void
CColorModelHSV::Limit()
{
  if(hMin < 0) hMin = 0;
  if(hMin > MAX_H) hMax = MAX_H;
  if(hMax < hMin) hMax = hMin;
  if(hMax - hMin > MAX_HRANGE) hMax = hMin + MAX_HRANGE;

  if(sMin < 0) sMin = 0;
  if(sMin > MAX_S) sMax = MAX_S;
  if(sMax < sMin) sMax = sMin;

  if(vMin < 0) vMin = 0;
  if(vMin > MAX_V) vMax = MAX_V;
  if(vMax < vMin) vMax = vMin;
}

void
CColorModelHSV::Print() const
{
  printf("  H: (%3d->%3d)\n", hMin, hMax);
  printf("  S: (%3d->%3d)\n", sMin, sMax);
  printf("  V: (%3d->%3d)\n", vMin, vMax);
}

//////////////////////////////////////////////////////////////////////
// Color selection HSV

CHsvPlane::CHsvPlane(int ixSize, int iySize, int ixCenter, 
			   int ihTop, int isMin, int ivMin) :
  xSize(ixSize), ySize(iySize), xCenter(ixCenter), 
  hTop(ihTop), sMin(isMin), vMin(ivMin)  
{
  if(xCenter == -1) {
    // Default to geometric center if not specified
    xCenter = xSize/2;
  }
}

///////////////////////////////////////////////////////////////////
// HSV -> XY

int CHsvPlane::HToY(unsigned short hue) const
{
  int uncorrectedHue=(hue+MAX_H-hTop)%MAX_H;
  return(uncorrectedHue*ySize/MAX_H);
}

int CHsvPlane::SToX(unsigned short s) const
{
  if(s < sMin) {
    // Generate x coordinate corresponding to sMin
    return(0);
  }
  else {
    return((xCenter*(s-sMin))/(MAX_S-sMin));
  }
}

int CHsvPlane::VToX(unsigned short v) const
{
  if(v < vMin) {
    // Generate x coordinate corresponding to sMin
    return(xSize-1);
  }
  else {
    return(xSize - ((xSize-xCenter)*(v-vMin))/(MAX_V-vMin));
  }
}

void 
CHsvPlane::ModelToBBox(const CColorModelHSV &model, 
			  int &left, int &top, int &right, int &bottom) const
{
  // Generate top based on hMin
  top = HToY(model.hMin);

  // Generate bottom based on hMax
  bottom = HToY(model.hMax);

  // Generate left based on sMin
  left = SToX(model.sMin);

  // Generate right based on vMin
  right = VToX(model.vMin);
}
  
void 
CHsvPlane::HSVToSegment(unsigned short &h, unsigned char &s, 
			   unsigned char &v,
			   int &y, int &leftX, int &rightX) const
{
  // Generate y based on h
  y = HToY(h);

  // Generate leftX based on s
  leftX = SToX(s);

  // Generate right based on v
  rightX = VToX(v);
}

///////////////////////////////////////////////////////////////////
// XY -> HSV 

// Given an (x, y) position in the selection plane, return the
// corresponding HSV value.  This is a well defined mapping and can
// be used to generate color reference images
void 
CHsvPlane::XYToHSV(int x, int y, unsigned short &h, 
		      unsigned char &s, unsigned char &v) const
{
  // Generate y based on y position
  h = (unsigned short)((hTop + (MAX_H*y)/ySize)%MAX_H);

  // Generate s and v based on x position
  if(x<xCenter) {
    s = (unsigned short)(sMin + ((MAX_S-sMin)*x)/xCenter);
    v = MAX_V;
  }
  else if(x==xCenter) {
    s = MAX_S;
    v = MAX_V;
  }
  else {
    s = MAX_S;
    v = (unsigned short)(MAX_V - 
			 ((MAX_V-vMin)*(x-xCenter))/(xSize-1-xCenter));

  }
}

///////////////////////////////////////////////////////////////////
// Hue wrap support

// If third arg non-null sets *trunc if value was truncated, leaves
// alone if not
int 
CHsvPlane::WrapHue(int hinit, int mod, bool *trunc)
{
  int nh = (hinit + MAX_H + mod) % MAX_H;

  if(mod == 0) {
    // Not going anywhere, return init value
    return(hinit);
  }
  else if(mod < 0) {
    // If we're going up allow crossing from 0 to MAX_H, but don't allow 
    // going past hTop
    if(hinit>=hTop && nh <=hTop) {
      nh = hTop;
      if(trunc!=0) *trunc = true;
    }
  }
  else {
    // We're going down, allow crossing from MAX_H to 0, but don't allow
    // going past hTop
    if(hinit < hTop && nh >= hTop) {
      nh = hTop-1;
    }
  }
  return(nh);
}



