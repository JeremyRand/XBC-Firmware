#ifndef CPRINTBUFCONTEXT_H
#define CPRINTBUFCONTEXT_H

#include "CPrintBuffer.h"
#include "textcontext.h"

// This context holds onto a reference to an ICRobot object, but does
// not own the storage for the ICRobot.  It is up to the user to make
// sure that the context is freed before the robot and that the robot
// is eventually freed.
class CPrintBufContext : public CTextContext
{
public:
  CPrintBufContext(CPrintBuffer &pbuf):
    CTextContext(), m_pbuf(pbuf) {}
  virtual ~CPrintBufContext() {}


  // This context now on top of stack
  virtual bool Focus();
  // Newer context on top of stack, lose focus
  virtual bool Blur();
  
  // Do a quantum of work and return
  virtual bool Process();	

  // Push printbuf context onto top of display stack if top is not
  // already showing printbuf
  static bool Push(CPrintBuffer &pbuf); 
  // Pop off top of display stack if top is currently showing printbuf
  static bool Pop();		

protected:
  CPrintBuffer &m_pbuf;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_pbufContextIgnoreBtns;
};

#endif 
