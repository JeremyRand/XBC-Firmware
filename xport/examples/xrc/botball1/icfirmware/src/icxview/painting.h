#ifndef PAINTING_H
#define PAINTING_H

#ifndef NO_ICXVIEW

//#include "ICXView.h"
#include "btnstate.h"
#include "gba.h"
#include "regbits.h"
#include <unistd.h>

#define CSstep 1
#define PAL_START 140
#define WAIT 300

#define GBA_SCREEN_WIDTH		240
#define GBA_SCREEN_HEIGHT		160

#define RGB16(r,g,b)  (((b)<<10)+((g)<<5)+(r))
#define VideoBuffer 	((unsigned short *)0x6000000)

#define BLUE 0x7C00 //0111 1100 0000 0000
#define GREEN 0x03E0 //0000 0011 1110 0000
#define RED 0x001F //0000 0000 0001 1111

class ICXPaint
{

public:
	ICXPaint();
	~ICXPaint();
	void StartPainting();

protected:
	short CSx, CSy;
	short CSsize, CScolor, CSbackcolor;
	short CSwait,Pwait,Swait;
	short * CSunder;
	short slmo;
	CBtnState btnState;

	void CSredraw(short x, short y, short oldX, short oldY);
	void paintPalette();
	void paintHere(short color);
	void paintColor();
	void changeColor();
};


#endif
#endif
