#ifndef _BOTBALL1MENU_H
#define _BOTBALL1MENU_H

#include "menucontext.h"
#include "visioncontext.h"
#include "visionmenu.h"
#include "axesc.h"
#include "gpioint.h"

class CBotball1Menu : public IMenuHandler, public CMenuContext
	{
public:
	CBotball1Menu(CVision &vision, CVisionDispOptions &options, CAxesClosed &axes, CGpioInt &gpio);
	virtual ~CBotball1Menu();
	
	bool HandleMenu();

protected:
	void PopulateMenu();
	virtual bool HandleMenuEvent(int eventType, CMenuElement &menu);

	virtual void DigitalValues();
	virtual void AnalogValues();
	virtual void BatteryVoltage();
	virtual void ShowMotorPositions();
	virtual void HoldMotorPositions();
	virtual void Record();

	CMenuDisp m_menu;
	CVisionMenuHandler m_vMenu;
	CAxesClosed &m_axes;
	CGpioInt &m_gpio;
};


#endif
