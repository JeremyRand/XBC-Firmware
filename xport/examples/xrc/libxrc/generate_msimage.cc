#include <stdio.h>
#include "xrc_lut.h"
#include "colormodel.h"
#include "image.h"

#define EMARGIN 1

int main(int argc, char **argv)
{
  CHsvPlane hp(MSI_XSIZE, MSI_YSIZE, MSI_XCENTER, 
	       MSI_START_H, MSI_MIN_S, MSI_MIN_V);
  unsigned short h;
  unsigned char r, g, b, s, v;
  int x,y;

  string outfile = "msimage.ppm";
  bool verbose = false;
  int gridSize = 10;
  bool showgrid = false;

  for (int i=1; i < argc; ++i) {
    if (!strcmp (argv[i], "-o")) {
      if (++i < argc) outfile = argv[i];
    }
    else if (!strcmp (argv[i], "-xSize")) {
      if (++i < argc) hp.xSize = atoi(argv[i]);
    }
    else if (!strcmp (argv[i], "-ySize")) {
      if (++i < argc) hp.ySize = atoi(argv[i]);
    }
    else if (!strcmp (argv[i], "-xCenter")) {
      if (++i < argc) hp.xCenter = atoi(argv[i]);
    }
    else if (!strcmp (argv[i], "-hTop")) {
      if (++i < argc) hp.hTop = atoi(argv[i]);
    }
    else if (!strcmp (argv[i], "-sMin")) {
      if (++i < argc) hp.sMin = atoi(argv[i]);
    }
    else if (!strcmp (argv[i], "-vMin")) {
      if (++i < argc) hp.vMin = atoi(argv[i]);
    }
    else if (!strcmp (argv[i], "-v")) {
      verbose = true;
    }
    else if (!strcmp (argv[i], "-g")) {
      showgrid = true;
    }
  }

  Image img(hp.ySize, hp.xSize);
  for (int y= 0; y < hp.ySize; y++) {
    hp.XYToHSV(0, y, h, s, v);
    int iy = hp.HToY(h);
    if(abs(iy-y)>EMARGIN) {
      printf("y=%d, iy=%d, h=%d\n", y, iy, h);
    }

    for (int x= 0; x < hp.xSize; x++) {
      hp.XYToHSV(x, y, h, s, v);
      HsvToRgbOV6620(h, s, v, r, g, b);
      img.set_rgb(x, y, r, g, b);

      // Test the inverse mapping
      int chkY, left, right;
      bool error=false;

      hp.HSVToSegment(h, s, v, chkY, left, right);
      if(x<hp.xCenter ) {
	if (abs(x - left) > EMARGIN) error = true;
      }
      else if(x>hp.xCenter) {
	if (abs(x - right) > EMARGIN) error = true;
      }
      else if(abs(x - left) > EMARGIN || abs(x - right) > EMARGIN) {
	error = true;
      }

      if(abs(y - chkY)>EMARGIN) error = true;
   
      if(error || verbose) {
	printf("(x,y)=(%3d,%3d) Chk= %3d: %3d->%3d  HSV=(%3d, %3d, %3d) %s\n", 
	       x, y, chkY, left, right, (int)h, (int)s, (int)v, 
	       (verbose?(error?"ERROR":""):""));
      }
    }
  }

//   if(showgrid) {
//     for (h= 0; h < MAX_H; h+=gridSize) {
//       for (s = MAX_S; s > hp.sMin; s-=gridSize) {
//       hp.XYToHSV(x, y, h, s, v);
//       HsvToRgbOV6620(h, s, v, r, g, b);
//       img.set_rgb(x, y, r, g, b);

//       // Test the inverse mapping
//       int chkY, left, right;
//       bool error=false;

//       hp.HSVToSegment(h, s, v, chkY, left, right);
//       if(x<hp.xCenter ) {
// 	if (abs(x - left) > EMARGIN) error = true;
//       }
//       else if(x>hp.xCenter) {
// 	if (abs(x - right) > EMARGIN) error = true;
//       }
//       else if(abs(x - left) > EMARGIN || abs(x - right) > EMARGIN) {
// 	error = true;
//       }

//       if(abs(y - chkY)>EMARGIN) error = true;
   
//       if(error || verbose) {
// 	printf("(x,y)=(%3d,%3d) Chk= %3d: %3d->%3d  HSV=(%3d, %3d, %3d) %s\n", 
// 	       x, y, chkY, left, right, (int)h, (int)s, (int)v, 
// 	       (verbose?(error?"ERROR":""):""));
//       }
//     }
//   }

  FILE *out = fopen(outfile.c_str(), "w");
  img.write_ppm(out);
  fclose(out);
}
