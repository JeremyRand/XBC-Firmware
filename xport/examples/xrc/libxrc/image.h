#ifndef IMAGE_H
#define IMAGE_H

#include <stdlib.h>

using namespace std;

#ifdef GBA
//////////////////////////////////////////////////////////////////////
// Setup includes, types, and defines for Gameboy
#include <gba.h>
#include "vision.h"
typedef unsigned short TPix;

#define GBA_NROWS 160
#define GBA_NCOLS 240
#else
//////////////////////////////////////////////////////////////////////
// Setup includes, types, and defines for Host
#include "file_utils.h"
#include "hsv.h"

typedef int TPix;

void chomp(string &str);
#endif

// Hue from 0 to 360
// 0:   red
// 60:  yellow
// 120: green
// 180: cyan
// 240: blue
// 300: magenta

class Image {
public:
  TPix *image;
  int nrows, ncols;
  bool do_free;

public:
  /////////////////////////////////////////////////////////////////
  // Common to both host and GBA
  Image(int nrows_init=0, int ncols_init=0) {
    image= NULL;
    nrows=ncols=0;
    do_free=true;
    resize(nrows_init, ncols_init);
  }
  Image(int nrows_init, int ncols_init, TPix *imgbuf) {
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
      image= (TPix*)realloc((void*)image, new_nrows*new_ncols*sizeof(TPix));
    nrows= new_nrows; ncols= new_ncols;
  }
  TPix &pixel(int x, int y) {
    return image[x+y*ncols];
  }
  const TPix pixel(int x, int y) const {
    return image[x+y*ncols];
  }
  void set_rgb(int x, int y,
               unsigned char r, unsigned char g, unsigned char b) {
    pixel(x,y)=make_rgb(r,g,b);
  }
  void get_rgb(int x, int y,
               unsigned char &r, unsigned char &g, unsigned char &b)  const {
    read_rgb(pixel(x,y), r, g, b);
  }
  static void read_rgb(TPix pixel,
		       unsigned char &r, unsigned char &g, unsigned char &b) {
#ifdef GBA
    r= (unsigned char) (pixel&0x1f);
    g= (unsigned char) (pixel>>5)&0x1f;
    b= (unsigned char) (pixel>>10)&0x1f;
#else 
    r= (unsigned char) (pixel>>16);
    g= (unsigned char) (pixel>>8);
    b= (unsigned char) pixel;
#endif
  }
  Image &operator=(const Image &rhs) {
    resize(rhs.nrows, rhs.ncols);
    for (int i= 0; i< nrows*ncols; i++) image[i]=rhs.image[i];
    return *this;
  }
  void fill(TPix pixel) {
    for (int i= 0; i< nrows*ncols; i++) image[i]= pixel;
  }
  void draw_line(int sx, int sy, int ex, int ey, TPix value);
  void draw_cross(int x, int y, int size, TPix value);
  void draw_box(int sx, int sy, int ex, int ey, TPix value);
  void draw_fillrect(int sx, int sy, int ex, int ey, TPix value);

  /////////////////////////////////////////////////////////////////
  // Common to both host and GBA, but some differences
  static TPix make_rgb(unsigned char r, unsigned char g, unsigned char b) {
#ifdef GBA
    return(DV_BUILD_COLOR(r, g, b));
#else 
    return (r << 16) | (g << 8) | b;
#endif
  }

  /////////////////////////////////////////////////////////////////
  // GBA only
#ifdef GBA
  void write_to_gba_screen(int minX=0, int minY=0, int maxX=0, int maxY=0);

  // This assumes it's already in Mode 3 display, and that the image
  // is already set to the right dimensions.  TODO: Check? Use
  // REG_BG2PA/D scalings?
  void read_from_gba_screen(int minX=0, int minY=0, int maxX=0, int maxY=0);
#endif

  /////////////////////////////////////////////////////////////////
  // Host only
#ifndef GBA
  void get_hsv(int x, int y,
               unsigned short &h, unsigned char &s, unsigned char &v)  const;
  bool read(const string &imagename);
  bool read_ppm(const string &imagename);
  bool write_ppm(FILE *out);
#endif
};

#ifdef GBA
// This is a special Image referring to the GBA screen
extern Image GbaScreenImage;
#endif
#endif
