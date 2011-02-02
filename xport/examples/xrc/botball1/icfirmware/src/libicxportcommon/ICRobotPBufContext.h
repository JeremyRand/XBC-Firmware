#ifndef ICROBOTPBUFCONTEXT_H
#define ICROBOTPBUFCONTEXT_H

#include "CPrintBufContext.h"
#include "simptimer.h"
class ICRobot;

// This context holds onto a reference to an ICRobot object, but does
// not own the storage for the ICRobot.  It is up to the user to make
// sure that the context is freed before the robot and that the robot
// is eventually freed.
class ICRobotPBufContext : public CPrintBufContext
{
public:
  ICRobotPBufContext(CPrintBuffer &pbuf, ICRobot &robot):
    CPrintBufContext(pbuf), m_robot(robot) {}
  virtual ~ICRobotPBufContext() {}

  // Override Focus to add status message
  virtual bool Focus();

  // Override Blur to clear IcOwnsBtns
  virtual bool Blur();

  // Do a quantum of work and return
  virtual bool Process();	

  // Update status display
  void UpdateStatusMsg();

  // Push printbuf context onto top of display stack if top is not
  // already showing printbuf
  static bool Push(CPrintBuffer &pbuf, ICRobot &robot); 
  // Pop off top of display stack if top is currently showing printbuf
  static bool Pop();		

protected:
  ICRobot &m_robot;

  CSimpTimer m_timer;
  long long unsigned lastUpdateTime;
  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_icpbufContextIgnoreBtns;
};

#endif 
