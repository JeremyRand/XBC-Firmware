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

#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "gba.h"
#include "textdisp.h"
#include "palette.h"
#include "display.h"
#include "regbits.h"
#include "btnstate.h"
#include "dispcontext.h"
#include "textcontext.h"

// Default palette and font in case nobody else has set them up
CPalette *ptd_defaultPalette;
CDispFont *ptd_defaultFont;

//////////////////////////////////////////////////////////////////////////
CTextContext::CTextContext()
{
}
bool 
CTextContext::Setup()
{
  //Turn off screen blank bit, as it is turned on at boot
  BFSET(GBA_REG_DISPCNT, forceScreenBlank, 0);
  
  // If there is already a palette or font in use, just leave it alone.
  // Otherwise, create defaults
  m_prevPalette = CDisplay::GetCurrPalette();
  if(m_prevPalette == NULL) {
    // Setup or use default
    if(ptd_defaultPalette == NULL) {
      ptd_defaultPalette = new CPalette(CPalette::BKGND, 254, 253);
      ptd_defaultPalette->SetColor(253, 0x0); // BG: black/transparent
      ptd_defaultPalette->SetColor(254, 0xffff); // FG: white
      ptd_defaultPalette->SetColor(2, 0, 0xff, 0);
    }
    m_palette = ptd_defaultPalette;
    CDisplay::SetCurrPalette(ptd_defaultPalette);
  }
  else {
    m_palette = (CPalette *)m_prevPalette;
  }

  m_prevFont = CDisplay::GetCurrDispFont();
  if(m_prevFont == NULL) {
    // Setup or use default
    if(ptd_defaultFont == NULL) {
      ptd_defaultFont = new CDispFont(0, *m_palette);
    }
    m_font = ptd_defaultFont;
    CDisplay::SetCurrDispFont(m_font);
  }
  else {
    m_font = (CDispFont *)m_prevFont;
  }

  return(Focus());
}

bool 
CTextContext::Blur()
{
  // Backup the text display memory
  ptd->BackupDisplay();
  return(true);
}

bool 
CTextContext::Focus()
{
  // Setup palette
  m_palette->WriteToGBA();
  CDisplay::SetCurrPalette(m_palette);

  // Setup font
  m_font->WriteToGBA();
  CDisplay::SetCurrDispFont(m_font);

  // Restore text display
  ptd->RestoreDisplay();

  // Ignore whatever buttons are currently being pressed
  m_textContextIgnoreBtns=true;
  return(true);
}

bool 
CTextContext::Shutdown()
{
  Blur();
  return(true);
}

bool 
CTextContext::Process()
{
  static CBtnState btnState;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  if(m_textContextIgnoreBtns) {
    btnState.PollKeys();
    m_textContextIgnoreBtns=false;
  }

  btnState.PollKeys();

//   printf("CTextContext: %x -> %x\n", btnState.PrevState(), 
// 	 btnState.CurrState());

  if(btnState.KeyHit(CBtnState::DOWN_BUTTON)) {
    const CTextBuf *sbuf = ptd->GetScreenTextBuf();
    if(sbuf->ValidLines()>TD_SCREEN_HEIGHT && 
       sbuf->ScrollPos()!=sbuf->EndScrollPos()) {
      ptd->Scroll(1);
    }
  }
  if(btnState.KeyHit(CBtnState::UP_BUTTON)) {
    const CTextBuf *sbuf = ptd->GetScreenTextBuf();
    if(sbuf->ValidLines()>TD_SCREEN_HEIGHT && 
       sbuf->ScrollPos()!=sbuf->TopLine()) {
      ptd->Scroll(-1);
    }
  }
  if(btnState.KeyHit(CBtnState::LEFT_BUTTON)) ptd->ScrollToTextEnd();
  if(btnState.KeyHit(CBtnState::RIGHT_BUTTON)) ptd->ScrollToTextEnd();
  if(btnState.KeyHit(CBtnState::B_BUTTON)) {
    // If this is the last item in the stack, do nothing.  Otherwise
    // pop this context
    if(DispContextStackSingleton.GetDepth()>1) {
      return(false);
    }
  }

  return(true);
}

bool 
CTextContext::Push()
{
  // Check if top of stack is already CTextContext
  if(dynamic_cast<CTextContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is already CTextContext, don't do anything
    return(false);
  }
  // Create a text context and push it onto the stack above this menu
  CTextContext *tContext = new CTextContext();
  DispContextStackSingleton.PushContext(tContext);
  return(true);
}

bool 
CTextContext::Pop()
{
  // Check if top of stack is already CTextContext
  if(dynamic_cast<CTextContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is text context, pop it
    DispContextStackSingleton.PopContext();
    return(true);
  }
  else {
    // Top is not text context, do nothing
    return(false);
  }
}
