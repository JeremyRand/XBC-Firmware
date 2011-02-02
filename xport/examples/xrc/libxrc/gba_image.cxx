#define GBA 1
#include <gba.h>
#include <regbits.h>

#include "image.cc"

// This is a special Image referring to the GBA screen
Image GbaScreenImage(GBA_NROWS, GBA_NCOLS, GBA_BASE_VRAM);

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
  BFSET(GBA_REG_DISPCNT, videoMode, GBA_BG_MODE_3);
  BFSET(GBA_REG_DISPCNT, drawBG2, 1);
    
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
