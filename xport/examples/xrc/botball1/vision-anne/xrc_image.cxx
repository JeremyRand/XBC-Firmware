#include "xrc_image.h"

#include <algorithm>
#include <gba.h>

// This is a special Image referring to the GBA screen
Image GbaScreenImage(GBA_NROWS, GBA_NCOLS, GBA_BASE_VRAM);

void
Image::draw_line(int sx, int sy, int ex, int ey, ushort value) 
{
  // Check bounds
  if(sx<0) sx=0;
  if(sy<0) sy=0;
  if(ex>=ncols) ex=ncols-1;
  if(ey>=ncols) ey=nrows-1;

  // Can currently only draw horizontal or vertical lines
  // TODO: Make diagonal lines work.
  if(sx == ex) {
    assert(ey >= sy);
    for(int y = sy; y<=ey; y++) {
      pixel(sx, y) = value;
    }
  }
  else if(sy == ey) {
    assert(ex >= sx);
    for(int x = sx; x<=ex; x++) {
      pixel(x, sy) = value;
    }
  }
}

void 
Image::draw_cross(int x, int y, int size, ushort value) 
{
  int sx = max(0, x-size), ex = min(ncols-1, x+size);
  int sy = max(0, y-size), ey = min(nrows-1, y+size);
  draw_line(sx, y, ex, y, value);
  draw_line(x, sy, x, ey, value);
}
void 
Image::draw_box(int sx, int sy, int ex, int ey, ushort value) 
{
  draw_line(sx, sy, ex, sy, value);
  draw_line(sx, sy, sx, ey, value);
  draw_line(ex, sy, ex, ey, value);
  draw_line(sx, ey, ex, ey, value);
}

void 
Image::draw_fillrect(int sx, int sy, int ex, int ey, ushort value) 
{
  for(int x = sx; x<=ex; x++) {
    for(int y = sy; y<=ey; y++) {
      pixel(x, y) = value;
    }
  }
}

void 
Image::write_to_gba_screen(int minX, int minY, int maxX, int maxY) 
{
  int i,j;
  unsigned short *sp; 

  if(maxX==0 && maxY==0) {
    maxX = GBA_NCOLS;
    maxY = GBA_NROWS;
  }

  // Set high-res graphics mode
  GBA_REG_DISPCNT = GBA_BG_MODE_3 | GBA_BG2_ENABLE<<8;
    
  // Set texture scaling to show full picture
  GBA_REG_BG2PA = (ncols<<8)/240;
  GBA_REG_BG2PD = (nrows<<8)/160;

  for (i=minY, sp=(unsigned short *)GBA_BASE_VRAM + (GBA_NCOLS * minY); 
       i<maxY && i<nrows; 
       i++, sp+=GBA_NCOLS) {
    for (j=minX; j<maxX && j<ncols; j++) {
      *(sp + j) = pixel(j,i);
    }
  }
}

// This assumes it's already in Mode 3 display, and that the image
// is already set to the right dimensions.  TODO: Check? Use
// REG_BG2PA/D scalings?
void 
Image::read_from_gba_screen(int minX, int minY, int maxX, int maxY) 
{
  int i,j;
  unsigned short *sp; 

  if(maxX==0 && maxY==0) {
    maxX = GBA_NCOLS;
    maxY = GBA_NROWS;
  }

  for (i=minY, sp=((unsigned short *)GBA_BASE_VRAM) + (GBA_NCOLS * minY); 
       i<maxY && i<nrows; 
       i++, sp+=GBA_NCOLS) {
    for (j=minX; j<maxX && j<ncols; j++) {
      pixel(j,i) = *(sp + j);
    }
  }
}
