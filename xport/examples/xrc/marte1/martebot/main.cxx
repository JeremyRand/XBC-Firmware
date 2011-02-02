#include "MarteBot.h"
#include "textdisp.h"
#include "intcont.h"
#include "axesclosedquad.h"
#include "TaskDefs.h"

CTextDisp td(TDM_LCD_AND_CPORT);


int main()
{
  CInterruptCont intcont;
  CMarteBot bot(&intcont);
  bot.RunRobotLoop();
  return 0;
}

 /*
int main()
{
  CInterruptCont intcont;
  CAxesClosedQuad m_axes(&intcont, 300);
  m_axes.SetPIDGains(500, 0, 0);

  printf("Moving first\n");
  m_axes.Move(MOTOR_CORE_CLAMP, -1000000, 10000, 10000);
  m_axes.Execute();
  for(int i = 0; i < 1000000; ++i); //Very short wait
  printf("Moving second\n");
  m_axes.Move(MOTOR_RAIL_HIGH_SPEED, 1000000, 10000, 10000);
  m_axes.Execute();
  while(1);
  return 0;
}


int main()
{
  CInterruptCont intcont;
  CAxesClosedQuad m_axes(&intcont, 300);

   m_axes.SetPIDGains(500, 0, 0);
   printf("Moving first\n");
  m_axes.Move(MOTOR_CORE_CLAMP, -1000000, 10000, 10000);
  printf("Moving second\n");
  m_axes.Move(MOTOR_RAIL_HIGH_SPEED, 1000000, 10000, 10000);
  m_axes.Execute();
  while(1);
  return 0;
}
*/
