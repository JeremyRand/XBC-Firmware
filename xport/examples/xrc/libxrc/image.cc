#include <algorithm>
#include "image.h"
#include <assert.h>

void
Image::draw_line(int sx, int sy, int ex, int ey, TPix value) 
{
	//Bresenham's line drawing algorithm
	//As seen in the wikipedia article: 
	// http://en.wikipedia.org/wiki/Bresenham's_line_algorithm
	// (implemented from pseudocode)
		
	int steep = 0;
	int x0,x1,y0,y1,temp;
	int deltax,deltay;
	int error,ystep;
	int x,y;
	
	if(sx<0 || sy<0 || ex<0 || ey<0) return;
	if(sx>=ncols || sy>=nrows || ex>=ncols || ey>=nrows) return;
	
	if(abs(ey-sy) > abs(ex-sx))
		steep = 1;
	if(steep) {
		x0=sy;
		y0=sx;
		x1=ey;
		y1=ex;
	}
	else {
		x0=sx;
		y0=sy;
		x1=ex;
		y1=ey;
	}
	if(x0 > x1) {
		temp=x0;
		x0=x1;
		x1=temp;
		temp=y0;
		y0=y1;
		y1=temp;
	}
	deltax = x1 - x0;
	deltay = abs(y1-y0);
	error = 0;
	y = y0;
	if(y0 < y1)
		ystep = 1;
	else
		ystep = -1;
	for(x = x0;x <= x1;x++) {
		if(steep)
			pixel(y,x) = value;
		else
			pixel(x,y) = value;
		error += deltay;
		if((error<<1) >= deltax) {
			y += ystep;
			error -= deltax;
		}
	}
}

void 
Image::draw_cross(int x, int y, int size, TPix value) 
{
  int sx = max(0, x-size), ex = min(ncols-1, x+size);
  int sy = max(0, y-size), ey = min(nrows-1, y+size);
  draw_line(sx, y, ex, y, value);
  draw_line(x, sy, x, ey, value);
}
void 
Image::draw_box(int sx, int sy, int ex, int ey, TPix value) 
{
  draw_line(sx, sy, ex, sy, value);
  draw_line(sx, sy, sx, ey, value);
  draw_line(ex, sy, ex, ey, value);
  draw_line(sx, ey, ex, ey, value);
}

void 
Image::draw_fillrect(int sx, int sy, int ex, int ey, TPix value) 
{
  for(int x = sx; x<=ex; x++) {
    for(int y = sy; y<=ey; y++) {
      pixel(x, y) = value;
    }
  }
}

