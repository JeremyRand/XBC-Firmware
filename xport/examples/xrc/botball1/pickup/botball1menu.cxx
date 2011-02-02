#include "botball1menu.h"
#include "textcontext.h"
#include "blobcontext.h"
#include "btnstate.h"
#include "menu.h"

CBotball1Menu::CBotball1Menu(CVision &vision, CVisionDispOptions &options, CAxesClosed &axes, CGpioInt &gpio) : 
	CMenuContext(m_menu, true), m_menu(this), m_vMenu(vision, options), m_axes(axes), m_gpio(gpio)
	{
	PopulateMenu();
	}

CBotball1Menu::~CBotball1Menu() 
	{
	}

bool CBotball1Menu::HandleMenu()
	{
	CBtnState btnState;

    btnState.PollKeys();
	if(btnState.KeyHeld(CBtnState::A_BUTTON))
		{
		DispContextStackSingleton.PushContext(this);
		while(DispContextStackSingleton.GetDepth()>1)
			DispContextStackSingleton.Process();
		ptd->Clear();
		return true;
		}
	return false;
	}

void CBotball1Menu::DigitalValues()
	{
	}

void CBotball1Menu::PopulateMenu()
	{
	CMenuElement *topElt = m_menu.GetCurrentElement();
	m_vMenu.PopulateMenu(*topElt);
//	m_menu.AddMenuItem("Digital values", 1000);
	m_menu.AddMenuItem("Analog values", 1001);
	m_menu.AddMenuItem("Battery voltage", 1002);
	m_menu.AddMenuItem("Show motor positions", 1003);
	m_menu.AddMenuItem("Hold motor positions", 1004);
	m_menu.AddMenuItem("Record", 1005);
	}

void CBotball1Menu::AnalogValues()
	{
	CBtnState btnState;
	ptd->Clear();
    CTextContext::Push();

	CMenu::Printxyf(6, 3, 3, true, " 1 ");
	CMenu::Printxyf(11, 3, 3, true, " 2 ");
	CMenu::Printxyf(16, 3, 3, true, " 3 ");
	CMenu::Printxyf(21, 3, 3, true, " 4 ");
	CMenu::Printxyf(6, 7, 3, true, " 5  ");
	CMenu::Printxyf(11, 7, 3, true, " 6 ");
	CMenu::Printxyf(16, 7, 3, true, " 7 ");
	CMenu::Printxyf(21, 7, 3, true, " 8 ");

	while(1)
		{
	    btnState.PollKeys();
		if(btnState.KeyHit(CBtnState::B_BUTTON))
			break;

		CMenu::Printxyf(6, 4, 3, false, "%03x", m_axes.GetAnalog(0));
		CMenu::Printxyf(11, 4, 3, false, "%03x", m_axes.GetAnalog(1));
		CMenu::Printxyf(16, 4, 3, false, "%03x", m_axes.GetAnalog(2));
		CMenu::Printxyf(21, 4, 3, false, "%03x", m_axes.GetAnalog(3));
		CMenu::Printxyf(6, 8, 3, false, "%03x", m_axes.GetAnalog(4));
		CMenu::Printxyf(11, 8, 3, false, "%03x", m_axes.GetAnalog(5));
		CMenu::Printxyf(16, 8, 3, false, "%03x", m_axes.GetAnalog(6));
		CMenu::Printxyf(21, 8, 3, false, "%03x", m_axes.GetAnalog(7));
		}
    CTextContext::Pop();
	}

void CBotball1Menu::BatteryVoltage()
	{
	CBtnState btnState;
	ptd->Clear();
    CTextContext::Push();

	CMenu::Printxyf(7, 7, 0, false, "Battery voltage");
	while(1)
		{
	    btnState.PollKeys();
		if(btnState.KeyHit(CBtnState::B_BUTTON))
			break;
		CMenu::Printxyf(12, 8, 0, false, "%3.2fV", (float)m_axes.GetAnalog(7)*0.00214);
		}
    CTextContext::Pop();
	}


void CBotball1Menu::ShowMotorPositions()
	{
	CBtnState btnState;

	ptd->Clear();
    CTextContext::Push();

	CMenu::Printxyf(4, 3, 0, false, "Axis 0");
	CMenu::Printxyf(18, 3, 0, false, "Axis 1");
	CMenu::Printxyf(4, 7, 0, false, "Axis 2");
	CMenu::Printxyf(18, 7, 0, false, "Axis 3");

	while(1)
		{
	    btnState.PollKeys();
		if(btnState.KeyHit(CBtnState::B_BUTTON))
			break;

		CMenu::Printxyf(4, 4, 8, false, "%d", m_axes.GetPosition(0));
		CMenu::Printxyf(18, 4, 8, false, "%d", m_axes.GetPosition(1));
		CMenu::Printxyf(18, 8, 8, false, "%d", m_axes.GetPosition(2));
		CMenu::Printxyf(4, 8, 8, false, "%d", m_axes.GetPosition(3));
		}
    CTextContext::Pop();
	}

void CBotball1Menu::HoldMotorPositions()
	{
	CBtnState btnState;

	m_axes.SyncTrajectory(0);
	m_axes.SyncTrajectory(1);
	m_axes.SyncTrajectory(2);
	m_axes.SyncTrajectory(3);
	m_axes.Hold(0, true);
	m_axes.Hold(1, true);
	m_axes.Hold(2, true);
	m_axes.Hold(3, true);
	m_axes.Execute();

	ptd->Clear();
    CTextContext::Push();

	CMenu::Printxyf(4, 3, 0, false, "Axis 0");
	CMenu::Printxyf(18, 3, 0, false, "Axis 1");
	CMenu::Printxyf(4, 7, 0, false, "Axis 2");
	CMenu::Printxyf(18, 7, 0, false, "Axis 3");

	while(1)
		{
	    btnState.PollKeys();
		if(btnState.KeyHit(CBtnState::B_BUTTON))
			break;

		CMenu::Printxyf(4, 4, 8, false, "%d", m_axes.GetPosition(0));
		CMenu::Printxyf(18, 4, 8, false, "%d", m_axes.GetPosition(1));
		CMenu::Printxyf(4, 8, 8, false, "%d", m_axes.GetPosition(3));
		CMenu::Printxyf(18, 8, 8, false, "%d", m_axes.GetPosition(2));
		}
    CTextContext::Pop();

	m_axes.Hold(0, false);
	m_axes.Hold(1, false);
	m_axes.Hold(2, false);
	m_axes.Hold(3, false);
	m_axes.Execute();
	}

void CBotball1Menu::Record()
	{
	CBtnState btnState;

    CTextContext::Push();
	while(1)
		{
		ptd->Clear();
		CMenu::Printxyf(5, 1, 19, false, "Press 'A' to record");
		CMenu::Printxyf(6, 2, 19, false, "Press 'B' to exit");
		while(1)
			{
		    btnState.PollKeys();
			if(btnState.KeyHit(CBtnState::A_BUTTON) || btnState.KeyHit(CBtnState::B_BUTTON))
				break;
			}
		if (btnState.KeyHit(CBtnState::B_BUTTON))
			break;

		m_axes.Record();
		CMenu::Printxyf(10, 0, 19, false, "Recording");
		CMenu::Printxyf(5, 1, 19, false, " Press 'A' to play");
		CMenu::Printxyf(4, 3, 0, false, "Axis 0");
		CMenu::Printxyf(18, 3, 0, false, "Axis 1");
		CMenu::Printxyf(4, 7, 0, false, "Axis 2");
		CMenu::Printxyf(18, 7, 0, false, "Axis 3");
		while(m_axes.Recording())
			{
		    btnState.PollKeys();
			if(btnState.KeyHit(CBtnState::A_BUTTON) || btnState.KeyHit(CBtnState::B_BUTTON))
				break;
			CMenu::Printxyf(4, 4, 8, false, "%d", m_axes.GetPosition(0));
			CMenu::Printxyf(18, 4, 8, false, "%d", m_axes.GetPosition(1));
			CMenu::Printxyf(18, 8, 8, false, "%d", m_axes.GetPosition(2));
			CMenu::Printxyf(4, 8, 8, false, "%d", m_axes.GetPosition(3));
			}
		if (btnState.KeyHit(CBtnState::B_BUTTON))
			break;

		m_axes.Play();
		CMenu::Printxyf(10, 0, 19, false, " Playing");
		CMenu::Printxyf(5, 1, 19, false, "");
		while(m_axes.Playing())
			{
		    btnState.PollKeys();
			if(btnState.KeyHit(CBtnState::A_BUTTON) || btnState.KeyHit(CBtnState::B_BUTTON))
				break;
			CMenu::Printxyf(4, 4, 8, false, "%d", m_axes.GetPosition(0));
			CMenu::Printxyf(18, 4, 8, false, "%d", m_axes.GetPosition(1));
			CMenu::Printxyf(18, 8, 8, false, "%d", m_axes.GetPosition(2));
			CMenu::Printxyf(4, 8, 8, false, "%d", m_axes.GetPosition(3));
			}
		m_axes.RStop();
		if (btnState.KeyHit(CBtnState::B_BUTTON))
			break;
		}
    CTextContext::Pop();
	}

bool CBotball1Menu::HandleMenuEvent(int eventType, CMenuElement &menu) 
	{
    switch(eventType) 
		{
		case 1000:
		DigitalValues();
		break;

		case 1001:
		AnalogValues();
		break;

		case 1002:
		BatteryVoltage();
		break;

		case 1003:
		ShowMotorPositions();
		break;

		case 1004:
		HoldMotorPositions();
		break;

		case 1005:
		Record();
		break;

		default:
		return false;
		}
    return true;
	}

