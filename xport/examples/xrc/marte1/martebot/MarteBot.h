#ifndef MARTEBOT_H
#define MARTEBOT_H
#include "adc.h"
#include "axesclosedquad.h"
#include "uartserial.h"
#include "TaskDefs.h"
#include "TaskList.h"


struct CMotorPosition
{
  int m_motorPosition;
  int m_motorVelocity;
  int m_motorGoalVelocity;
};

class CMarteBot : public CAxesClosedQuad
{
  CAdc m_adc;
  CUartSerial m_serial;
  unsigned char m_boardIndex;
  char m_currentCommand[2];

  int HoldPosition();
  int SetPosition();
  int MoveToPosition();

  int GetDigital(int port);

  int PlaceLiner();
  int AcceptCore();
  int TightenClamp();
  int FaceCore();
  int MoveClampToPosition();
  int StoreCoreAtPosition();
  int RetrieveCoreFromPosition();
  int MoveToSubsamplePosition();
  int GraspSample();
  int TransferSample();
  int DiscardTube();
  int CancelTask();
  int ResetActuators();
  int ReleaseClamp();
  int EmptyClamp();
  int CutSubsample();
  int CrushSubsample();
  int StopAll();

  CTaskList m_taskList;

public:
  CMarteBot(CInterruptCont* intcont);
  virtual ~CMarteBot();
  void RunRobotLoop();
  void RunTask();
  int SendStatusPackage();
  int SendTaskStatusPackage();

};
#endif
