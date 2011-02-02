//  BEGIN LICENSE BLOCK
// 
//  Version: MPL 1.1
// 
//  The contents of this file are subject to the Mozilla Public License Version
//  1.1 (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
// 
//  Software distributed under the License is distributed on an "AS IS" basis,
//  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
//  for the specific language governing rights and limitations under the
//  License.
// 
//  The Original Code is The Xport Software Distribution.
// 
//  The Initial Developer of the Original Code is Charmed Labs LLC.
//  Portions created by the Initial Developer are Copyright (C) 2003
//  the Initial Developer. All Rights Reserved.
// 
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
// 
//  END LICENSE BLOCK 

#ifndef _TEXTCONTEXT_H
#define _TEXTCONTEXT_H

#include "textdisp.h"
#include "idispcontext.h"

// Implement specialization of IDispContext for text display
class CPalette;
class CDispFont;

class CTextContext : public IDispContext 
{
public:
  CTextContext();
  virtual ~CTextContext() {}

  virtual bool Setup();	    // Newly created context, now top of stack
  virtual bool Blur();	  // Newer context on top of stack, lose focus
  virtual bool Focus();	  // Newer context(s) done, top of stack again
  virtual bool Shutdown();     // Context done, being popped off stack

  virtual bool Process();	// Do a quantum of work and return

  static bool Push();		// Push onto top of display stack if
				// top is not already showing TextDisp
  static bool Pop();		// Pop off top of display stack if 
				// top is currently showing TextDisp
protected:
  CPalette const *m_prevPalette;
  CDispFont const *m_prevFont;

  CPalette *m_palette;
  CDispFont *m_font;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_textContextIgnoreBtns;
};

#endif
