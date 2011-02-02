#ifndef ICXVIEW_H
#define ICXVIEW_H

#include "ICRobot.h"

#ifndef NO_ICXVIEW

#include "painting.h"
#include "gba.h"
#include "regbits.h"

#define GBA_SCREEN_WIDTH		240
#define GBA_SCREEN_HEIGHT		160

#define RGB16(r,g,b)  (((b)<<10)+((g)<<5)+(r))
#define VideoBuffer 	((unsigned short *)0x6000000)

//typedef unsigned char u8;

class ICXView
{
public:

	ICXView();
	~ICXView();
	bool SetViewOn();
	bool SetViewOff();
	bool IsOn();
	
	void Blank(unsigned short color);
	void SetColor(short x, short y, short color);
	short GetColor(short x, short y);
	void DrawRect(short left, short top, short width, short height, unsigned short color);
	void DrawRectPixIDs(int topleft, int dimensions, unsigned short color);
	void PixelID2Coords(int pixelid, int *x, int *y);
	void StartPaint();

	//void printRGB(unsigned char *picR, unsigned char *picG, unsigned char *picB, int width, int height, int xOffset, int yOffset, int mult);
	//void printArray(unsigned short *graphics, int width, int height, int xOffset, int yOffset, int mult);
protected:

	//Global vars
	bool v_on;
	ICXPaint *m_xpaint;
	//unsigned short DoubleBuffer[38400]; //Size in pixels of whole screen
	
};

#endif
#endif
