#include <stdarg.h>
#include <stdio.h>
#include "CStatusBar.h"
#include "ICRobot.h"
#include "regbits.h"
#include "display.h"
#include "string.h"
#include "textdisp.h"
#include "simptimer.h"
#include "CPrintBuffer.h"
#define STATUS_BLOCK ((unsigned char)22)


#define HACKEDGRAPHICS
#define HACKEDSOUND


CStatusBar::CStatusBar() : 
m_priorState(INFO_STATE),
m_currentState(INFO_STATE), 
m_infoBuf(NULL), m_infoScreenBuf(NULL)
{

	m_data= (volatile unsigned short *)DG_BASE;
	m_dataDir = m_data+1;

	// m_infoBuf and m_infoScreenBuf are CTextBuf that are the width of
	// the screen and two lines long, plus an extra to deal with padding
	// the final carriage return.  m_infoBuf is an off-screen buffer to
	// hold onto the info text when it is not showing on the screen, and
	// m_infoScreenBuf points at the video memory for the status block
	// for when it is showing on the screen.
	m_infoBuf = new CTextBuf(3, TD_SCREEN_VWIDTH, 0, 2);
	m_infoScreenBuf = new CTextBuf(CDisplay::ScreenBaseBlockAddr(STATUS_BLOCK),
		3, TD_SCREEN_VWIDTH, 0, 2);
	m_infoBuf->SetWrapEnable(false);
	m_infoScreenBuf->SetWrapEnable(false);
	m_infoBuf->Clear();
}

CStatusBar::~CStatusBar()
{
}

void CStatusBar::SetupDisplay()
{
	//Turn on Background 4 settings 
	//0 = 256x256, since our background is 240x160, this should be fine
	BFSET(GBA_REG_BG3CNT, backgroundSize, 0x00);
	//Highest priority
	BFSET(GBA_REG_BG3CNT, priority, 0x0);

	// Setup tile block based on the font info
	CDispFont const *font=CDisplay::GetCurrDispFont();
	if(!font) {
		return;
	}
	BFSET(GBA_REG_BG3CNT, baseTileBlock, 0x0);
	BFSET(GBA_REG_BG3CNT, baseMapBlock, STATUS_BLOCK);
	//No mosaic
	BFSET(GBA_REG_BG3CNT, mosaic, 0x0);
	//256 color palette
	BFSET(GBA_REG_BG3CNT, palette, 0x1);
	BFSET(GBA_REG_DISPCNT, drawBG3, 1);
	GBA_REG_BG3VOFS = (unsigned short)-144;

	// Fill in the unused tiles with 96 which is the leftmost of a
	// row of blank tiles between the normal and inverse font
	// tiles
	unsigned short* mapBaseBlock = 
		(unsigned short*)(0x6000000+(STATUS_BLOCK*2048));
	for(int i = 0; i < 1024; ++i) {
		mapBaseBlock[i] = 96;
	}
}

void CStatusBar::UpdateStatusBar(ICRobot& robot)
{
	if(!robot.UsingDisplay()) return;
	if(m_priorState == INFO_STATE && m_currentState != m_priorState) {
		// Was showing the info page, but not now.  Copy screen to
		// backup and clear screen
		m_infoBuf->CopyFromTextBuf(*m_infoScreenBuf);
		m_infoScreenBuf->Clear();
	}
	if(m_currentState == INFO_STATE && m_currentState != m_priorState)
	{
		// Was not showing the info page, but will now.
		// Copy backup to screen
		m_infoScreenBuf->CopyFromTextBuf(*m_infoBuf);
		m_priorState = m_currentState;
		return;
	}
/*
Use this code to start a new menu thing.
strcpy(m_lineContent[0], "                                ");
strcpy(m_lineContent[1], "                                ");
strcpy(m_lineMask[0],    "00000000000000000000000000000000");
strcpy(m_lineMask[1],    "00000000000000000000000000000000");
*/
	if(m_currentState == COMM_STATE)
	{
		//Don't increase the serial baud rate because the buffer on the XBC
		//Overflows much too easily
		//two strings of length 32, only 30 show up
		char* baud = "      ";
		switch(robot.GetSerialMode())
			{
			case 2600: baud = "9600  "; break;
			case 1302: baud = "19200 "; break;
			case 651:  baud = "38400 "; break;
			case 434:  baud = "57600 "; break;
			case 217:  baud = "115200"; break;
			case 26:   baud = "921600"; break;
			}

		sprintf(m_lineContent[0], "Baud: %s                    ", baud);
		strcpy(m_lineMask[0],     "11111000000000000000000000000000");

		sprintf(m_lineContent[1],  "Graphics: %s   Sound: %s      ",
#ifdef HACKEDGRAPHICS
			"Yes",
#else
			"No ",
#endif
#ifdef HACKEDSOUND
			"Yes"
#else
			"No "
#endif
			);
		strcpy(m_lineMask[1],     "11111111100000001111110000000000");
	}
	else if(m_currentState == INTRPT_STATE)
	{
		pin=0;
		sprintf(m_lineContent[0], "Dgtl:%c%c%c%c%c%c%c%c Tmr0:%04hx 2:%04hx", //11 left
			((*m_dataDir)&(1<<(1*2)))?'I':'O',
			((*m_dataDir)&(1<<(2*2)))?'I':'O',
			((*m_dataDir)&(1<<(3*2)))?'I':'O',
			((*m_dataDir)&(1<<(4*2)))?'I':'O',
			((*m_dataDir)&(1<<(5*2)))?'I':'O',
			((*m_dataDir)&(1<<(6*2)))?'I':'O',
			((*m_dataDir)&(1<<(7*2)))?'I':'O',
			((*m_dataDir)&(1<<(8*2)))?'I':'O',
			GBA_REG_TM0D,
			GBA_REG_TM2D
			);
		sprintf(m_lineContent[1], "Pins:%c%c%c%c%c%c%c%c    1:%04hx 3:%04hx",
			((*m_dataDir)&(1<<(1*2)))?'I':'O',
			((*m_dataDir)&(1<<(2*2)))?'I':'O',
			((*m_dataDir)&(1<<(3*2)))?'I':'O',
			((*m_dataDir)&(1<<(4*2)))?'I':'O',
			((*m_dataDir)&(1<<(5*2)))?'I':'O',
			((*m_dataDir)&(1<<(6*2)))?'I':'O',
			((*m_dataDir)&(1<<(7*2)))?'I':'O',
			((*m_dataDir)&(1<<(8*2)))?'I':'O',
			GBA_REG_TM1D,
			GBA_REG_TM3D
			);
		pin=0;
		sprintf(m_lineMask[0],   "00000%c%c%c%c%c%c%c%c0000%c0%c%c%c%c0%c0%c%c%c%c",
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			BFGET(GBA_REG_TM0CNT,enableTimer)?'1':'0',
			BFGET(GBA_REG_TM0CNT,intOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM0CNT,incOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM0CNT,clockSpan)&0x2?'1':'0',
			BFGET(GBA_REG_TM0CNT,clockSpan)&0x1?'1':'0',
			BFGET(GBA_REG_TM2CNT,enableTimer)?'1':'0',
			BFGET(GBA_REG_TM2CNT,intOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM2CNT,incOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM2CNT,clockSpan)&0x2?'1':'0',
			BFGET(GBA_REG_TM2CNT,clockSpan)&0x1?'1':'0'
			);
		sprintf(m_lineMask[1],   "00000%c%c%c%c%c%c%c%c0000%c0%c%c%c%c0%c0%c%c%c%c",
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			(*m_data)&1<<((pin++)*2+1)?'1':'0',
			BFGET(GBA_REG_TM1CNT,enableTimer)?'1':'0',
			BFGET(GBA_REG_TM1CNT,intOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM1CNT,incOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM1CNT,clockSpan)&0x2?'1':'0',
			BFGET(GBA_REG_TM1CNT,clockSpan)&0x1?'1':'0',
			BFGET(GBA_REG_TM3CNT,enableTimer)?'1':'0',
			BFGET(GBA_REG_TM3CNT,intOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM3CNT,incOnOverflow)?'1':'0',
			BFGET(GBA_REG_TM3CNT,clockSpan)&0x2?'1':'0',
			BFGET(GBA_REG_TM3CNT,clockSpan)&0x1?'1':'0'
			);
	}
	else if(m_currentState == BTN_STATE)
	{
		static CBtnState btns;
		btns.JustUpdateKeys();
		strcpy(m_lineContent[0], " START  SELECT  A  B  L  R        ");
		strcpy(m_lineContent[1], " < /\\ \\/ >                      ");
		//TODO: Figure out how to get button states.
		//      Then make the masks reflect that state.

		//Graphics Testing area. Use this if its ok until stuff works and I make a better place.


		sprintf(m_lineMask[0],   "0%05d00%06d00%1d00%1d00%1d00%1d00000000",
			btns.KeyDown(CBtnState::START_BUTTON)?		11111	:	0,
			btns.KeyDown(CBtnState::SELECT_BUTTON) ?	111111	:	0,
			btns.KeyDown(CBtnState::A_BUTTON) ?			1		:	0,
			btns.KeyDown(CBtnState::B_BUTTON) ?			1		:	0,
			btns.KeyDown(CBtnState::L_BUTTON) ?			1		:	0,
			btns.KeyDown(CBtnState::R_BUTTON) ?			1		:	0
			);
		sprintf(m_lineMask[1],    "0%1d0%02d0%02d0%1d0000000000000000000000",
			btns.KeyDown(CBtnState::LEFT_BUTTON)?		1		:	0,
			btns.KeyDown(CBtnState::UP_BUTTON)?			11		:	0,
			btns.KeyDown(CBtnState::DOWN_BUTTON)?		11		:	0,
			btns.KeyDown(CBtnState::RIGHT_BUTTON)?		1		:	0
			);
	}
	else if(m_currentState == SENSOR_STATE)
	{
		strcpy(m_lineContent[0], "A 0A 1A 2A 3A 4A 5A 6A 7 D89AB  ");
		sprintf(m_lineContent[1], "%3d%3d%3d%3d%3d%3d%3d%3d  CDEF  ",
#ifndef NO_MOTOR			
			robot.GetScaledAnalog(0),
			robot.GetScaledAnalog(1),
			robot.GetScaledAnalog(2),
			robot.GetScaledAnalog(3),
			robot.GetScaledAnalog(4),
			robot.GetScaledAnalog(5),
			robot.GetScaledAnalog(6),
			robot.GetScaledAnalog(7)
#else
			0,0,0,0,0,0,0,0
#endif
			);
		sprintf(m_lineMask[0],    "00011100011100011100011100%c%c%c%c00",
#ifndef NO_GPIO
			robot.GetDigital(0) ? '1' : '0',
			robot.GetDigital(2) ? '1' : '0',
			robot.GetDigital(4) ? '1' : '0',
			robot.GetDigital(6) ? '1' : '0'
#else
			'0', '0', '0', '0'
#endif
			);
		sprintf(m_lineMask[1],    "00011100011100011100011100%c%c%c%c00",
#ifndef NO_GPIO
			robot.GetDigital(8) ? '1' : '0',
			robot.GetDigital(10) ? '1' : '0',
			robot.GetDigital(12) ? '1' : '0',
			robot.GetDigital(14) ? '1' : '0'
#else
			'0', '0', '0', '0'
#endif
			);
	}
	else if(m_currentState == MOTOR_STATE)
	{
		sprintf(m_lineContent[0], "M0%4dP%8ldM1%4dP%8ld  ", 
#ifndef NO_MOTOR
			//robot.GetPWM(0) != 0 ? (robot.GetPWM(0) > 0 ? ((robot.GetPWM(0) * 100) >> 8)+1 : -(((-robot.GetPWM(0)) * 100) >> 8)+1) : 0,
			robot.GetPWM(0) == 0 ? 0 : ((robot.GetPWM(0) * 100)/255),
			robot.IcGetPosition(0),
			robot.GetPWM(1) == 0 ? 0 : ((robot.GetPWM(1) * 100)/255),
			robot.IcGetPosition(1)
#else
			0, 0, 0, 0
#endif
			);
			sprintf(m_lineContent[1], "M2%4dP%8ldM3%4dP%8ld  ", 
#ifndef NO_MOTOR
			robot.GetPWM(2) == 0 ? 0 : ((robot.GetPWM(2) * 100)/255),
			robot.IcGetPosition(2),
			robot.GetPWM(3) == 0 ? 0 : ((robot.GetPWM(3) * 100)/255),
			robot.IcGetPosition(3)
#else
			0, 0, 0, 0
#endif
			);
		strcpy(m_lineMask[0],    "11000010000000011000010000000000");
		strcpy(m_lineMask[1],    "11000010000000011000010000000000");
	}
	else if(m_currentState == OTHER_STATE)
	{
		unsigned long long time;
		m_timer.GetCount(&time);
#ifndef NO_MOTOR
		strcpy(m_lineContent[0], "POWER  TIME       S 0S 1S 2S 3  ");
		sprintf(m_lineContent[1],"%3.2fV  %4d       %3d%3d%3d%3d  ", robot.GetBatteryVoltage(),
			(unsigned int)((unsigned long long)time/(unsigned long long)1000000),
			robot.GetServoPosition(3),
			robot.GetServoPosition(2),
			robot.GetServoPosition(1),
			robot.GetServoPosition(0)
			);
#else
		strcpy(m_lineContent[0], "POWER  TIME                     ");
		sprintf(m_lineContent[1],"noneV  %4d                     ",
			(unsigned int)((unsigned long long)time/(unsigned long long)1000000));
#endif
		strcpy(m_lineMask[0],    "11111001111000000011100011100000");
		strcpy(m_lineMask[1],    "00000000000000000011100011100000");
	}
	if(m_currentState!=INFO_STATE)
	{
		
		for(unsigned short i = 0; i < 30; ++i)
		{
			*(unsigned short*)(0x6000000+(STATUS_BLOCK*2048)+(i*2)) = CDispFont::AsciiToTileIndex(m_lineContent[0][i], m_lineMask[0][i] != '0');
			*(unsigned short*)(0x6000000+(STATUS_BLOCK*2048)+(i*2)+64) = CDispFont::AsciiToTileIndex(m_lineContent[1][i], m_lineMask[1][i] != '0');
		}
		
	}
	m_priorState = m_currentState;

}

void CStatusBar::IncreaseStatusBarState()
{
	++m_currentState;
	if(m_currentState == STATE_COUNT) m_currentState = 0;
}

void CStatusBar::DecreaseStatusBarState()
{
	if(m_currentState == 0) 
		m_currentState = STATE_COUNT-1;
	else 
		m_currentState--;
}

void CStatusBar::InfoPrint(char *message)
{
	if(m_currentState == INFO_STATE) {
		m_infoScreenBuf->Clear();
		m_infoScreenBuf->Print(message);
	}
	else {
		m_infoBuf->Clear();
		m_infoBuf->Print(message);
	}
}

void CStatusBar::InfoPrintf(char *format, ...)
{
	char buf[128];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	InfoPrint(buf);
}

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 8
//    c-basic-offset: 8
//   End:
