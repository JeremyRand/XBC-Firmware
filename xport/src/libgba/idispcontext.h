#ifndef IDISPCONTEXT_H
#define IDISPCONTEXT_H

class IDispContext
{
public:
  IDispContext() {}
  virtual ~IDispContext() {}

  // Newly created context, now top of stack
  virtual bool Setup() {return(false);}	

  // Newer context on top of stack, lose focus
  virtual bool Blur() {return(false);}	

  // Newer context(s) done, top of stack again
  virtual bool Focus() {return(false);}	

  // Context done, being popped off stack
  virtual bool Shutdown() {return(false);}	

  // Do a quantum of work and return.  If returns true, keep on top of stack.
  // If returns false, pop off the stack.
  virtual bool Process() {return(false);}	
};

#endif
