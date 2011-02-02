#ifndef CMenu_H
#define CMenu_H

//#define MAX_MENU_ITEMS 10
#define MAX_MENU_ITEMS 99
#define MENU_NAMELEN 30

#include "display.h"

class IMenuHandler;
class CMenuElement;

class CMenuItem
{
  bool m_isMenuLink;
  char m_itemName[MENU_NAMELEN];
  int m_itemMessage;
  CMenuElement* m_childElement;
public:
  CMenuItem(const char* itemName, CMenuElement* childElement);
  CMenuItem(const char* itemName, int itemMessage);
  bool IsMenuLink() { return m_isMenuLink; }
  const char* GetName() { return m_itemName; }
  void SetName(const char *newName);
  int GetMessage() { if (!m_isMenuLink) return m_itemMessage; return -1; }
  CMenuElement* GetChildMenu() { return m_childElement; }
};

class CMenuElement
{
  CMenuItem* m_menuItems[MAX_MENU_ITEMS];
  unsigned short m_numMenuItems;
  unsigned short m_currentItemIndex;
  CMenuElement* m_parentElement;
  IMenuHandler* m_menuHandler;
public:
  CMenuElement(CMenuElement* parentElement, IMenuHandler* menuHandler);
  int GetNumMenuItems() { return m_numMenuItems; }
  const char* GetItemNameAt(int index) { return m_menuItems[index]->GetName(); }
  CMenuItem* GetItemAt(int index) { return m_menuItems[index]; }
  CMenuItem* AddMenuItem(const char* itemName, int itemMessage);
  CMenuElement* AddMenuLink(const char* itemName, IMenuHandler* menuHandler);
  CMenuElement* SelectMenuItem();
  unsigned short GetCurrentItemIndex() { return m_currentItemIndex; }
  void IncreaseItemIndex() { if (m_currentItemIndex < (m_numMenuItems - 1)) ++m_currentItemIndex; else m_currentItemIndex=0;}
  void DecreaseItemIndex() { if (m_currentItemIndex > 0) --m_currentItemIndex; else m_currentItemIndex=m_numMenuItems-1; }
  CMenuElement* GetParentMenu() { return m_parentElement; }
  ~CMenuElement();
};

class CMenuDisp
{
public:
private:
  CMenuElement* m_menuElement;
public:
  CMenuDisp(IMenuHandler* menuHandler);
  ~CMenuDisp();

  CMenuItem *AddMenuItem(const char* itemName, int itemMessage);
  CMenuElement *AddMenuLink(const char* itemName, IMenuHandler* menuHandler);

  // Menu
  void SelectMenuItem();
  void IncreaseItemIndex();
  void DecreaseItemIndex();
  bool GoToParentMenu();

  // Setup/modification support
  CMenuElement *GetCurrentElement();

  // Display support.  The menu displays on BG2, which is available in
  // video modes 0 through 2, and uses video memory in CharBaseBlock
  // 27 - 31.  SetupDisplay sets up the BG2 registers for the menu.
  // This assumes that the display context has already been set up in
  // a compatible video mode, and that a CDispFont has already been
  // loaded and registered with CDisplay.  See CMenuContext for
  // details.
  void SetupDisplay();
  void ClearMenuBackground();
  void DrawMenu();
};

////////////////////////////////////////////////////////////////////////
// Base class for menu handlers
class IMenuHandler
{
public:
	IMenuHandler() {}
	virtual ~IMenuHandler() {}
	virtual bool HandleMenuEvent(int eventType, CMenuElement &menu) = 0;
};

////////////////////////////////////////////////////////////////////////
// Helper classes for menu entries for display/modification of values
class CValueEntry {
public:
  CValueEntry(const char *name) : m_name(name) {}
  virtual ~CValueEntry() {}
  virtual void SetName(CMenuItem &menuItem) = 0;
  virtual bool Update(CMenuItem &menuItem) = 0;
protected:
  const char *m_name;
};

class CBoolEntry : public CValueEntry 
{
public:
  CBoolEntry(const char *name, bool &val) : 
    CValueEntry(name), m_val(val) {}
  virtual ~CBoolEntry() {}
  virtual void SetName(CMenuItem &menuItem);
  virtual bool Update(CMenuItem &menuItem);
protected:
  bool &m_val;
};

class CIntEntry : public CValueEntry 
{
public:
  CIntEntry(const char *name, int &val, int step=1) : 
    CValueEntry(name), m_val(val), m_step(step) {}
  virtual ~CIntEntry() {}
  virtual void SetName(CMenuItem &menuItem);
  virtual bool Update(CMenuItem &menuItem);
protected:
  int &m_val;
  int m_step;
};

#endif
