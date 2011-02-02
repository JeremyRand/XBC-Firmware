#include "ICXView.h"

#ifndef NO_ICXVIEW
bool ICXView::SetViewOn()
{
	//Turn to ICXView
	//Change to Mode 3 and blank BGs
	BFSET(GBA_REG_DISPCNT, videoMode, GBA_BG_MODE_3);
	BFSET(GBA_REG_DISPCNT, drawBG2, 1);
	BFSET(GBA_REG_DISPCNT, drawBG0, 0);
	BFSET(GBA_REG_DISPCNT, drawBG1, 0);
	BFSET(GBA_REG_DISPCNT, drawBG3, 0);
	BFSET(GBA_REG_DISPCNT, forceScreenBlank, 0);

	//um....prove you can write to the screen at all?
	//OH SNAP I CAN!!!

	int x,y;
	for(x = 0; x < GBA_SCREEN_WIDTH; x++)   
        for(y = 0; y < GBA_SCREEN_HEIGHT; y++)  
           VideoBuffer [x + y * GBA_SCREEN_WIDTH] = RGB16(31,31,31);

	v_on=true;
	//DrawLoop();

	return v_on;
}

void ICXView::Blank(unsigned short color)
{
	if(!v_on) return;
	while(BFGET(GBA_REG_DISPSTAT, isVBlank));

	int x,y;
	for(x = 0; x < GBA_SCREEN_WIDTH; x++)   
        for(y = 0; y < GBA_SCREEN_HEIGHT; y++)  
           VideoBuffer [x + y * GBA_SCREEN_WIDTH] = color;
}


void ICXView::SetColor(short x, short y, short color)
{
	if(!v_on) return;

    VideoBuffer[x + (y * GBA_SCREEN_WIDTH)] = color;//RGB16(31,0,0);
}


short ICXView::GetColor(short x, short y)
{
	if(v_on)
		return VideoBuffer[x + y * GBA_SCREEN_WIDTH];
	else
		return 0;
}

void ICXView::DrawRect(short left, short top, short width, short height, unsigned short color)
{
	// Draws a non-filled rectangle.
	// Added by Jeremy Rand.
	// Tested on Sep. 26, 2006 by Jeremy Rand
	// Bug: bottom right corner is not rendered.  Should be easy to fix.

	// Do bounds checking (ToDo)

	if(!v_on) return;
	while(BFGET(GBA_REG_DISPSTAT, isVBlank));

	int x,y;

	// Display top edge.
	y = top;
	for(x = left; x < left+width; x++)
		VideoBuffer [x + y * GBA_SCREEN_WIDTH] = color;
	
	// Display bottom edge.
	y = top + height;
	for(x = left; x < left+width; x++)
		VideoBuffer [x + y * GBA_SCREEN_WIDTH] = color;

	// Display left edge.
	x = left;
	for(y = top; y < top+height; y++)
		VideoBuffer [x + y * GBA_SCREEN_WIDTH] = color;
	
	// Display right edge.
	x = left + width;
	for(y = top; y < top+height; y++)
		VideoBuffer [x + y * GBA_SCREEN_WIDTH] = color;
}

void ICXView::DrawRectPixIDs(int topleft, int dimensions, unsigned short color)
{
	// DrawRect with Pixel IDs rather than coordinates.
	// Added by Jeremy Rand.
	// Tested on Sep. 26, 2006 by Jeremy Rand.

	int left, top, width, height;
	
	PixelID2Coords(topleft, &left, &top);
	PixelID2Coords(dimensions, &width, &height);

	DrawRect(left, top, width, height, color);
}

void ICXView::PixelID2Coords(int pixelid, int *x, int *y)
{
	// Converts a single int value containing the position of a pixel in the video buffer into the x and y coords
	// of that pixel.
	// Added by Jeremy Rand.
	// Tested on Sep. 26, 2006 by Jeremy Rand.

	// Need to do bounds checking.

	*x = pixelid % GBA_SCREEN_WIDTH;
	*y = pixelid / GBA_SCREEN_WIDTH;
}

bool ICXView::SetViewOff()
{
	//Back to normal... oh shit?
	//Change to Mode 0
	BFSET(GBA_REG_DISPCNT, videoMode, GBA_BG_MODE_0);
	//Turn on BG0,BG2
	BFSET(GBA_REG_DISPCNT, drawBG2, 1);
	BFSET(GBA_REG_DISPCNT, drawBG0, 1);
	//Turn off BG3,BG1
	BFSET(GBA_REG_DISPCNT, drawBG3, 0);
	BFSET(GBA_REG_DISPCNT, drawBG1, 0);
	//BFSET(GBA_REG_DISPCNT, forceScreenBlank, 0);
	v_on=false;
	return v_on;
}

bool ICXView::IsOn()
{
	return v_on;
}

void ICXView::StartPaint()
{
	m_xpaint->StartPainting();
}

ICXView::~ICXView()
{
	SetViewOff();
}

ICXView::ICXView()
: m_xpaint()
{
	m_xpaint = new ICXPaint();
	v_on = false;
}

#endif
/*
For drawing stuff via firmware only:

//#define FAHHEMPICTURE
//#define DINIGROUP
//#define RAINBOW
#define RGB

#ifndef RGB
	short xblack = RGB16(0,0,0);
	short xwhite = RGB16(31,31,31);
	short xblue  = RGB16(0,0,31);
	short xgreen = RGB16(0,31,0);
	short xred   = RGB16(31,0,0);
	short xpurple= RGB16(31,0,31);
#endif

#ifdef FAHHEMPICTURE
#include "fahhemPicture.h"
#elif defined(DINIGROUP)
#include "dinigroup.h"
#elif defined(RAINBOW)
#include "rainbowRGB.h"
#endif

#ifndef RGB
	printArray(picarray, width, height, 0, 40, 5);
#else
//	printRGB(picR,picG,picB,width,height,40,50,5);
#endif

void ICXView::printArray(unsigned short *graphics, int width, int height, int xOffset, int yOffset, int mult)
{
	//int end=xOffset + width;
	//int bottom=yOffset + height;

	while(!BFGET(GBA_REG_DISPSTAT, isVBlank));

	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			for(int xp=0;xp<mult;xp++)
				for(int yp=0;yp<mult;yp++)
					VideoBuffer [(x*mult+xOffset+xp) 
											+ 
									((y*mult+yOffset+yp) * GBA_SCREEN_WIDTH)]
												=
											graphics[x + y * width];
		}
	}
}

void ICXView::printRGB(u8 *picR, u8 *picG, u8 *picB, int width, int height, int xOffset, int yOffset, int mult)
{
	unsigned int pixel;
	while(!BFGET(GBA_REG_DISPSTAT, isVBlank));
	for(int y=0; y<height; y++)
	{
		for(int x=0; x<width; x++)
		{
			for(int xp=0;xp<mult;xp++)
			{
				for(int yp=0;yp<mult;yp++)
				{
					pixel = RGB16(
						( picR[x/8+y*width/8] & 1<<(7-x%8) )?31:0,
						( picG[x/8+y*width/8] & 1<<(7-x%8) )?31:0,
						( picB[x/8+y*width/8] & 1<<(7-x%8) )?31:0
							);

					VideoBuffer[(x*mult+xOffset+xp) 
											+ 
									((y*mult+yOffset+yp) * GBA_SCREEN_WIDTH)]
												= pixel;

				}
			}
		}
	}
}

*/
