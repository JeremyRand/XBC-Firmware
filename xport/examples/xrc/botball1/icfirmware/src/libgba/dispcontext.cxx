#include "dispcontext.h"
#include <unistd.h>
#include "textdisp.h"

CDispContextStack::CDispContextStack() 
{
  m_depth=0;
  for(int i=0; i<MAX_CONTEXT_DEPTH; i++) {
    m_contextStack[i]=NULL;
  }
}

CDispContextStack::~CDispContextStack() 
{
  while(m_depth>0) {
    PopContext();
  }
}

// Blur current context, push new context on top of stack and run
// setup.  CDislayContextStack takes ownership of context and will
// free it when popped
bool 
CDispContextStack::PushContext(IDispContext *context)
{
  if(context == NULL || m_depth >= MAX_CONTEXT_DEPTH) {
    return(false);
  }

  if(m_depth>0) {
    m_contextStack[m_depth-1]->Blur();
  }
  m_contextStack[m_depth++] = context;

  bool ret = context->Setup();
  return(ret);
}

// Shutdown and free current context, Focus previous context
bool 
CDispContextStack::PopContext()
{
  if(m_depth<1) {
    return false;
  }

  // Shutdown and delete current top of stack
  IDispContext *dc = m_contextStack[--m_depth];
  assert(dc!=NULL);
  dc->Shutdown();
  delete dc;

  // Call Focus on new top of stack
  if(m_depth>0) {
    m_contextStack[m_depth-1]->Focus();
  }
  return true;
}

// Do a quantum of work on top item of stack
bool 
CDispContextStack::Process()
{
  IDispContext *dc = GetCurrentContext();
  if(dc!=NULL) {
    bool ret = dc->Process();
    if(!ret) {
      ret = PopContext();
    }
    return(ret);
  }
  else {
    return(false);
  }
}

// Return current context
IDispContext *
CDispContextStack::GetCurrentContext()
{
  if(m_depth>0) {
    return(m_contextStack[m_depth-1]);
  }
  else {
    return(NULL);
  }
}
