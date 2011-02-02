//  BEGIN LICENSE BLOCK
// 
//  Version: MPL 1.1
// 
//  The contents of this file are subject to the Mozilla Public License Version
//  1.1 (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
// 
//  Software distributed under the License is distributed on an "AS IS" basis,
//  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
//  for the specific language governing rights and limitations under the
//  License.
// 
//  The Original Code is The Xport Software Distribution.
// 
//  The Initial Developer of the Original Code is Charmed Labs LLC.
//  Portions created by the Initial Developer are Copyright (C) 2003
//  the Initial Developer. All Rights Reserved.
// 
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
// 
//  END LICENSE BLOCK 

#include <string.h>
#include "../../libxrc/vision.h"
#include "intcont.h"
#include "textdisp.h"
#include "gba.h"
#include "visioncontext.h"
#include "visionmenu.h"
#include "menucontext.h"
#include "textcontext.h"
#include "blobcontext.h"
#include "axesc.h"
#include "gpioint.h"
#include "rcservo.h"
#include "simptimer.h"
#include "../logic/botball1.h"

CTextDisp td(TDM_CPORT);

class CDerivedGpioInt : public CGpioInt
	{
public:
	CDerivedGpioInt(CInterruptCont *pIntCont) : CGpioInt(pIntCont)
		{
		memset((void *)m_count, 0, sizeof(m_count));
		}
	volatile int m_count[16];

protected:
	virtual void Interrupt(unsigned char vector)
		{
		int i;
		unsigned short status = *m_intStatus;

		for (i=0; i<16; i++)
			{
			if (status&0x0001)
				m_count[i]++;
			status >>= 1;
			}
		}

	};

int main(void)
	{
	CInterruptCont intCont;
	CDerivedGpioInt gpio(&intCont);
	CRCServo servo(4, RCSERVO_BASE, true, XP_REG_IDENTIFIER==BITSTREAM_XBC2005?1:2);
	CSimpTimer timer;

// 	printf("Masking all interrupts\n");
// 	intCont.MaskAll();

	printf("Creating vision object...\n");
	CVision vision(&intCont, DV_VISION_BASE, 17, 18, 19, XP_REG_IDENTIFIER==BITSTREAM_XBC2005?1:2);

	// Timer test
#if 0
	unsigned long long count;

	while(1)
		{
		timer.GetCount(&count);
		printf("%d\n", (unsigned long)count);
		}
#endif

	// test analag
#if 0
	while(1)
		printf("%x %x %x %x\n", ac.GetAnalog(0), ac.GetAnalog(1), ac.GetAnalog(2), ac.GetAnalog(3));
#endif

	// GpioInt test
	// Physically loop bit 0 to bit 1
#if 0
	// reset data state
	*gpio.m_data = 0;
	// set bit 0 as output
	*gpio.m_dataDir = 0x0001;
	// set bit 1 as positive edge triggered
	*gpio.m_intEdge = 0x0002;
	// enable bit 1 interrupt
	*gpio.m_intMask = 0x0002;

	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 1;
	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 0;
	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 1;
	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 0;
	printf("%d\n", gpio.m_count[1]);

	// change edge
	*gpio.m_intEdge = 0x0000;

	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 1;
	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 0;
	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 1;
	printf("%d\n", gpio.m_count[1]);
	*gpio.m_data = 0;
	printf("%d\n", gpio.m_count[1]);
	while(1);
#endif


	// RC servo test
#if 0
	volatile unsigned long d;

	servo.SetBounds(0, 32, 228);
	servo.SetBounds(1, 32, 228);
	servo.SetBounds(2, 32, 228);
	servo.SetBounds(3, 32, 228);

	// loop test for rc servos
	while(1)
		{
		// move maximum counter-clockwise
		servo.SetPosition(0, 0x00);
		servo.SetPosition(1, 0x00);
		servo.SetPosition(2, 0x00);
		servo.SetPosition(3, 0x00);
		printf("Pos: 0x%x\n", servo.GetPosition(0));
		for (d=0; d<1000000; d++);

		// move middle
		servo.SetPosition(0, 0x80);
		servo.SetPosition(1, 0x80);
		servo.SetPosition(2, 0x80);
		servo.SetPosition(3, 0x80);
		printf("Pos: 0x%x\n", servo.GetPosition(0));
		for (d=0; d<1000000; d++);

		// move maximum clockwise
		servo.SetPosition(0, 0xff);
		servo.SetPosition(1, 0xff);
		servo.SetPosition(2, 0xff);
		servo.SetPosition(3, 0xff);
		printf("Pos: 0x%x\n", servo.GetPosition(0));
		for (d=0; d<1000000; d++);
		}

#endif

	// Test bemf controller -- enable motors, hold positions
#if 0
	printf("Creating CAxesClosed\n");
	CAxesClosed ac(&intCont, BEMF4_BASE, BEMF4_VECTOR, DAO_MAX_AXES, XP_REG_IDENTIFIER==BITSTREAM_XBC2005?1:2);

	// set gains
	ac.SetPIDGains(1600, 0, 2500);
	ac.MoveVelocity(0, 8000, 1000);
	printf("Set PWM\n");
//	ac.SetPWM(0, 255);
//	while(1);
	ac.Hold(0, true);
	ac.Hold(1, true);
	ac.Hold(2, true);
	ac.Hold(3, true);
	ac.Execute();
#endif

#if 0
	// continuous render
	// set interleave to 3 frames to save CPU (for example)
	vision.SetRenderInterleave(3);
	vision.SetRenderMode(RM_RAW);
	// rendering takes place in the background
	while(1);	
#endif

#if 0
	// single frame grab test

	int i = 0;
	volatile unsigned long d;

	vision.SetupDisplay();

	// grab single frame
	vision.SetRenderFrames(1);
	// set scaling to 1 to 1
//	vision.SetRenderScaling(0x100, 0x100);
	// move to center of lcd (roughly)
//	vision.SetRenderOffset(30, 10);
	vision.SetRenderMode(RM_RAW);
	vision.SetGamma(true);
	
	while(1)
		{
		// wait for A key
		while(GBA_REG_P1&GBA_KEY_A);
		// grab single frame
		vision.SetRenderFrames(1);
		// wait for frame grab
		while(vision.GetRenderFrames()==0);
		printf("rendered %d\n", i++);
		// cheap switch debounce
		for (d=0; d<200000; d++);
		}
#endif

	// Blob analysis test
#if 1
	printf("Verify\n\n");
	printf("Colors:    Render  Sprites\n");
	printf("Channel 0: Red     Green\n");
	printf("Channel 1: Yellow  Blue\n");
	printf("Channel 2: Green   Red\n\n");
	printf(" UP:    select all channels\n");
	printf(" LEFT:  select channel 0\n");
	printf(" DOWN:  select channel 1\n");
	printf(" RIGHT: select channel 2\n\n");
	printf(" A:     toggle video mode\n");
	printf(" R:     toggle text display\n");
	printf(" B:     exit context\n\n");


	// Model and render color loading are now done from flash by
	// visionmenu

	// set interleave to 3 frames to save CPU (for example)
	vision.SetRenderInterleave(3);

	// Setup display options
	CVisionDispOptions options;
 	options.minBlobArea=100;

 	printf("Setting up vision menu...\n");
	CVisionMenuHandler vmenu(vision, options);
	CMenuDisp menu(&vmenu);
	CMenuElement *topElt = menu.GetCurrentElement();
	vmenu.PopulateMenu(*topElt);

	CMenuContext *mContext = new CMenuContext(menu);
	DispContextStackSingleton.PushContext(mContext);

	while(1) {
	  DispContextStackSingleton.Process();
	}
#endif
	}
