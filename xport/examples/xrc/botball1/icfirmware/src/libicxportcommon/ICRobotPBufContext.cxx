#include <btnstate.h>
#include <dispcontext.h>
#include "ICRobotPBufContext.h"
#include "ICRobot.h"


bool 
ICRobotPBufContext::Focus()
{
  // Setup display registers for displaying the robot stuff
  m_robot.SetupDisplay();

  // Do CPrintBufContext::Focus to setup display
  CPrintBufContext::Focus();
	
  // Print navigation/status instructions in status bar info page
  UpdateStatusMsg();

  // Set IcOwnsBtns in ICRobot if showing the IC console, meaning page
  // 1
  m_robot.SetIcOwnsBtns(m_pbuf.GetCurrentIndex() == 1);

  // Ignore whatever buttons are currently being pressed
  m_icpbufContextIgnoreBtns=true;

  return(true);
}

bool 
ICRobotPBufContext::Blur()
{
  // Tell ICRobot not to change the display anymore
  m_robot.TeardownDisplay();

  CPrintBufContext::Blur();

  // Clear IcOwnsBtns
  m_robot.SetIcOwnsBtns(false);

  return(true);
}

bool 
ICRobotPBufContext::Process() 
{
  static CBtnState btnState;
  bool ret=true;

  if(m_icpbufContextIgnoreBtns) {
    btnState.PollKeys();
    m_icpbufContextIgnoreBtns=false;
  }
  btnState.PollKeys();

  if(btnState.KeyHit(CBtnState::R_BUTTON)) {
    m_robot.ChangeStatusBarPage(1);
  }
  else if(btnState.KeyHit(CBtnState::L_BUTTON)) {
    m_robot.ChangeStatusBarPage(-1);
  }
  else if(btnState.KeyHit(CBtnState::SELECT_BUTTON)) {
    // If showing IC Console, select by default pops the context and
    // you end up back in the menu.  If you hold down A select flips
    // the buffers regardless of which page is showing.  If flipping
    // to IC Console set IcOwnsBtns in ICRobot
    if(m_pbuf.GetCurrentIndex() != CPrintBuffer::BUF_IC ||
       (m_pbuf.GetCurrentIndex() == CPrintBuffer::BUF_IC &&
	btnState.KeyHeld(CBtnState::A_BUTTON))) {
      m_pbuf.FlipPrintBuffers();
      m_robot.SetIcOwnsBtns(m_pbuf.GetCurrentIndex() == 1);
      UpdateStatusMsg();
    }
    else {
      // Showing IC console witout A held, pop context
      return(false);
    }
  } 
  else if(btnState.KeyHit(CBtnState::START_BUTTON)) {
    m_robot.ToggleICProgramState();
  } 
  else if(btnState.KeyHit(CBtnState::UP_BUTTON) &&
                        m_robot.GetCodeState() != ICRobot::IC_CODE_RUNNING) {
          g_printBuffer->Scroll(-1);
  }
  else if(btnState.KeyHit(CBtnState::DOWN_BUTTON) &&
                  m_robot.GetCodeState() != ICRobot::IC_CODE_RUNNING) {
          g_printBuffer->Scroll(1);
  }
  else if(m_pbuf.GetCurrentIndex() != 1) {
    // When in IC mode don't do scrolling or exiting
    ret = CTextContext::Process();
  }
  long long unsigned currentTime;
  m_timer.GetCount(&currentTime);
  if(currentTime - lastUpdateTime > 200000)
  {
	  m_robot.UpdateStatusBar();
	  lastUpdateTime = currentTime;
  }
  return(ret);
}

void
ICRobotPBufContext::UpdateStatusMsg()
{
  int currPage = m_pbuf.GetCurrentIndex();
  CStatusBar &statusBar = m_robot.GetStatusBar();

  if(currPage == 0) {
    statusBar.InfoPrint("Firmware Msgs, B: Exit to Menu\nSelect: Change buffers");
  }
  else if(currPage == 1) {
    statusBar.InfoPrint("IC Console\nSelect: Exit to Menu");
  }
  else if(currPage == 2) {
    statusBar.InfoPrint("System Msgs, B: Exit to Menu\nSelect: Change buffers");
  }
}

bool 
ICRobotPBufContext::Push(CPrintBuffer &pbuf, ICRobot &robot)
{
  // Check if top of stack is already ICRobotPBufContext
  if(dynamic_cast<ICRobotPBufContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is already ICRobotPBufContext, don't do anything
    return(false);
  }
  // Create a print buffer context and push it onto the stack
  ICRobotPBufContext *context = new ICRobotPBufContext(pbuf, robot);
  DispContextStackSingleton.PushContext(context);
  return(true);
}

bool 
ICRobotPBufContext::Pop()
{
  // Check if top of stack is already ICRobotPBufContext
  if(dynamic_cast<ICRobotPBufContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is print buffer context, pop it
    DispContextStackSingleton.PopContext();
    return(true);
  }
  else {
    // Top is not print buffer context, do nothing
    return(false);
  }
}
