#ifndef ICROBOTCONTEXT_H
#define ICROBOTCONTEXT_H

#include "ICRobot.h"
#include "menucontext.h"
#include "simptimer.h"

// This context holds onto a reference to an ICRobot object, but does
// not own the storage for the ICRobot.  It is up to the user to make
// sure that the context is freed before the robot and that the robot
// is eventually freed.
class ICRobotContext : public CMenuContext
{
public:
  ICRobotContext(ICRobot &robot):
    CMenuContext(robot.m_menuSystem), m_robot(robot) {}
  virtual ~ICRobotContext() {}

  // Override Focus to add background
  virtual bool Focus();
  // Override Blur to tell ICRobot not to modify display
  virtual bool Blur();

  // Do a quantum of work and return
  virtual bool Process();

  static bool Push(ICRobot &robot); 
 				// Push robot context onto top of
				// display stack if top is not already
				// showing vision
  static bool Pop();		// Pop off top of display stack if 
				// top is currently showing vision

protected:
  ICRobot &m_robot;

  CSimpTimer m_timer;
  long long unsigned lastUpdateTime;
  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_icrContextIgnoreBtns;
};

#endif 
