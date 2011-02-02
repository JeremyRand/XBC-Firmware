#include "textdisp.h"
#include "CPrintBuffer.h"
#include "CInput.h"
#include "CInterruptContSingleton.h"
#include "CTimer.h"
#include "gba.h"
#include "CDispContext.h"

#include "AnneRobot.h"

int main(void)
{
	//Make sure we create all of our global singletons
	//Don't have to worry about storing them since they're
	//kept within their own class

	//Unfortunatly, construction order matters here, since we need
	//the Interrupt Container to be constructed before the Timer 
	//manager. 
	new CTextDisp(TDM_CPORT);
	new CInterruptContSingleton();
	new CInput();
	new CTimer();

	// Initialize the DispContextStack Singleton
	new CDispContextStack();

	// TODO: learn how to use CInterruptContSingleton
	CInterruptCont intCont;
	AnneRobot robot(&intCont);

	robot.RunRobotLoop();
	return 0;
}
