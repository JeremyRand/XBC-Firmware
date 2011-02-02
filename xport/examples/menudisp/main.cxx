#include <stdio.h>
#include "dispcontext.h"
#include "menucontext.h"
#include "palette.h"
#include "textdisp.h"
#include "btnstate.h"

using namespace std;

class CMHTest : public IMenuHandler
{
public:
  CMHTest() : m_enable(false), m_x(120), m_y(80), 
	      m_enableEnt("Enable", m_enable),
	      m_xEnt("X", m_x), 
	      m_yEnt("Y", m_y) {}
  virtual ~CMHTest() {}
  void PopulateMenu(CMenuElement &menuElt) {
    CMenuElement *subElem = menuElt.AddMenuLink("mhtest", this);
    m_enableEnt.SetName(*subElem->AddMenuItem("", 0));
    m_xEnt.SetName(*subElem->AddMenuItem("", 1));
    m_yEnt.SetName(*subElem->AddMenuItem("", 2));
  }
  virtual bool HandleMenuEvent(int eventType, 
			       CMenuElement &menu) {
    CMenuItem *item = menu.GetItemAt(eventType);
    if(item == NULL) {
      return(false);
    }

    switch(eventType) {
    case 0:
      m_enableEnt.Update(*item);
      break;
    case 1:
      m_xEnt.Update(*item);
      break;
    case 2:
      m_yEnt.Update(*item);
      break;
    default:
      return(false);
    }
    return(true);
  }
protected:
  bool m_enable;
  int m_x, m_y;
  CBoolEntry m_enableEnt;
  CIntEntry m_xEnt, m_yEnt;
};


CTextDisp td(TDM_LCD_AND_CPORT);

int main()
{
  CMHTest mhtest;
  CMenuDisp menu(&mhtest);

  CMenuElement *elem = menu.GetCurrentElement();
  mhtest.PopulateMenu(*elem);

  CMenuContext *mContext = new CMenuContext(menu);
  DispContextStackSingleton.PushContext(mContext);

  int i=0;
  CBtnState btnState;

  while(1) {
    DispContextStackSingleton.Process();
    btnState.PollKeys();

    if(btnState.KeyHit(CBtnState::A_BUTTON)) {
      printf("Hi %d\n", i++);
    }
  }
}
