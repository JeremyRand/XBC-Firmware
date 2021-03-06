//==========================================================================
//
//      text_disp.c
//
//      LCD text display code for Game Boy Advance
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    rich@charmedlabs.com
// Contributors: rich@charmedlabs.com
// Date:         2002-09-02
// Purpose:      allow text to be displayed on Game Boy Avance LCD
// Description:  Implementations LCD text display code
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <cyg/hal/gba.h>
#include <cyg/hal/text_disp.h>

extern const unsigned char fontData[];

static unsigned short xPos;
static unsigned short yPos;
static unsigned short scrollPos;
static unsigned short *textMap;
static unsigned char bgColor;
static unsigned char textColor;

static void scroll(short val);
static void crlf(void);


void TD_Init(void)
{
	unsigned short i, j, k;
	unsigned char temp8;
	unsigned short *palette = GBA_BASE_PAL_RAM;
	unsigned short *tiles = GBA_BASE_VRAM;
	unsigned char map[6] = {0, 6, 12, 18, 25, 31};

	textMap = GBA_BASE_VRAM + 0x2000;
	GBA_REG_DISPCNT = 0x0400;
	GBA_REG_BG2CNT = 0xa880;
	
	bgColor = 0;
	textColor = 31;
	
	// assume grayscale for now
	for (i=0; i<32; i++)
		palette[i]= i|(i<<5)|(i<<10);
	
	// the rest of the entries are "cube 6" colors (6*6*6 = 216)
	for (i=0; i<6; i++)
		for (j=0; j<6; j++)
			for (k=0; k<6; k++)
				palette[i*36 + j*6 + k + 32] = (map[i]<<10)|(map[j]<<5)|(map[k]*6); 
		
	for(i=32; i<128; i++)
	{
		for (j=0; j<8; j++)
		{
			temp8 = fontData[(i-32)*8 + j];
			tiles[i*32+j*4+0] = ((temp8&0x80)?textColor:bgColor) | ((temp8&0x40)?textColor<<8:bgColor<<8);
			tiles[i*32+j*4+1] = ((temp8&0x20)?textColor:bgColor) | ((temp8&0x10)?textColor<<8:bgColor<<8);
			tiles[i*32+j*4+2] = ((temp8&0x08)?textColor:bgColor) | ((temp8&0x04)?textColor<<8:bgColor<<8);
			tiles[i*32+j*4+3] = ((temp8&0x02)?textColor:bgColor) | ((temp8&0x01)?textColor<<8:bgColor<<8);
		}
	}
	TD_Clear();
}

static void crlf(void)
{
	unsigned char x;
	unsigned short temp16;
	
	xPos = 0;
	yPos++;
	if ((yPos-scrollPos)==TD_SCREEN_HEIGHT)
	{
		scroll(1);
		if (scrollPos>=TD_SCREEN_VHEIGHT && yPos>=TD_SCREEN_VHEIGHT)
		{
			scrollPos -= TD_SCREEN_VHEIGHT;
			yPos -= TD_SCREEN_VHEIGHT;
		}
	}
	
	// clear line
	temp16 = (yPos&(TD_SCREEN_VHEIGHT-1))*TD_SCREEN_VWIDTH; 
    for(x=0; x<TD_SCREEN_WIDTH; x++)
		textMap[temp16 + x] = 32;
}

void TD_PrintChar(char c)
{
	unsigned short temp16;

	temp16 = (yPos&(TD_SCREEN_VHEIGHT-1))*TD_SCREEN_VWIDTH;
	if (xPos==TD_SCREEN_WIDTH)
	{
		crlf();
		temp16 = (yPos&(TD_SCREEN_VHEIGHT-1))*TD_SCREEN_VWIDTH;
	}
	if (c>=32 && c<=127)
	{
		textMap[temp16 + xPos] = (unsigned short)c;
		xPos++;
	}
	else if (c==10)
	{
		crlf();
		temp16 = (yPos&(TD_SCREEN_VHEIGHT-1))*TD_SCREEN_VWIDTH;
	}
	else if (c==8)
	{
		xPos--;
		textMap[temp16 + xPos] = 32;
	}
	else if (c>=0 && c<32)
	{
		textMap[temp16 + xPos] = 32;
		xPos++;
	}	
}


void TD_Print(char *string)
{
	unsigned short i;
	
	for (i=0; string[i]; i++) 
		TD_PrintChar(string[i]);
}


static void scroll(short val)
{
	scrollPos += val;
	GBA_REG_BG2VOFS = (scrollPos&(TD_SCREEN_VHEIGHT-1))*8;
}

void TD_Clear(void)
{
	unsigned char x, y;
	
	xPos = 0;
	yPos = 0;
	scrollPos = 0;
	scroll(0);
	
	for(y=0; y<TD_SCREEN_HEIGHT; y++)
        for(x=0; x<TD_SCREEN_WIDTH; x++)
			textMap[y*TD_SCREEN_VWIDTH + x] = 32;
}

void TD_SetPosition(unsigned short x, unsigned short y)
{
	xPos = x;
	yPos = y + scrollPos;
}

void TD_GetPosition(unsigned short *x, unsigned short *y)
{
	if (x)
		*x = xPos;
	if (y)
		*y = (yPos - scrollPos)&(TD_SCREEN_VHEIGHT-1);
}

#if 1
const unsigned char fontData[]=
{	/* 32 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 33 */	0x10, 0x10, 0x10, 0x10, 0x10, 0x00, 0x10, 0x00,
	/* 34 */	0x28, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 35 */	0x28, 0x28, 0xfe, 0x28, 0xfe, 0x28, 0x28, 0x00,
	/* 36 */	0x10, 0x7c, 0x80, 0x7c, 0x02, 0x7c, 0x10, 0x00,
	/* 36 */	0xc2, 0xc4, 0x08, 0x10, 0x20, 0x46, 0x86, 0x00,
	/* 38 */	0x60, 0x90, 0x60, 0x90, 0x88, 0x84, 0x7a, 0x00,
	/* 39 */	0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 40 */	0x08, 0x10, 0x20, 0x20, 0x20, 0x10, 0x08, 0x00,
	/* 41 */	0x20, 0x10, 0x08, 0x08, 0x08, 0x10, 0x20, 0x00,
	/* 42 */	0x44, 0x28, 0x10, 0xfe, 0x10, 0x28, 0x44, 0x00,
	/* 43 */	0x00, 0x10, 0x10, 0x7c, 0x10, 0x10, 0x00, 0x00,
	/* 44 */	0x00, 0x00, 0x00, 0x00, 0x18, 0x08, 0x10, 0x00,
	/* 45 */	0x00, 0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00,
	/* 46 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00,
	/* 47 */	0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,
	/* 48 */	0x7c, 0x86, 0x8a, 0x92, 0xa2, 0xc2, 0x7c, 0x00,
	/* 49 */	0x10, 0x30, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00,
	/* 50 */	0x7c, 0x82, 0x04, 0x08, 0x10, 0x20, 0xfe, 0x00,
	/* 51 */	0x7c, 0x82, 0x02, 0x3c, 0x02, 0x82, 0x7c, 0x00,
	/* 52 */	0x40, 0x40, 0x84, 0x84, 0xfe, 0x04, 0x04, 0x00,
	/* 53 */	0xfe, 0x80, 0x80, 0xfc, 0x02, 0x82, 0x7c, 0x00,
	/* 54 */	0x10, 0x20, 0x40, 0xfc, 0x82, 0x82, 0x7c, 0x00,
	/* 55 */	0xfe, 0x02, 0x04, 0x08, 0x10, 0x10, 0x10, 0x00,
	/* 56 */	0x7c, 0x82, 0x82, 0x7c, 0x82, 0x82, 0x7c, 0x00,
	/* 57 */	0x7c, 0x82, 0x82, 0x7e, 0x04, 0x08, 0x10, 0x00,
	/* 58 */	0x00, 0x00, 0x10, 0x00, 0x10, 0x00, 0x00, 0x00,
	/* 59 */	0x00, 0x00, 0x10, 0x00, 0x10, 0x10, 0x20, 0x00,
	/* 60 */	0x04, 0x08, 0x10, 0x20, 0x10, 0x08, 0x04, 0x00,
	/* 61 */	0x00, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0x00, 0x00,
	/* 62 */	0x40, 0x20, 0x10, 0x08, 0x10, 0x20, 0x40, 0x00,
	/* 63 */	0x7c, 0x82, 0x04, 0x08, 0x10, 0x00, 0x10, 0x00,
	/* 64 */	0x7c, 0x82, 0xbe, 0xa2, 0xbe, 0x80, 0x7e, 0x00,
	/* 65 */	0x10, 0x28, 0x44, 0x82, 0xfe, 0x82, 0x82, 0x00,
	/* 66 */	0xfc, 0x82, 0x82, 0xfc, 0x82, 0x82, 0xfc, 0x00,
	/* 67 */	0x7c, 0x82, 0x80, 0x80, 0x80, 0x82, 0x7c, 0x00,
	/* 68 */	0xfc, 0x82, 0x82, 0x82, 0x82, 0x82, 0xfc, 0x00,
	/* 69 */	0xfe, 0x80, 0x80, 0xfc, 0x80, 0x80, 0xfe, 0x00,
	/* 70 */	0xfe, 0x80, 0x80, 0xfc, 0x80, 0x80, 0x80, 0x00,
	/* 71 */	0x7c, 0x82, 0x80, 0x8e, 0x82, 0x82, 0x7e, 0x00,
	/* 72 */	0x82, 0x82, 0x82, 0xfe, 0x82, 0x82, 0x82, 0x00,
	/* 73 */	0x38, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00,
	/* 64 */	0x02, 0x02, 0x02, 0x02, 0x02, 0x82, 0x7c, 0x00,
	/* 75 */	0x82, 0x8c, 0xb0, 0xc0, 0xb0, 0x8c, 0x82, 0x00,
	/* 76 */	0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xfe, 0x00,
	/* 77 */	0x82, 0xc6, 0xaa, 0x92, 0x82, 0x82, 0x82, 0x00,
	/* 78 */	0x82, 0xc2, 0xa2, 0x92, 0x8a, 0x86, 0x82, 0x00,
	/* 79 */	0x7c, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00,
	/* 80 */	0xfc, 0x82, 0x82, 0xfc, 0x80, 0x80, 0x80, 0x00,
	/* 81 */	0x7c, 0x82, 0x82, 0x82, 0x82, 0x84, 0x7a, 0x00,
	/* 82 */	0xfc, 0x82, 0x82, 0xfc, 0x88, 0x84, 0x82, 0x00,
	/* 83 */	0x7c, 0x82, 0x80, 0x7c, 0x02, 0x82, 0x7c, 0x00,
	/* 84 */	0xfe, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
	/* 85 */	0x82, 0x82, 0x82, 0x82, 0x82, 0x82, 0x7c, 0x00,
	/* 86 */	0x82, 0x82, 0x82, 0x82, 0x44, 0x28, 0x10, 0x00,
	/* 87 */	0x82, 0x82, 0x82, 0x92, 0x92, 0x54, 0x28, 0x00,
	/* 88 */	0x82, 0x44, 0x28, 0x10, 0x28, 0x44, 0x82, 0x00,
	/* 89 */	0x82, 0x82, 0x44, 0x28, 0x10, 0x10, 0x10, 0x00,
	/* 90 */	0xfe, 0x04, 0x08, 0x10, 0x20, 0x40, 0xfe, 0x00,
	/* 91 */	0x38, 0x20, 0x20, 0x20, 0x20, 0x20, 0x38, 0x00,
	/* 92 */	0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00,
	/* 93 */	0x38, 0x08, 0x08, 0x08, 0x08, 0x08, 0x38, 0x00,
	/* 94 */	0x10, 0x28, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 95 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x00,
	/* 96 */	0x20, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 97 */	0x00, 0x00, 0x7e, 0x82, 0x82, 0x86, 0x7a, 0x00,
	/* 98 */	0x80, 0x80, 0xfc, 0x82, 0x82, 0x82, 0xfc, 0x00,
	/* 99 */	0x00, 0x00, 0x7e, 0x80, 0x80, 0x80, 0x7e, 0x00,
	/* 100 */	0x02, 0x02, 0x7e, 0x82, 0x82, 0x82, 0x7e, 0x00,
	/* 101 */	0x00, 0x00, 0x7c, 0x82, 0xfe, 0x80, 0x7c, 0x00,
	/* 102 */	0x0e, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x10, 0x00,
	/* 103 */	0x00, 0x00, 0x7e, 0x82, 0x7e, 0x02, 0x7c, 0x00,
	/* 104 */	0x80, 0x80, 0xfc, 0x82, 0x82, 0x82, 0x82, 0x00,
	/* 105 */	0x10, 0x00, 0x30, 0x10, 0x10, 0x10, 0x38, 0x00,
	/* 106 */	0x08, 0x00, 0x18, 0x08, 0x08, 0x08, 0x70, 0x00,
	/* 107 */	0x80, 0x80, 0x86, 0x98, 0xe0, 0x98, 0x86, 0x00,
	/* 108 */	0x30, 0x10, 0x10, 0x10, 0x10, 0x10, 0x38, 0x00,
	/* 109 */	0x00, 0x00, 0xec, 0x92, 0x92, 0x92, 0x82, 0x00,
	/* 110 */	0x00, 0x00, 0xbc, 0xc2, 0x82, 0x82, 0x82, 0x00,
	/* 111 */	0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0x7c, 0x00,
	/* 112 */	0x00, 0x00, 0xfc, 0x82, 0xfc, 0x80, 0x80, 0x00,
	/* 113 */	0x00, 0x00, 0x7e, 0x82, 0x7e, 0x02, 0x02, 0x00,
	/* 114 */	0x00, 0x00, 0xbe, 0xc0, 0x80, 0x80, 0x80, 0x00,
	/* 115 */	0x00, 0x00, 0x7e, 0x80, 0x7c, 0x02, 0xfc, 0x00,
	/* 116 */	0x10, 0x10, 0x7c, 0x10, 0x10, 0x10, 0x0c, 0x00,
	/* 117 */	0x00, 0x00, 0x82, 0x82, 0x82, 0x86, 0x7a, 0x00,
	/* 118 */	0x00, 0x00, 0x82, 0x82, 0x44, 0x28, 0x10, 0x00,
	/* 119 */	0x00, 0x00, 0x92, 0x92, 0x92, 0x92, 0x6c, 0x00,
	/* 120 */	0x00, 0x00, 0x82, 0x44, 0x38, 0x44, 0x82, 0x00,
	/* 121 */	0x00, 0x00, 0x82, 0x82, 0x7e, 0x02, 0x7c, 0x00,
	/* 122 */	0x00, 0x00, 0xfe, 0x0c, 0x30, 0xc0, 0xfe, 0x00,
	/* 123 */	0x0c, 0x10, 0x10, 0x20, 0x10, 0x10, 0x0c, 0x00,
	/* 124 */	0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x00,
	/* 125 */	0x60, 0x10, 0x10, 0x08, 0x10, 0x10, 0x60, 0x00,
	/* 126 */	0x34, 0x2c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 127 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
#else
const unsigned char fontData[]=
{
	/* 32 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	/* 33 */	0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00, 
	/* 34 */	0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 
	/* 35 */	0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00, 
	/* 36 */	0x30, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x30, 0x00, 
	/* 37 */	0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00, 
	/* 38 */	0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00, 
	/* 39 */	0x60, 0x60, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	/* 40 */	0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00, 
	/* 41 */	0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00, 
	/* 42 */	0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00, 
	/* 43 */	0x00, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x00, 0x00, 
	/* 44 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x30, 0x60, 
	/* 45 */	0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00, 
	/* 46 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 
	/* 47 */	0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00, 
	/* 48 */	0x78, 0xcc, 0xdc, 0xfc, 0xec, 0xcc, 0x78, 0x00, 
	/* 49 */	0x30, 0xf0, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00, 
	/* 50 */	0x78, 0xcc, 0x0c, 0x38, 0x60, 0xcc, 0xfc, 0x00, 
	/* 51 */	0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00, 
	/* 52 */	0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x0c, 0x00, 
	/* 53 */	0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00, 
	/* 54 */	0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00, 
	/* 55 */	0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x60, 0x60, 0x00, 
	/* 56 */	0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00, 
	/* 57 */	0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00, 
	/* 58 */	0x00, 0x00, 0x30, 0x30, 0x00, 0x30, 0x30, 0x00, 
	/* 59 */	0x00, 0x00, 0x30, 0x30, 0x00, 0x70, 0x30, 0x60, 
	/* 60 */	0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00, 
	/* 61 */	0x00, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x00, 
	/* 62 */	0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00, 
	/* 63 */	0x78, 0xcc, 0x0c, 0x18, 0x30, 0x00, 0x30, 0x00, 
	/* 64 */	0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x78, 0x00, 
	/* 65 */	0x30, 0x78, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0x00, 
	/* 66 */	0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00, 
	/* 67 */	0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00, 
	/* 68 */	0xfc, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xfc, 0x00, 
	/* 69 */	0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00, 
	/* 70 */	0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00, 
	/* 71 */	0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3e, 0x00, 
	/* 72 */	0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00, 
	/* 73 */	0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 
	/* 74 */	0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00, 
	/* 75 */	0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00, 
	/* 76 */	0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00, 
	/* 77 */	0xc6, 0xee, 0xfe, 0xd6, 0xc6, 0xc6, 0xc6, 0x00, 
	/* 78 */	0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00, 
	/* 79 */	0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00, 
	/* 80 */	0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00, 
	/* 81 */	0x78, 0xcc, 0xcc, 0xcc, 0xdc, 0x78, 0x1c, 0x00, 
	/* 82 */	0xfc, 0x66, 0x66, 0x7c, 0x78, 0x6c, 0xe6, 0x00, 
	/* 83 */	0x78, 0xcc, 0xe0, 0x38, 0x1c, 0xcc, 0x78, 0x00, 
	/* 84 */	0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 
	/* 85 */	0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00, 
	/* 86 */	0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00, 
	/* 87 */	0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0xee, 0xc6, 0x00, 
	/* 88 */	0xc6, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0xc6, 0x00, 
	/* 89 */	0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x78, 0x00, 
	/* 90 */	0xfe, 0xcc, 0x98, 0x30, 0x62, 0xc6, 0xfe, 0x00, 
	/* 91 */	0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00, 
	/* 92 */	0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00, 
	/* 93 */	0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00, 
	/* 94 */	0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00, 
	/* 95 */	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 
	/* 96 */	0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 
	/* 97 */	0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00, 
	/* 98 */	0xe0, 0x60, 0x7c, 0x66, 0x66, 0x66, 0xbc, 0x00, 
	/* 99 */	0x00, 0x00, 0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x00, 
	/* 100 */	0x1c, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x76, 0x00, 
	/* 101 */	0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00, 
	/* 102 */	0x38, 0x6c, 0x60, 0xf0, 0x60, 0x60, 0xf0, 0x00, 
	/* 103 */	0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8, 
	/* 104 */	0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00, 
	/* 105 */	0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00, 
	/* 106 */	0x18, 0x00, 0x78, 0x18, 0x18, 0x18, 0xd8, 0x70, 
	/* 107 */	0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00, 
	/* 108 */	0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 
	/* 109 */	0x00, 0x00, 0xec, 0xfe, 0xd6, 0xc6, 0xc6, 0x00, 
	/* 110 */	0x00, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, 
	/* 111 */	0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00, 
	/* 112 */	0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0, 
	/* 113 */	0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e, 
	/* 114 */	0x00, 0x00, 0xd8, 0x6c, 0x6c, 0x60, 0xf0, 0x00, 
	/* 115 */	0x00, 0x00, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x00, 
	/* 116 */	0x10, 0x30, 0x7c, 0x30, 0x30, 0x34, 0x18, 0x00, 
	/* 117 */	0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00, 
	/* 118 */	0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00, 
	/* 119 */	0x00, 0x00, 0xc6, 0xc6, 0xd6, 0xfe, 0x6c, 0x00, 
	/* 120 */	0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00, 
	/* 121 */	0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8, 
	/* 122 */	0x00, 0x00, 0xfc, 0x98, 0x30, 0x64, 0xfc, 0x00, 
	/* 123 */	0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00, 
	/* 124 */	0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 
	/* 125 */	0xe0, 0x30, 0x30, 0x1c, 0x30, 0x30, 0xe0, 0x00, 
	/* 126 */	0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	/* 127 */	0x10, 0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0xfe, 0x00
};
#endif
