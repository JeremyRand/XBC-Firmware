#include <btnstate.h>
#include <dispcontext.h>
#include "CPrintBufContext.h"

bool 
CPrintBufContext::Focus()
{
  CTextContext::Focus();
  m_pbuf.CurrPrintBufToScreen();

  // Ignore whatever buttons are currently being pressed
  m_pbufContextIgnoreBtns=true;
  return(true);
}

bool 
CPrintBufContext::Blur()
{
  m_pbuf.ScreenToCurrPrintBuf();
  CTextContext::Blur();
  return(true);
}

bool 
CPrintBufContext::Process() 
{
  static CBtnState btnState;

  if(m_pbufContextIgnoreBtns) {
    btnState.PollKeys();
    m_pbufContextIgnoreBtns=false;
  }
  btnState.PollKeys();

  if(btnState.KeyHit(CBtnState::LEFT_BUTTON)) {
    m_pbuf.FlipPrintBuffers(-1);
  }
  else if(btnState.KeyHit(CBtnState::RIGHT_BUTTON)) {
    m_pbuf.FlipPrintBuffers(1);
  } 
  else {
    return(CTextContext::Process());
  }
  return(true);
}

bool 
CPrintBufContext::Push(CPrintBuffer &pbuf)
{
  // Check if top of stack is already CPrintBufContext
  if(dynamic_cast<CPrintBufContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is already CPrintBufContext, don't do anything
    return(false);
  }
  // Create a print buffer context and push it onto the stack
  CPrintBufContext *context = new CPrintBufContext(pbuf);
  DispContextStackSingleton.PushContext(context);
  return(true);
}

bool 
CPrintBufContext::Pop()
{
  // Check if top of stack is already CPrintBufContext
  if(dynamic_cast<CPrintBufContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is print buffer context, pop it
    DispContextStackSingleton.PopContext();
    return(true);
  }
  else {
    // Top is not print buffer context, do nothing
    return(false);
  }
}
