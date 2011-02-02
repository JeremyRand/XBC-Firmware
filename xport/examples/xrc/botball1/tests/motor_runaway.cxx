#include <textdisp.h>
#include <axesc.h>
#include <mathutils.h>
#include "btnstate.h"

using namespace gba;

CTextDisp td(TDM_LCD_AND_CPORT);

#define IC_BEMF_DIV 20
#define DEFAULT_MOTION_ACCEL 100000/IC_BEMF_DIV

class ICRobot : public CAxesClosed {
public:
  ICRobot(CInterruptCont *pIntCont) : 
    CAxesClosed(pIntCont, DAO_BEMF_BASE, 20) {}
  ~ICRobot() {}

  void 
  IcMove(unsigned char axis,
	 AxisPosition endPosition, int velocity, 
	 unsigned int acceleration)
  {
    // Sanity check sign on velocity so that it actually goes in
    // the appropriate direction to the goal regardless of the
    // sign passed in for velocity.  Unfortunately there is a race
    // condition.  If the motor is currently moving forward and
    // you request a move to a position a little in front of where
    // it is when you check, but it has already gotten past that
    // point when the CAxesClosed::Move executes then you'll end
    // up with the motor forever forward and never reaching it's
    // goal.
    AxisPosition realEndPos = endPosition;
    AxisPosition currPos = IcGetPosition(axis);
    if(sgn(realEndPos-currPos)!=sgn(velocity)) {
      velocity *= -1;
    }

	// need to sync trajectory -- completely necessary, but can prevent jolts when move starts
	SyncTrajectory(axis);
    printf("Axis %d: Move to pos %d, vel %d, accel %d, start pos %d\n",
	   axis, realEndPos, velocity, acceleration, currPos);
    Move(axis, realEndPos*IC_BEMF_DIV, velocity*IC_BEMF_DIV, 
	 acceleration*IC_BEMF_DIV);
  }
  AxisPosition 
  IcGetPosition(unsigned char axis)
  {
    int opos = CAxesOpen::GetPosition(axis)/IC_BEMF_DIV;
    int cpos = CAxesClosed::GetPosition(axis)/IC_BEMF_DIV;
    printf("Axis %d: opos=%d, cpos=%d, done=%s\n", 
	   axis, opos, cpos, Done(axis)?"true":"false");
    return(opos);
  }
  void         
  IcSetPosition(unsigned char axis, AxisPosition pos)
  {
    // Cancel any trajectory currently in progress, including a
    // freeze command.  Otherwise changing the position will cause
    // the motor to move, possiblly violently.
    if(!Done(axis)) {
      Stop(axis);
      Execute();
    }
    SetPosition(axis, 
		pos*IC_BEMF_DIV);
    SyncTrajectory(axis);
  }

  void PrintPosUntilBtn(unsigned char axis) {
    static CBtnState btnState;
    btnState.PollKeys();

    while(!btnState.KeyHit(CBtnState::B_BUTTON)) {
      IcGetPosition(axis);
      btnState.PollKeys();
    }
    printf("Done\n");
  }
};

int main(void)
	{
	CInterruptCont intCont;

	// create closed-loop motor controller, pass in interrupt controller
	ICRobot icr(&intCont);

	// set gains
	icr.SetPIDGains(600, 0, 750);

	// Work around initial value corruption
	// set_motor_position_counter(0, 0L)
	icr.IcSetPosition(0, 0L);

	icr.PrintPosUntilBtn(0);

	printf("move_to_position(0, 1000L, 1000L)\n");
	printf("  Goes there properly\n");

	icr.IcMove(0, 1000L, 1000L, DEFAULT_MOTION_ACCEL);
	icr.Execute();
	
	icr.PrintPosUntilBtn(0);

	printf("set_motor_position_counter(0, 0L)\n");
	printf("   Both open and closed agree we're at 0\n");
	icr.IcSetPosition(0, 0L);

	icr.PrintPosUntilBtn(0);

	printf("move_to_position(0, 1000L, 1000L)\n");
	printf("  Does a short negative move, when it should move forward 1000\n");

	icr.IcMove(0, 1000L, 1000L, DEFAULT_MOTION_ACCEL);
	icr.Execute();
	
	icr.PrintPosUntilBtn(0);

	printf("move_to_position(0, 1000L, 0)\n");
	printf("  Moves a little bit negative, then switches to positive and goes forever\n");

	icr.IcMove(0, 0, 1000L, DEFAULT_MOTION_ACCEL);
	icr.Execute();
	
	icr.PrintPosUntilBtn(0);
	}
