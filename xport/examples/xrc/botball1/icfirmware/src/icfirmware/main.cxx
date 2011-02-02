#include <stdlib.h>
#include "textdisp.h"
#include "CPrintBuffer.h"
#include "CInterruptContSingleton.h"
//#include "CTimer.h"// TODO
#include "btnstate.h"
#include "gba.h"
#include "menudisp.h"
#include "btcomm.h"
#include "../../../logic/botball1.h"

#ifdef icxbc
#define ROBOT_TYPE XBCRobot
#define ROBOT_TYPESTR "XBCRobot"
#include "XBCRobot.h"
#else
#define ROBOT_TYPE XRCRobot
#define ROBOT_TYPESTR "XRCRobot"
#include "XRCRobot.h"
#endif

CTextDisp td(TDM_LCD_AND_CPORT);

#ifndef NO_VISION
#include "vision.h"
#include "visionmenu.h"
#endif
#define NO_BLUETOOTH

//#define DEBUG

#ifdef DEBUG
#include "breadcrumbs.h"
#define DBG() ADD_CRUMB(0xa190)
#define DBG2(x) ADD_CRUMB(0xa190); \
                ADD_DEBUG_CRUMB(0xa192, x)
#define DBG4(x) ADD_CRUMB(0xa190); \
                ADD_DEBUG_CRUMB(0xa194, x>>16); \
                ADD_DEBUG_CRUMB(0xa194, x&0xffff)
#else
#define DBG() 
#define DBG2(x) 
#define DBG4(x) 
#endif

int main(void)
{
#ifdef DEBUG
  init_crumbs(200, true);
#endif

	//Make sure we create all of our global singletons
	//Don't have to worry about storing them since they're
	//kept within their own class
        printf("Initializing IC Firmware\n");

	//Unfortunatly, construction order matters here, since we need
	//the Interrupt Container to be constructed before the Timer 
	//manager. 
	CInterruptCont &intCont = CInterruptContSingleton::Instance();

	intCont.MaskAll();
	printf("  Creating %s\n", ROBOT_TYPESTR);
	ROBOT_TYPE robot;

	// TODO: Check if have bluetooth, if so add menu here

#ifndef NO_VISION
	printf("  Creating vision\n");
	CVision m_vision(InterruptContSingleton.InstancePtr());

	// Give vision object to robot
	robot.SetVision(&m_vision);

	// Setup display options
	CVisionDispOptions options;
 	options.minBlobArea=100;

	// set interleave to 3 frames to save CPU
	m_vision.SetRenderInterleave(3);

	//robot.HappyBeep();

 	printf("Setting up vision menu...\n");
	CVisionMenuHandler vmenu(m_vision, options);
	CMenuDisp &menu = robot.GetMenuSystem();
	CMenuElement *topElt = menu.GetCurrentElement();
	vmenu.PopulateMenu(*topElt);

	//robot.SadBeep();

	// Give vision menu object to robot
	robot.SetVisionMenu(&vmenu);
#endif

	
#ifndef NO_BLUETOOTH
	printf("Detecting Bluetooth...");
	UART_SET_BT();
	CBluetoothComm* bttest = new CBluetoothComm(UART_BASE);
	if(bttest->Reset() >= 0)
		menu.AddMenuItem("Bluetooth Connect", 21);
	UART_SET_RS232();
#endif


#ifdef DEBUG
	dump_crumbs();
#endif
	intCont.UnmaskAll();

	printf("  Running Robot Loop\n");
	
	// Debug so we can see messages.
	//while(1);
	
	robot.RunRobotLoop();
	return 0;
}
