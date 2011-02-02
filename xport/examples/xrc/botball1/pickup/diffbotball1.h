#ifndef _DIFFBOTBALL1_H
#define _DIFFBOTBALL1_H

#include "botball1menu.h"
#include "diffbase.h"

class CDiffBotball1Menu : public CBotball1Menu
	{
public:
	CDiffBotball1Menu(CVision &vision, CVisionDispOptions &options, CDiffBase &base, CGpioInt &gpio);
	virtual ~CDiffBotball1Menu();


protected:
	void PopulateMenu();
	
	void MoveForward();
	void TurnRight();
	void TurnLeft();
	virtual void ShowMotorPositions();

	virtual bool HandleMenuEvent(int eventType, CMenuElement &menu);
	CDiffBase &m_diffBase;
	};

#endif
