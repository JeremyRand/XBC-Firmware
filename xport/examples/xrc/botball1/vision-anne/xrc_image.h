#ifndef IMAGE_H
#define IMAGE_H

#include <stdlib.h>
#include "../../libxrc/vision.h"

using namespace std;
typedef unsigned short ushort;

// Hue from 0 to 360
// 0:   red
// 60:  yellow
// 120: green
// 180: cyan
// 240: blue
// 300: magenta

#define GBA_NROWS 160
#define GBA_NCOLS 240

class Image {
public:
  ushort *image;
  int nrows, ncols;
  bool do_free;
  Image(int nrows_init=0, int ncols_init=0) {
    image= NULL;
    nrows=ncols=0;
    do_free=true;
    resize(nrows_init, ncols_init);
  }
  Image(int nrows_init, int ncols_init, ushort *imgbuf) {
    image= imgbuf;
    nrows=nrows_init;
    ncols=ncols_init;
    do_free=false;
  }

  ~Image() { 
    if(do_free) 
      free(image); 
  }
  void resize(int new_nrows, int new_ncols) {
    if(!do_free) {
      return;
    }
    if (new_nrows != nrows || new_ncols != ncols)
      image= (ushort*)realloc((void*)image, new_nrows*new_ncols*sizeof(ushort));
    nrows= new_nrows; ncols= new_ncols;
  }
  ushort &pixel(int x, int y) {
    return image[x+y*ncols];
  }
  const ushort pixel(int x, int y) const {
    return image[x+y*ncols];
  }
  void set_rgb(int x, int y,
               unsigned char r, unsigned char g, unsigned char b) {
    pixel(x,y)=DV_BUILD_COLOR(r, g, b);
  }
  void get_rgb(int x, int y,
               unsigned char &r, unsigned char &g, unsigned char &b)  const {
    read_rgb(pixel(x,y), r, g, b);
  }
//   void get_hsv(int x, int y,
//                unsigned short &h, unsigned char &s, unsigned char &v)  const {
//     unsigned char r,g,b;
//     get_rgb(x,y,r,g,b);
//     rgb_to_hsv(r,g,b,h,s,v);
//   }
  static ushort make_rgb(unsigned char r, unsigned char g, unsigned char b) {
    return(DV_BUILD_COLOR(r, g, b));
  }
  static void read_rgb(ushort pixel,
		       unsigned char &r, unsigned char &g, unsigned char &b) {
    r= (unsigned char) (pixel&0x1f);
    g= (unsigned char) (pixel>>5)&0x1f;
    b= (unsigned char) (pixel>>10)&0x1f;
  }
  Image &operator=(const Image &rhs) {
    resize(rhs.nrows, rhs.ncols);
    for (int i= 0; i< nrows*ncols; i++) image[i]=rhs.image[i];
    return *this;
  }
  void fill(ushort pixel) {
    for (int i= 0; i< nrows*ncols; i++) image[i]= pixel;
  }
  void draw_line(int sx, int sy, int ex, int ey, ushort value);
  void draw_cross(int x, int y, int size, ushort value);
  void draw_box(int sx, int sy, int ex, int ey, ushort value);
  void draw_fillrect(int sx, int sy, int ex, int ey, ushort value);

  void write_to_gba_screen(int minX=0, int minY=0, int maxX=0, int maxY=0);

  // This assumes it's already in Mode 3 display, and that the image
  // is already set to the right dimensions.  TODO: Check? Use
  // REG_BG2PA/D scalings?
  void read_from_gba_screen(int minX=0, int minY=0, int maxX=0, int maxY=0);
};

// This is a special Image referring to the GBA screen
extern Image GbaScreenImage;
#endif
