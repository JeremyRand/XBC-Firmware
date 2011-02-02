#include "stdio.h"
#include "btnstate.h"
#include "menudisp.h"
#include "gba.h"
#include "string.h"
#include "regbits.h"
#include "palette.h"

// #define DEBUG

// #ifdef DEBUG
// #include "breadcrumbs.h"
// #define DBG2(x) ADD_CRUMB(0xe900); \
//                 ADD_DEBUG_CRUMB(0xe902, x)
// #define DBG4(x) ADD_CRUMB(0xe900); \
//                 ADD_DEBUG_CRUMB(0xe904, x>>16); \
//                 ADD_DEBUG_CRUMB(0xe904, x&0xffff)
// #else
#define DBG2(x) 
#define DBG4(x) 
// #endif

//////////////////////////////////////////////////////////////////////
// CMenuItem
CMenuItem::CMenuItem(const char* itemName, int itemMessage) : m_isMenuLink(false), m_itemMessage(itemMessage)
{
  SetName(itemName);
}

CMenuItem::CMenuItem(const char* itemName, CMenuElement* childElement) : m_isMenuLink(true), m_childElement(childElement)
{
  SetName(itemName);
}

void CMenuItem::SetName(const char *newName) 
{
  strncpy(m_itemName, newName, MENU_NAMELEN);
  m_itemName[MENU_NAMELEN-1]='\0';
}

//////////////////////////////////////////////////////////////////////
// CMenuElement
CMenuElement::CMenuElement(CMenuElement* parentElement, IMenuHandler* menuHandler)
	: m_numMenuItems(0), 
		m_currentItemIndex(0), 
		m_parentElement(parentElement), 
		m_menuHandler(menuHandler)
{
  DBG2(0xe1cc);
  DBG4(((int)this));
}


CMenuItem *CMenuElement::AddMenuItem(const char* itemName, int itemMessage)
{
	if(m_numMenuItems == MAX_MENU_ITEMS)
	{
		return(NULL);
	}
	++m_numMenuItems;
	int currentIndex = m_numMenuItems - 1;
	m_menuItems[currentIndex] = new CMenuItem(itemName, itemMessage);
	return(m_menuItems[currentIndex]);
}

CMenuElement* CMenuElement::AddMenuLink(const char* itemName, IMenuHandler* menuHandler)
{
	if(m_numMenuItems == MAX_MENU_ITEMS)
	{
		return NULL;
	}
	++m_numMenuItems;
	int currentMaxIndex = m_numMenuItems - 1;
	m_menuItems[currentMaxIndex] = new CMenuItem(itemName, new CMenuElement(this, menuHandler));
	return m_menuItems[currentMaxIndex]->GetChildMenu();
}

CMenuElement* CMenuElement::SelectMenuItem()
{
	if(m_menuItems[m_currentItemIndex]->IsMenuLink())
	{
		return m_menuItems[m_currentItemIndex]->GetChildMenu();
	}
	m_menuHandler->HandleMenuEvent(m_menuItems[m_currentItemIndex]->GetMessage(), 
				       *this);
	return NULL;
}

//////////////////////////////////////////////////////////////////////
// CMenuDisp
CMenuDisp::CMenuDisp(IMenuHandler* menuHandler) : m_menuElement(new CMenuElement(NULL, menuHandler))
{
  DBG2(0xd1cc);
  DBG4(((int)this))
  DBG4(((int)m_menuElement))
}

CMenuDisp::~CMenuDisp()
{
}

void CMenuDisp::SetupDisplay()
{
  DBG4(((int)m_menuElement))

  //Turn on Mode 0, background 2, and sprite drawing
  BFSET(GBA_REG_DISPCNT, videoMode, 0);
  BFSET(GBA_REG_DISPCNT, drawBG2, 1);

  // Turn on Background 2 settings 
  // 0 = 256x256, since our background is 240x160, this should be fine

  BFSET(GBA_REG_BG2CNT, backgroundSize, 0x00);
  // Highest priority
  BFSET(GBA_REG_BG2CNT, priority, 0x1);
  
  // Set scroll to zero
  GBA_REG_BG2HOFS = 0;
  GBA_REG_BG2VOFS = 0;

  // Setup tile block based on the font info
  CDispFont const *font=CDisplay::GetCurrDispFont();
  if(font==NULL) {
    return;
  }
  unsigned short bblock = font->GetCharBaseBlock();
  BFSET(GBA_REG_BG2CNT, baseTileBlock, bblock);

  // Setup base for map data for BG2 
  BFSET(GBA_REG_BG2CNT, baseMapBlock, 27);
  //No mosaic
  BFSET(GBA_REG_BG2CNT, mosaic, 0x0);
  //256 color palette
  BFSET(GBA_REG_BG2CNT, palette, 0x1);
  GBA_REG_BG2VOFS = ((unsigned short)-32);
  GBA_REG_BG2HOFS = ((unsigned short)-8);

  // Clear buffer and draw menu
  ClearMenuBackground();
  DrawMenu();

  BFSET(GBA_REG_DISPCNT, drawBG2, 1);
  DBG4(((int)m_menuElement))
}

void CMenuDisp::DrawMenu()
{
  DBG4(((int)m_menuElement))
  CDispFont const *font=CDisplay::GetCurrDispFont();
  if(font==NULL) {
    return;
  }

  unsigned short* menuMemory = (unsigned short*)(0x6000000 + (27*2048));
  //Frame background and sprites

  const char* itemName;
  unsigned short centerValue, letterPosition;
  for(unsigned short i = 0; i < m_menuElement->GetNumMenuItems(); ++i)
    {
      bool rev = false;
      
      if(i == m_menuElement->GetCurrentItemIndex()) {
	rev=true;
      }

      itemName = m_menuElement->GetItemNameAt(i);
      centerValue = (27 - strlen(itemName)) / 2;
      for(letterPosition = 0; letterPosition < centerValue; ++letterPosition)
	{
	  menuMemory[letterPosition] = font->AsciiToTileIndex(' ');
	}
      for(; letterPosition < strlen(itemName) + centerValue; ++letterPosition)
	{
	  unsigned char c = itemName[letterPosition-centerValue];
	  menuMemory[letterPosition] = font->AsciiToTileIndex(c, rev);
	}
      for(letterPosition = strlen(itemName) + centerValue; 
	  letterPosition < 32; ++letterPosition)
	{
	  menuMemory[letterPosition] = font->AsciiToTileIndex(' ');
	}
      menuMemory += 32;
    }
  DBG4(((int)m_menuElement))
}

CMenuItem *CMenuDisp::AddMenuItem(const char* itemName, int itemMessage)
{
  DBG4(((int)m_menuElement))
  return(m_menuElement->AddMenuItem(itemName, itemMessage));
}

CMenuElement* 
CMenuDisp::AddMenuLink(const char* itemName, IMenuHandler* menuHandler)
{
  DBG4(((int)m_menuElement))
  return(m_menuElement->AddMenuLink(itemName, menuHandler));
}

void CMenuDisp::SelectMenuItem()
{
  DBG4(((int)m_menuElement))
	ClearMenuBackground();
	CMenuElement* newElement = m_menuElement->SelectMenuItem();
	//If it was a command execution, we don't care
	if(newElement == NULL) return;
	//If it was a new menu, we care
	m_menuElement = newElement;
}

void CMenuDisp::IncreaseItemIndex()
{
  DBG4(((int)m_menuElement))
	m_menuElement->IncreaseItemIndex();
}

void CMenuDisp::DecreaseItemIndex()
{
  DBG4(((int)m_menuElement))
	m_menuElement->DecreaseItemIndex();
}

void CMenuDisp::ClearMenuBackground()
{
  DBG4(((int)m_menuElement))
  CDispFont const *font=CDisplay::GetCurrDispFont();
  if(font==NULL) {
    return;
  }
  unsigned short* menuMemory = (unsigned short*)(0x6000000 + (27*2048));
  for(int i = 0; i < 32*32; ++i) {
    menuMemory[i] = font->AsciiToTileIndex(' ');
  }
  DBG4(((int)m_menuElement))
}

bool CMenuDisp::GoToParentMenu() 
{ 
  bool result;
  DBG4(((int)m_menuElement))
  ClearMenuBackground(); 
  if(m_menuElement->GetParentMenu()) 
  {
    m_menuElement = m_menuElement->GetParentMenu(); 
	result = true;
  }
  else 
    result = false;
  DBG4(((int)m_menuElement))
  return result;
}

// Setup/modification support
CMenuElement *CMenuDisp::GetCurrentElement() 
{
  DBG4(((int)m_menuElement))
  return(m_menuElement);
}

////////////////////////////////////////////////////////////////////////
// Helper classes for menu entries for display/modification of values
bool 
CBoolEntry::Update(CMenuItem &menuItem) 
{
  // Toggle value
  m_val = !m_val;
  SetName(menuItem);
  return(true);
}

void 
CBoolEntry::SetName(CMenuItem &menuItem) 
{
  // Setup new name to reflect status
  char buf[MENU_NAMELEN];
  sprintf(buf, "%s=%s", m_name, m_val?"true":"false");
  menuItem.SetName(buf);
}

bool 
CIntEntry::Update(CMenuItem &menuItem) 
{
  // Read buttons and modify value
  CBtnState bstate;
  if(!(bstate.CurrState() & CBtnState::LEFT_BUTTON)) {
    m_val -= m_step;
  } else if(!(bstate.CurrState() & CBtnState::RIGHT_BUTTON)) {
    m_val += m_step;
  }
  else {
    return(false);
  }
  SetName(menuItem);
  return(true);
}

void 
CIntEntry::SetName(CMenuItem &menuItem) 
{
  // Setup new name to reflect status
  char buf[MENU_NAMELEN];
  sprintf(buf, "%s=%d", m_name, m_val);
  menuItem.SetName(buf);
}
