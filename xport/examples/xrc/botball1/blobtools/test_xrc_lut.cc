#include "xrc_lut.h"
#include "image.h"

// h from 0 to 360
// s from 0 to 224
// v from 0 to 224

void image_threshold_hsv_cube_xrc(const Image &src, Image &dest,
                                  int min_h, int max_h,
                                  int min_s, int max_s,
                                  int min_v, int max_v)
{
  CXRCLut model;
  model.ComputeFromHSVCube(min_h, max_h, min_s, max_s, min_v, max_v);
  //model.print();

  dest.resize(src.nrows, src.ncols);
  // Now filter the pixels using the LUT
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      unsigned char r8,g8,b8;
      src.get_rgb(x,y,r8,g8,b8);
      unsigned char r = r8>>(8-CXRCLut::rBits);
      unsigned char g = g8>>(8-CXRCLut::gBits);
      unsigned char b = b8>>(8-CXRCLut::bBits);
      unsigned char min_g, max_g;
      model.getGRange(r, b, min_g, max_g);
      
      bool ok= min_g < g && g <= max_g;
      if (ok) {
        dest.set_rgb(x, y, r8/2+128, g8/2+128, b8/2+128);
      } else {
        dest.pixel(x,y)=0;
      }
    }
  }
}
