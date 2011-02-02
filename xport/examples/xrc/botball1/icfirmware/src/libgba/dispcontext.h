#ifndef DISPCONTEXT_H
#define DISPCONTEXT_H

#include "singleton.h"
#include "idispcontext.h"

#define MAX_CONTEXT_DEPTH 10

/////////////////////////////////////////////////////////////////////
// Disp context stack
#define DispContextStackSingleton CDispContextStack::Instance()

class CDispContextStack : public CSingleton<CDispContextStack>
{
public:
  CDispContextStack();
  ~CDispContextStack();

  // Blur current context, push new context on top of stack and run
  // setup.  CDislayContextStack takes ownership of context and will
  // free it when popped
  bool PushContext(IDispContext *context);
  // Shutdown and free current context, Focus previous context
  bool PopContext();

  // Do a quantum of work on top item of stack
  bool Process();

  // Return current context
  IDispContext *GetCurrentContext();
  // Return the depth of the context stack
  int GetDepth() { return m_depth; }
  
protected:
  IDispContext* m_contextStack[MAX_CONTEXT_DEPTH];
  int              m_depth;
};

#endif
