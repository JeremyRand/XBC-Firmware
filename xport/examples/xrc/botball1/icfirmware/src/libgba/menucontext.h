#ifndef CMENUCONTEXT_H
#define CMENUCONTEXT_H

#include "dispcontext.h"
#include "menudisp.h"

// This context holds onto a reference to an associated menu, but does
// not own the storage for the menu.  It is up to the user to make
// sure that the context is freed before its menu, and that the menu
// is eventually freed.
class CMenuContext : public IDispContext
{
public:
  CMenuContext(CMenuDisp &menu, bool exit=false);
  virtual ~CMenuContext() {}

  virtual bool Setup();	    // Newly created context, now top of stack
  virtual bool Blur();	  // Newer context on top of stack, lose focus
  virtual bool Focus();	  // Newer context(s) done, top of stack again
  virtual bool Shutdown();     // Context done, being popped off stack

  virtual bool Process();	// Do a quantum of work and return

protected:
  CMenuDisp &m_menu;
  CPalette *m_palette;
  CDispFont *m_font;
  
  CPalette const *m_prevPalette;
  CDispFont const *m_prevFont;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_menuContextIgnoreBtns;
  bool m_exit;  // can be popped off of context stack
};

#endif
