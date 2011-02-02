#include "menucontext.h"
#include "btnstate.h"
#include "textdisp.h"
#include "textcontext.h"
#include "palette.h"
#include "regbits.h"
#include <stdlib.h>

CMenuContext::CMenuContext(CMenuDisp &menu, bool exit) :
    m_menu(menu), m_exit(exit), m_palette(NULL), m_font(NULL), 
    m_prevPalette(NULL), m_prevFont(NULL)
{
}
bool 
CMenuContext::Setup()
{
  //Turn off screen blank bit, as it is turned on at boot
  BFSET(GBA_REG_DISPCNT, forceScreenBlank, 0);
  
  // If there is already a palette or font in use, just leave it alone.
  // Otherwise, create defaults
  m_prevPalette = CDisplay::GetCurrPalette();
  if(m_prevPalette == NULL) {
    // Setup default
    m_palette = new CPalette(CPalette::BKGND, 254, 253);
    m_palette->SetColor(253, 0x0); // BG: black/transparent
    m_palette->SetColor(254, 0xffff); // FG: white
    CDisplay::SetCurrPalette(m_palette);
  }
  else {
    m_palette = (CPalette *)m_prevPalette;
  }

  m_prevFont = CDisplay::GetCurrDispFont();
  if(m_prevFont == NULL) {
    // Setup default
    m_font = new CDispFont(0, *m_palette);
    CDisplay::SetCurrDispFont(m_font);
  }
  else {
    m_font = (CDispFont *)m_prevFont;
  }

  return(Focus());
}

bool 
CMenuContext::Blur()
{
  return(true);
}

bool 
CMenuContext::Focus()
{
  // Setup palette
  m_palette->WriteToGBA();
  CDisplay::SetCurrPalette(m_palette);

  // Setup font
  m_font->WriteToGBA();
  CDisplay::SetCurrDispFont(m_font);

  // Setup menu display
  m_menu.SetupDisplay();

  // Ignore whatever buttons are currently being pressed
  m_menuContextIgnoreBtns=true;
  return(true);
}

bool 
CMenuContext::Shutdown()
{
  if(m_prevPalette == NULL) {
    // We had to use a default palette.  Unregister m_palette and
    // delete it
    CDisplay::SetCurrPalette(NULL);
    delete m_palette;
  }
  if(m_prevFont == NULL) {
    // We had to use a default font.  Unregister m_font and
    // delete it
    CDisplay::SetCurrDispFont(NULL);
    delete m_font;
  }
  return(true);
}

bool 
CMenuContext::Process()
{
  static CBtnState btnState;
  bool result = true;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  if(m_menuContextIgnoreBtns) {
    btnState.PollKeys();
    m_menuContextIgnoreBtns=false;
  }

  btnState.PollKeys();

  if(btnState.KeyHit(CBtnState::DOWN_BUTTON)) m_menu.IncreaseItemIndex();
  if(btnState.KeyHit(CBtnState::UP_BUTTON)) m_menu.DecreaseItemIndex();
  if(btnState.KeyHit(CBtnState::A_BUTTON) || 
     btnState.KeyHit(CBtnState::LEFT_BUTTON) ||
     btnState.KeyHit(CBtnState::RIGHT_BUTTON)) {
    m_menu.SelectMenuItem();
  }
  if(btnState.KeyHit(CBtnState::B_BUTTON)) result = m_menu.GoToParentMenu();
  if(btnState.KeyHit(CBtnState::SELECT_BUTTON)
#ifndef NORMBUTTONS
	  && btnState.KeyDown(CBtnState::L_BUTTON)
#endif
	  ) {
    // Push textdisp on top of the stack
    CTextContext::Push();
    return(true);
  }
  m_menu.DrawMenu();

  if (m_exit)
	  return(result);
  else
	  return(true);
}
