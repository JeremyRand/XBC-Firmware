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

#include "textdisp.h"
#include "xport.h"
#include "intcont.h"
#include "rpcargs.h"
#include "uartserial.h"
#include "flashxfer.h"
#include "btnstate.h"
#include "vision.h"
#include "gba_blob_image.h"
#include "visioncontext.h"
#include "textcontext.h"
#include "blobcontext.h"
#include "axesc.h"
#include "gpioint.h"
#include "rcservo.h"
#include "simptimer.h"
#include "menu.h"
#include "../logic/botball1.h"

#define BOOTLOADER_VERSION			"1.2"

CTextDisp td(TDM_LCD_AND_CPORT);

int main(void)
	{
	int res, i;
	CBtnState btnState;
	CInterruptCont intCont;
	CUartSerial serial(&intCont);
	CFlashXfer xfer(&serial, &intCont);
	// servo can twitch when writing to flash because it affects PHI clock
	// enabling servos minimizes twitching
	CRCServo servo(4, RCSERVO_BASE, true, XP_REG_IDENTIFIER==BITSTREAM_XBC2005?1:2);
	CRpcArgs args;
	void (*start)() = (void (*)())0x8020000;

	serial.SetBaud(DUC_BAUD_38K);

	btnState.PollKeys();
	if (!btnState.KeyHeld(CBtnState::R_BUTTON | CBtnState::L_BUTTON))
		{
		printf("Charmed Labs and KIPR\n");
		printf("XBC Bootloader v" BOOTLOADER_VERSION "\n\n");

		// wait for 1000 milliseconds
		res = xfer.Dispatch(&args, 1000);
	
		if (xfer.GetMode()==EXM_NONE)
			{
			printf("Running at 0x8020000...\n");
			(*start)();
			}
		else
			{
			// putting a print message here can sometimes confuse things because the dispatch
			// call above often handles all RPC dispatches for a given upload causing the message
			// to be printed late
			while(1)
				xfer.Dispatch(&args);
			}		
		}
	else
		{
		/* factory testing */
		unsigned short logicVersion = XP_REG_IDENTIFIER&0xff;
		bool cameraConnected;

		printf("Factory test, logic ver. %d\n", logicVersion);

		if ((XP_REG_IDENTIFIER>>8)!=BITSTREAM_IDENTIFIER)
			{
			printf("Incorrect logic configuration.\n");
			while(1);
			}

		printf("Initializing...\n");

		CVision vision(&intCont, DV_VISION_BASE, 17, 18, 19, XP_REG_IDENTIFIER==BITSTREAM_XBC2005?1:2);
		CGpioInt gpio(&intCont);
		CAxesClosed ac(&intCont, BEMF4_BASE, BEMF4_VECTOR, DAO_MAX_AXES, DAO_MAX_AXES, 8, 8, 
			XP_REG_IDENTIFIER==BITSTREAM_XBC2005?1:2);
		CSimpTimer timer;
		unsigned long count;
		volatile unsigned long d;

		cameraConnected = vision.m_camera.Connected();

		if (!cameraConnected)
		printf("Camera not connected.\n");

		// set rc servo limits
		servo.SetBounds(0, 32, 228);
		servo.SetBounds(1, 32, 228);
		servo.SetBounds(2, 32, 228);
		servo.SetBounds(3, 32, 228);
		
		// set gains
		ac.SetPIDGains(500, 0, 650);
		ac.Hold(0, true);
		ac.Hold(1, true);
		ac.Hold(2, true);
		ac.Hold(3, true);
		ac.Execute();
		
		if (cameraConnected)
			{
			unsigned char r[3] = {31, 31, 0}; // red
			unsigned char g[3] = {0, 31, 31}; // yellow
			unsigned char b[3] = {0, 0, 0};	// green
			
			vision.UploadModel(0, 0, 15, 150, 224, 100, 224); // red 
			//		vision.UploadModel(1, 50, 70, 150, 224, 100, 224); // yellow
			//		vision.UploadModel(2, 100, 160, 100, 224, 75, 224); // green
			
			printf("Initializing Render Params...\n");
			for (i=0; i<DV_MODELS; i++) {
				vision.SetRenderColor(i, 
					DV_BUILD_COLOR(r[i], g[i], b[i]));
				}
			
			// set interleave to 3 frames to save CPU (for example)
			vision.SetRenderInterleave(3);
			
			// Setup display options
			CVisionDispOptions options;
			options.mode = RM_RAW;
			options.showBlobs=true;
			options.showBlobText=true;
			options.minBlobArea=100;
			
			printf("Setting up blob context...\n");
			
			printf("Showing blob context...\n");
			CBlobContext::Push(&intCont, vision, options);
			}

		unsigned short servoPosition = 0;
		unsigned short position;
		unsigned char error;
		char c;
		
		enum 
			{ 
			TEST_BLOB,
			TEST_BEMF,
			TEST_RCSERVO,
			TEST_ANALOG,
			TEST_BATTERY_VOLTAGE,
			TEST_GPIO,
			TEST_SERIAL,
			TEST_CHARGER
			}
		testState = TEST_BLOB;
		
		while(1) 
			{
			// toggle LEDs
			timer.GetCount(&count);
			count >>= 17;
			if (count&1)
				XP_REG_LED = XP_LED_GREEN;
			else
				XP_REG_LED = XP_LED_RED;
			
			btnState.PollKeys();
			if(btnState.KeyHit(CBtnState::R_BUTTON) && testState<TEST_CHARGER)
				{
				((int)testState)++;
				
				CTextContext::Push();
				ptd->Clear();
				}
			else if(btnState.KeyHit(CBtnState::L_BUTTON) && testState>TEST_BLOB)
				{
				((int)testState)--;
				ptd->Clear();
				
				if (testState==TEST_BLOB)
					CTextContext::Pop();
				}
			
			switch(testState)
				{
				case TEST_BLOB:
					if (cameraConnected)
						DispContextStackSingleton.Process();
					break;

				case TEST_BEMF:
					CMenu::Printxyf(10, 1, 0, false, "Motor Test");
					CMenu::Printxyf(4, 5, 0, false, "Wheel 0");
					CMenu::Printxyf(18, 5, 0, false, "Wheel 1");
					CMenu::Printxyf(4, 8, 0, false, "Wheel 2");
					CMenu::Printxyf(18, 8, 0, false, "Wheel 3");	
					
					CMenu::Printxyf(4, 6, 8, false, "%d", ac.GetPosition(0));
					CMenu::Printxyf(18, 6, 8, false, "%d", ac.GetPosition(1));
					CMenu::Printxyf(4, 9, 8, false, "%d", ac.GetPosition(2));
					CMenu::Printxyf(18, 9, 8, false, "%d", ac.GetPosition(3));
					break;
					
				case TEST_RCSERVO:
					servoPosition++;
					if (servoPosition==0x200)
						servoPosition = 0;
					if (servoPosition&0x100)
						position = 0x100 - (servoPosition&0xff);
					else
						position = servoPosition&0xff;
					
					servo.SetPosition(0, position);
					servo.SetPosition(1, position);
					servo.SetPosition(2, position);
					servo.SetPosition(3, position);
					CMenu::Printxyf(8, 1, 0, false, "RC Servo Test");
					CMenu::Printxyf(10, 5, 0, false, "Position");
					CMenu::Printxyf(12, 6, 8, false, "%d", position);
					for (d=0; d<2500; d++);
					break;
					
				case TEST_ANALOG:
					CMenu::Printxyf(9, 1, 0, false, "Analog Test");
					CMenu::Printxyf(6, 5, 3, true, " 0 ");
					CMenu::Printxyf(11, 5, 3, true, " 1 ");
					CMenu::Printxyf(16, 5, 3, true, " 2 ");
					CMenu::Printxyf(21, 5, 3, true, " 3 ");
					CMenu::Printxyf(6, 9, 3, true, " 4 ");
					CMenu::Printxyf(11, 9, 3, true, " 5 ");
					CMenu::Printxyf(16, 9, 3, true, " 6 ");
					CMenu::Printxyf(21, 9, 3, true, " 7 ");
					
					CMenu::Printxyf(6, 6, 3, false, "%03x", ac.GetAnalog(0));
					CMenu::Printxyf(11, 6, 3, false, "%03x", ac.GetAnalog(1));
					CMenu::Printxyf(16, 6, 3, false, "%03x", ac.GetAnalog(2));
					CMenu::Printxyf(21, 6, 3, false, "%03x", ac.GetAnalog(3));
					CMenu::Printxyf(6, 10, 3, false, "%03x", ac.GetAnalog(4));
					CMenu::Printxyf(11, 10, 3, false, "%03x", ac.GetAnalog(5));
					CMenu::Printxyf(16, 10, 3, false, "%03x", ac.GetAnalog(6));
					CMenu::Printxyf(21, 10, 3, false, "%03x", ac.GetAnalog(7));
					break;
					
				case TEST_BATTERY_VOLTAGE:
					CMenu::Printxyf(5, 1, 0, false, "Battery Voltage Test");
					CMenu::Printxyf(12, 6, 0, false, "%3.2fV", (float)ac.GetAnalog(7)*0.00214);
					break;
					
				case TEST_GPIO:
					error = 0;
					*gpio.m_dataDir = 0x5555;
					*gpio.m_data = 0x0001;
					if (*gpio.m_data!=0x0003)
						error |= 0x01;
					*gpio.m_data = 0x0004;
					if (*gpio.m_data!=0x000c)
						error |= 0x02;
					*gpio.m_data = 0x0010;
					if (*gpio.m_data!=0x0030)
						error |= 0x04;
					*gpio.m_data = 0x0040;
					if (*gpio.m_data!=0x00c0)
						error |= 0x08;
					*gpio.m_data = 0x0100;
					if (*gpio.m_data!=0x0300)
						error |= 0x10;
					*gpio.m_data = 0x0400;
					if (*gpio.m_data!=0x0c00)
						error |= 0x20;
					*gpio.m_data = 0x1000;
					if (*gpio.m_data!=0x3000)
						error |= 0x40;
					*gpio.m_data = 0x4000;
					if (*gpio.m_data!=0xc000)
						error |= 0x80;
					CMenu::Printxyf(6, 1, 0, false, "GPIO Loopback Test");
					CMenu::Printxyf(14, 5, 8, false, "%02x", error);
					if (error)
						CMenu::Printxyf(11, 6, 7, false, " error");
					else
						CMenu::Printxyf(11, 6, 7, false, "success");
					break;
					
				case TEST_SERIAL:
					CMenu::Printxyf(5, 1, 0, false, "Serial Loopback Test");
					c = 'A';
					serial.Send(&c, 1, 0);
					if (serial.Receive(&c, 1, 1)==1 && c=='A')
						CMenu::Printxyf(11, 6, 7, false, "success");
					else
						{
						CMenu::Printxyf(11, 6, 7, false, " error");
						for (d=0; d<50000; d++);
						}			
					break;
					
				case TEST_CHARGER:
					CMenu::Printxyf(9, 1, 0, false, "Charger Test");
					break;
				}
			}		
		}
	}
