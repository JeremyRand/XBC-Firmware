#include "painting.h"

#ifndef NO_ICXVIEW
ICXPaint::ICXPaint()
{
	/*CSsize=2;
	CSunder = new short[CSsize*CSsize];
	CSy=80;
	CSx=120;
	btnState = CBtnState();
	CScolor=RGB16(31,0,0);
	CSbackcolor=RGB16(31,31,31);*/
}

ICXPaint::~ICXPaint()
{
}

//puts cursor in new position
void ICXPaint::CSredraw(short x, short y, short oldX, short oldY)
{
	//put old stuff back on cursor
	short oldPos = oldX + oldY*GBA_SCREEN_WIDTH;
	for(short cx=0;cx<CSsize;cx++)
	{
		for(short cy=0;cy<CSsize;cy++)
		{
			VideoBuffer[ oldPos + cx + cy * GBA_SCREEN_WIDTH ]
						=CSunder[cx + cy*CSsize];
		}
	}
	//put cursor in new place
	short newPos = x + y*GBA_SCREEN_WIDTH;
	for(short cx=0;cx<CSsize;cx++)
	{
		for(short cy=0;cy<CSsize;cy++)
		{
			CSunder[cx + cy*CSsize]
							=
					VideoBuffer[ newPos + cx + cy * GBA_SCREEN_WIDTH ];
			VideoBuffer[ newPos + cx + cy * GBA_SCREEN_WIDTH ]
						= CScolor;
		}
	}
}

//draws the palette
void ICXPaint::paintPalette()
{
	//put the palette at the bottom
	//palette info:
	//takes up bottom 20 px, 20x240
	//MIXES THE COLORS!!! SWEEEET
	//Red, Green, Blue, White, Black
	//each is 20x48
	//really 18x48, 2px line at the top for the current color

	paintColor();
	
	//First, red
	for(short y=0;y<18;y++)
	{
		for(short x=0;x<48;x++)
		{
			VideoBuffer[ x      + (PAL_START+2+y)*GBA_SCREEN_WIDTH]//RED
					= RED;
			VideoBuffer[(x+ 48) + (PAL_START+2+y)*GBA_SCREEN_WIDTH]//GREEN
					= GREEN;
			VideoBuffer[(x+ 96) + (PAL_START+2+y)*GBA_SCREEN_WIDTH]//BLUE
					= BLUE;
			VideoBuffer[(x+144) + (PAL_START+2+y)*GBA_SCREEN_WIDTH]//WHITE
					= RGB16(31,31,31);
			VideoBuffer[(x+192) + (PAL_START+2+y)*GBA_SCREEN_WIDTH]//BLACK
					= 0;
		}
	}
}

//paints the color right above the palette
void ICXPaint::paintColor()
{
	//ok, so, draw first two lines
	for(short x=0;x<GBA_SCREEN_WIDTH;x++)
	{
		VideoBuffer[x+PAL_START*GBA_SCREEN_WIDTH]=CScolor;
		VideoBuffer[x+(PAL_START+1)*GBA_SCREEN_WIDTH]=CScolor;
	}
}

void ICXPaint::paintHere(short color)
{
	for(short y=0;y<CSsize;y++)
		for(short x=0;x<CSsize;x++)
			CSunder[x+y*CSsize]=color;
}

void ICXPaint::StartPainting()
{
	//init
	CSsize=4;
	CSunder = new short[CSsize*CSsize];
	CSy=80;
	CSx=120;
	btnState = CBtnState();
	CScolor=RGB16(31,0,0);
	CSbackcolor=RGB16(31,31,31);
	CSwait = 0;
	Pwait = 0;
	Swait = 0;
	slmo = 1; // Slow mo: 5-Slow, 1-Normal


//Paint method!
	paintPalette();
	
	//Palette starts at 140
	
	
	while(1)
	{
		btnState.PollKeys();									//poll keys
		if(btnState.KeyDown(btnState.START_BUTTON))				//start button?
			break;													//break;
		if(btnState.KeyDown(btnState.UP_BUTTON) && (CSwait--)<0 )//arrow keys?
		{
			if(CSy-CSstep >= 0)
			{
				CSredraw(CSx       ,CSy-CSstep,CSx,CSy);
				CSy-=CSstep;
				//CSx+=0     ;
			}
			CSwait = slmo * WAIT;
		}
		else if(btnState.KeyDown(btnState.DOWN_BUTTON) && (CSwait--)<0 )
		{
			if(CSy+CSsize + CSstep < GBA_SCREEN_HEIGHT)
			{
				CSredraw(CSx       ,CSy+CSstep,CSx,CSy);
				CSy+=CSstep;
				//CSx+=0;
			}
			CSwait = slmo * WAIT;
		}
		else if(btnState.KeyDown(btnState.RIGHT_BUTTON) && (CSwait--)<0 )
		{
			if( CSx+CSsize + CSstep <= GBA_SCREEN_WIDTH)
			{
				CSredraw(CSx+CSstep,CSy       ,CSx,CSy);
				//CSy+=0     ;
				CSx+=CSstep;
			}
			CSwait = slmo * WAIT;
		}
		else if(btnState.KeyDown(btnState.LEFT_BUTTON) && (CSwait--)<0 )
		{
			if(CSx-CSstep >= 0)
			{
				CSredraw(CSx-CSstep,CSy       ,CSx,CSy);
				//CSy+=0     ;
				CSx-=CSstep;
			}
			CSwait = slmo * WAIT;
		}

		if(btnState.KeyDown(btnState.A_BUTTON) && (Pwait--)<0 )//a button?
		{
			if(CSy>PAL_START-2) {
				changeColor();
				Pwait = 16*WAIT;
			} else {
				paintHere(CScolor);
				Pwait = WAIT;
			}
		}//put actual dot there
		else if(btnState.KeyDown(btnState.B_BUTTON) && (Pwait--)<0 )//b button?
		{
			if(CSy>PAL_START-2) {
				changeColor();
				Pwait = 16*WAIT;
			} else {
				paintHere(CSbackcolor);
				Pwait = WAIT;
			}
		}//put other dot there
		if(btnState.KeyDown(btnState.R_BUTTON) && (Swait--)<0) //r button/l button?
		{
			CSsize++;
			/*short * newUnder = new short[CSsize*CSsize];
			short curpos = CSx + CSy*GBA_SCREEN_WIDTH;
			for(short cx=0;cx<CSsize;cx++)
			{
				for(short cy=0;cy<CSsize;cy++)
				{
					if(cx<CSsize-1 && cy<CSsize-1)
						newUnder[cx + cy*(CSsize-1)]
									=
							CSunder[cx + cy*(CSsize-1)];
					else
						newUnder[cx + cy*CSsize]
									=
							VideoBuffer[ curpos + cx + cy * GBA_SCREEN_WIDTH ];
				}
			}
			CSunder = newUnder;*/
			CSunder = new short[CSsize*CSsize];
		}else if(btnState.KeyDown(btnState.L_BUTTON) && (Swait--)<0){
			CSsize--;
			/*short * newUnder = new short[CSsize*CSsize];
			for(short cx=0;cx<CSsize;cx++)
			{
				for(short cy=0;cy<CSsize;cy++)
				{
					newUnder[cx + cy*CSsize]
									=
							CSunder[cx + cy*(CSsize+1)];
				}
			}
			CSunder = newUnder;*/
			CSunder = new short[CSsize*CSsize];
		}//change cursor size, r = bigger

		if(btnState.KeyDown(btnState.SELECT_BUTTON)) //select button?
		{
			slmo=5;
		}else if(slmo==5){
			slmo=1;
		}//Slow down												
	}
}

void ICXPaint::changeColor()
{
	//Interesting method....
	//We "mix" the colors, so when they click on a color
	//   they are "adding" that color. So they make purple
	//   by clicking on red and blue once.
	//   More redish purple is once more on red....

	//AH HA!!! Take the average of the new color and the old color!!!

	//However, to get a full white or black would take forever, so 
	//   a R_BUTTON+A/B makes a full switch

	if(CSy>PAL_START+2)
	{//In the right area
		if(btnState.KeyDown(btnState.A_BUTTON))
		{//A button is pressed
			short halfCScolor =	  (((CScolor & RED)/3)*2 & RED)
								+ (((CScolor & GREEN)/3)*2 & GREEN)
								+ (((CScolor & BLUE)/3)*2 & BLUE);
#define REPME 10
			if(CSx<48){		  //Red?
				CScolor = RGB16(REPME,0,0) + halfCScolor;
			}else if(CSx<96){ //Green?
				CScolor = RGB16(0,REPME,0) + halfCScolor;
			}else if(CSx<144){ //Blue?
				CScolor = RGB16(0,0,REPME) + halfCScolor;
			}else if(CSx<192){ //White?
				if(btnState.KeyDown(btnState.R_BUTTON))
					CScolor = RGB16(31,31,31);
				else
					CScolor = RGB16(REPME,REPME,REPME) + halfCScolor;
			}else{			  //Black?
				if(btnState.KeyDown(btnState.R_BUTTON))
					CScolor = RGB16(0,0,0);
				else
					CScolor =					halfCScolor;
			}
		}else{//B button pressed
			short halfCSbackcolor =	  (CSbackcolor/2 & RGB16(31,0 ,0 ))
									+ (CSbackcolor/2 & RGB16(0 ,31,0 ))
									+ (CSbackcolor/2 & RGB16(0 ,0 ,31));
			if(CSx<48){		  //Red?
				CSbackcolor = RGB16(15,0 ,0 ) + halfCSbackcolor;
			}else if(CSx<96){ //Green?
				CSbackcolor = RGB16(0 ,15,0 ) + halfCSbackcolor;
			}else if(CSx<144){ //Blue?
				CSbackcolor = RGB16(0 ,0 ,15) + halfCSbackcolor;
			}else if(CSx<192){ //White?
				if(btnState.KeyDown(btnState.R_BUTTON))
					CSbackcolor = RGB16(31,31,31);
				else
					CSbackcolor = RGB16(15,15,15) + halfCSbackcolor;
			}else{			  //Black?
				if(btnState.KeyDown(btnState.R_BUTTON))
					CSbackcolor = RGB16(0,0,0);
				else
					CSbackcolor =					halfCSbackcolor;
			}
		}
		paintColor();
#undef REPME
	}
}

#endif
