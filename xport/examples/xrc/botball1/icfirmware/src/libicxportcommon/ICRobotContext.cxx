#include <gba.h>
#include <regbits.h>
#include "ICRobotContext.h"
#include "ICRobotPBufContext.h"
#include "btnstate.h"

bool 
ICRobotContext::Focus()
{
	// Setup display registers for displaying the robot stuff This
	// calls SetupDisplay on the menu, so we don't have to call
	// that here explicitly
	m_robot.SetupDisplay();
	
	// Print navigation instructions in status bar info page
	m_robot.GetStatusBar().InfoPrint("Up/Down: Move menu highlight\nA: Select item, B: Go back");

	// Ignore whatever buttons are currently being pressed and
	// make sure that the CMenuContext level will also ignore them
	m_icrContextIgnoreBtns=true;
	m_menuContextIgnoreBtns=true;
	return(true);
}

bool 
ICRobotContext::Blur()
{
  //  printf("ICRobotContext::Blur()\n");
	// Tell ICRobot not to change the display anymore
	m_robot.TeardownDisplay();
	return(CMenuContext::Blur());
}

bool 
ICRobotContext::Process() 
{
	static CBtnState btnState;

	if(m_icrContextIgnoreBtns) {
	  btnState.PollKeys();
	  m_icrContextIgnoreBtns=false;
	}
	btnState.PollKeys();

	if(btnState.KeyHit(CBtnState::SELECT_BUTTON)
#ifndef NORMBUTTONS
				&& btnState.KeyDown(CBtnState::L_BUTTON)
				//L_BUTTON added so that you could use the select button within the program
#endif
				){
//		if(m_robot.IsXViewOn())
//			m_robot.SetXViewOff();

		ICRobotPBufContext::Push(*g_printBuffer, m_robot);
	}
	else if(btnState.KeyHit(CBtnState::R_BUTTON)) {
		m_robot.ChangeStatusBarPage(1);
	}
	else if(btnState.KeyHit(CBtnState::L_BUTTON)) {
		m_robot.ChangeStatusBarPage(-1);
	}
	else if(btnState.KeyHit(CBtnState::START_BUTTON)
#ifndef NORMBUTTONS
				&& btnState.KeyDown(CBtnState::L_BUTTON)) {
#endif
				//L_BUTTON added so that you could use the start button within the program
		m_robot.ToggleICProgramState();
	} 
	else {
		CMenuContext::Process();
	}
	long long unsigned currentTime;
	m_timer.GetCount(&currentTime);
	if(currentTime - lastUpdateTime > 200000)
	{
		m_robot.UpdateStatusBar();
		lastUpdateTime = currentTime;
	}

	return(true);
}

bool 
ICRobotContext::Push(ICRobot &robot)
{
	// Check if top of stack is already ICRobotContext
	if(dynamic_cast<ICRobotContext*>(DispContextStackSingleton.GetCurrentContext())) {
		// Top is already ICRobotContext, don't do anything
		return(false);
	}
	// Create a robot context and push it onto the stack
	ICRobotContext *context = new ICRobotContext(robot);
	DispContextStackSingleton.PushContext(context);
	return(true);
}

bool 
ICRobotContext::Pop()
{
	// Check if top of stack is already ICRobotContext
	if(dynamic_cast<ICRobotContext*>(DispContextStackSingleton.GetCurrentContext())) {
		// Top is vision context, pop it
		DispContextStackSingleton.PopContext();
		return(true);
	}
	else {
		// Top is not vision context, do nothing
		return(false);
	}
}

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 8
//    c-basic-offset: 8
//   End:
