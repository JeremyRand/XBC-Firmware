#ifndef MARTE_COMPONENT_H
#define MARTE_COMPONENT_H
#include "axesclosedquad.h"
#include "MotorPositions.h"

class CMarteComponent
{
protected:
  CMarteComponent(CAxesClosedQuad* ac) : m_ac(ac) {}
  virtual ~CMarteComponent() {}

  CAxesClosedQuad* m_ac;
public:
  virtual void RunTask(int taskNumber) {}
};
#endif
