#include "diffbotball1.h"
#include "textcontext.h"
#include "btnstate.h"
#include "menu.h"

CDiffBotball1Menu::CDiffBotball1Menu(CVision &vision, CVisionDispOptions &options, CDiffBase &base, CGpioInt &gpio) :
	CBotball1Menu(vision, options, base, gpio), m_diffBase(base)
	{
	PopulateMenu();
	}

CDiffBotball1Menu::~CDiffBotball1Menu()
	{
	}

void CDiffBotball1Menu::PopulateMenu()
	{
	m_menu.AddMenuItem("Move forward", 0);
	m_menu.AddMenuItem("Turn right", 1);
	m_menu.AddMenuItem("Turn left", 2);
	}

bool CDiffBotball1Menu::HandleMenuEvent(int eventType, CMenuElement &menu)
	{
    switch(eventType) 
		{
		case 0:
		MoveForward();
		break;

		case 1:
		TurnRight();
		break;

		case 2:
		TurnLeft();
		break;
		}

	return CBotball1Menu::HandleMenuEvent(eventType, menu);
	}

void CDiffBotball1Menu::ShowMotorPositions()
	{
	CBtnState btnState;

	ptd->Clear();
    CTextContext::Push();

	CMenu::Printxyf(4, 3, 0, false, "Translation");
	CMenu::Printxyf(18, 3, 0, false, "Rotation");
	CMenu::Printxyf(4, 7, 0, false, "Left (raw)");
	CMenu::Printxyf(18, 7, 0, false, "Right (raw)");

	while(1)
		{
	    btnState.PollKeys();
		if(btnState.KeyHit(CBtnState::B_BUTTON))
			break;

		CMenu::Printxyf(4, 4, 8, false, "%d", m_diffBase.GetPosition(TRANS_AXIS));
		CMenu::Printxyf(18, 4, 8, false, "%d", m_diffBase.GetPosition(ROTATE_AXIS));
		CMenu::Printxyf(18, 8, 8, false, "%d", m_diffBase.CAxesOpen::GetPosition(0));
		CMenu::Printxyf(4, 8, 8, false, "%d", m_diffBase.CAxesOpen::GetPosition(1));
		}
    CTextContext::Pop();
	}

void CDiffBotball1Menu::MoveForward()
	{
	ptd->Clear();
    CTextContext::Push();

	m_diffBase.SyncTrajectory(TRANS_AXIS);
	m_diffBase.Move(TRANS_AXIS, m_diffBase.GetPosition(TRANS_AXIS)+10000, 1800, 1800);
	m_diffBase.Execute();
	CMenu::Printxyf(8, 6, 0, false, "Translation");
	while(!m_diffBase.Done(TRANS_AXIS))
		CMenu::Printxyf(11, 7, 8, false, "%d", m_diffBase.GetPosition(TRANS_AXIS));

    CTextContext::Pop();	
	}


void CDiffBotball1Menu::TurnRight()
	{
	ptd->Clear();
    CTextContext::Push();

	m_diffBase.SyncTrajectory(ROTATE_AXIS);
	m_diffBase.Move(ROTATE_AXIS, m_diffBase.GetPosition(ROTATE_AXIS)-1571, 2000, 2000);
	m_diffBase.Execute();
	CMenu::Printxyf(10, 6, 0, false, "Rotation");
	while(!m_diffBase.Done(ROTATE_AXIS))
		CMenu::Printxyf(11, 7, 8, false, "%d", m_diffBase.GetPosition(ROTATE_AXIS));

    CTextContext::Pop();	
	}

void CDiffBotball1Menu::TurnLeft()
	{
	ptd->Clear();
    CTextContext::Push();

	m_diffBase.SyncTrajectory(ROTATE_AXIS);
	m_diffBase.Move(ROTATE_AXIS, m_diffBase.GetPosition(ROTATE_AXIS)+1571, 2000, 2000);
	m_diffBase.Execute();
	CMenu::Printxyf(10, 6, 0, false, "Rotation");
	while(!m_diffBase.Done(ROTATE_AXIS))
		CMenu::Printxyf(11, 7, 8, false, "%d", m_diffBase.GetPosition(ROTATE_AXIS));

    CTextContext::Pop();	
	}


