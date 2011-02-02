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

#include "humrrobot1.h"
#include "textdisp.h"


CHumrRobot1::CHumrRobot1(unsigned int translationScale, unsigned int rotationScale) :
	CHumrBase(translationScale, rotationScale, 0),
	CBluetoothQueue(0) 
	{
	if (XP_REG_IDENTIFIER!=0x8c01)
		{
		printf("Incorrect logic configuration %x.\n", XP_REG_IDENTIFIER);
		while(1);
		}

	CHumrBase::SetInterruptCont(&m_intCont);
	CBluetoothQueue::SetInterruptCont(&m_intCont);

	m_menu.AddItem(ShowMotorPositions, (int)this, "Show motor positions");
	m_menu.AddItem(HoldMotorPositions, (int)this, "Hold motor positions");
	m_menu.AddItem(MoveForward, (int)this, "Move forward");
	m_menu.AddItem(TurnRight, (int)this, "Turn right");
	m_menu.AddItem(TurnLeft, (int)this, "Turn left");
	m_menu.AddItem(XirLowPower, (int)this, "Xir low power");
	m_menu.AddItem(XirHighPower, (int)this, "Xir high power");
	m_menu.AddItem(DigitalValues, (int)this, "Digital values");
	m_menu.AddItem(AnalogValues, (int)this, "Analog values");
	m_menu.AddItem(BatteryVoltage, (int)this, "Battery voltage");
	m_menu.AddItem(RecordBase, (int)this, "Record");

	if (m_menu.GetInput())
		m_menu.Execute();
	}

CHumrRobot1::~CHumrRobot1()
	{
	}

CInterruptCont *CHumrRobot1::GetInterruptCont()
	{
	return &m_intCont;
	}


void ShowMotorPositions(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	CMenu::Printxyf(4, 3, 0, false, "X-axis");
	CMenu::Printxyf(18, 3, 0, false, "Y-axis");
	CMenu::Printxyf(13, 6, 0, false, "Rotation");
	CMenu::Printxyf(4, 9, 0, false, "Wheel 0");
	CMenu::Printxyf(18, 9, 0, false, "Wheel 1");
	CMenu::Printxyf(4, 12, 0, false, "Wheel 2");
	CMenu::Printxyf(18, 12, 0, false, "Wheel 3");

	while(!CMenu::Return())
		{
		CMenu::Printxyf(4, 4, 8, false, "%d", probot->GetPosition(X_AXIS));
		CMenu::Printxyf(18, 4, 8, false, "%d", probot->GetPosition(Y_AXIS));
		CMenu::Printxyf(13, 7, 8, false, "%d", probot->GetPosition(ROTATE_AXIS));
		CMenu::Printxyf(4, 10, 8, false, "%d", probot->CAxesOpen::GetPosition(0));
		CMenu::Printxyf(18, 10, 8, false, "%d", probot->CAxesOpen::GetPosition(1));
		CMenu::Printxyf(4, 13, 8, false, "%d", probot->CAxesOpen::GetPosition(2));
		CMenu::Printxyf(18, 13, 8, false, "%d", probot->CAxesOpen::GetPosition(3));
		}
	}

void HoldMotorPositions(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	probot->SyncTrajectory(X_AXIS);
	probot->SyncTrajectory(Y_AXIS);
	probot->SyncTrajectory(ROTATE_AXIS);
	probot->Hold(0, true);
	probot->Hold(1, true);
	probot->Hold(2, true);
	probot->Hold(3, true);
	probot->Execute();

	CMenu::Printxyf(4, 6, 0, false, "0 (raw)");
	CMenu::Printxyf(18, 6, 0, false, "1 (raw)");
	CMenu::Printxyf(4, 10, 0, false, "2 (raw)");
	CMenu::Printxyf(18, 10, 0, false, "3 (raw)");
	while(!CMenu::Return())
		{
		CMenu::Printxyf(4, 7, 8, false, "%d", probot->CAxesOpen::GetPosition(0));
		CMenu::Printxyf(18, 7, 8, false, "%d", probot->CAxesOpen::GetPosition(1));
		CMenu::Printxyf(4, 11, 8, false, "%d", probot->CAxesOpen::GetPosition(2));
		CMenu::Printxyf(18, 11, 8, false, "%d", probot->CAxesOpen::GetPosition(3));
		}

	probot->Hold(0, false);
	probot->Hold(1, false);
	probot->Hold(2, false);
	probot->Hold(3, false);
	probot->Execute();
	}

void MoveForward(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	probot->Move(X_AXIS, probot->GetPosition(X_AXIS)+10000, 1800, 1800);
	probot->Execute();
	CMenu::Printxyf(11, 6, 0, false, "Translation");
	while(!probot->Done(X_AXIS))
		CMenu::Printxyf(11, 7, 8, false, "%d", probot->GetPosition(X_AXIS));
	}

void TurnRight(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	probot->Move(ROTATE_AXIS, probot->GetPosition(ROTATE_AXIS)-1571, -2000, 2000);
	probot->Execute();
	CMenu::Printxyf(11, 6, 0, false, "Rotation");
	while(!probot->Done(ROTATE_AXIS))
		CMenu::Printxyf(11, 7, 8, false, "%d", probot->GetPosition(ROTATE_AXIS));
	}

void TurnLeft(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	probot->Move(ROTATE_AXIS, probot->GetPosition(ROTATE_AXIS)+1571, 2000, 2000);
	probot->Execute();
	CMenu::Printxyf(11, 6, 0, false, "Rotation");
	while(!probot->Done(ROTATE_AXIS))
		CMenu::Printxyf(11, 7, 8, false, "%d", probot->GetPosition(ROTATE_AXIS));
	}

void MoveHighTorque(int val, int arg0, int arg1)
	{
	}

void MoveLowTorque(int val, int arg0, int arg1)
	{
	}

void XirLowPower(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	CMenu::Printxyf(6, 3, 3, true, " 1 ");
	CMenu::Printxyf(11, 3, 3, true, " 2 ");
	CMenu::Printxyf(16, 3, 3, true, " 3 ");
	CMenu::Printxyf(21, 3, 3, true, " 4 ");
	CMenu::Printxyf(6, 7, 3, true, " 5  ");
	CMenu::Printxyf(11, 7, 3, true, " 6 ");
	CMenu::Printxyf(16, 7, 3, true, " 7 ");
	CMenu::Printxyf(21, 7, 3, true, " 8 ");

	probot->SetMode(0xff);
	probot->SetXirPower(0, 0x02);
	probot->SetXirPower(1, 0x02);
	probot->SetXirRingLength(8);
	probot->SetXirAverager(2);

	while(!CMenu::Return())
		{
		CMenu::Printxyf(6, 4, 3, false, "%d", probot->GetXir(0));
		CMenu::Printxyf(11, 4, 3, false, "%d", probot->GetXir(1));
		CMenu::Printxyf(16, 4, 3, false, "%d", probot->GetXir(2));
		CMenu::Printxyf(21, 4, 3, false, "%d", probot->GetXir(3));
		CMenu::Printxyf(6, 8, 3, false, "%d", probot->GetXir(4));
		CMenu::Printxyf(11, 8, 3, false, "%d", probot->GetXir(5));
		CMenu::Printxyf(16, 8, 3, false, "%d", probot->GetXir(6));
		CMenu::Printxyf(21, 8, 3, false, "%d", probot->GetXir(7));
		}
	}

void XirHighPower(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	CMenu::Printxyf(6, 3, 3, true, " 1 ");
	CMenu::Printxyf(11, 3, 3, true, " 2 ");
	CMenu::Printxyf(16, 3, 3, true, " 3 ");
	CMenu::Printxyf(21, 3, 3, true, " 4 ");
	CMenu::Printxyf(6, 7, 3, true, " 5  ");
	CMenu::Printxyf(11, 7, 3, true, " 6 ");
	CMenu::Printxyf(16, 7, 3, true, " 7 ");
	CMenu::Printxyf(21, 7, 3, true, " 8 ");

	probot->SetMode(0xff);
	probot->SetXirPower(0, 0xff);
	probot->SetXirPower(1, 0xff);
	probot->SetXirRingLength(8);
	probot->SetXirAverager(2);

	while(!CMenu::Return())
		{
		CMenu::Printxyf(6, 4, 3, false, "%d", probot->GetXir(0));
		CMenu::Printxyf(11, 4, 3, false, "%d", probot->GetXir(1));
		CMenu::Printxyf(16, 4, 3, false, "%d", probot->GetXir(2));
		CMenu::Printxyf(21, 4, 3, false, "%d", probot->GetXir(3));
		CMenu::Printxyf(6, 8, 3, false, "%d", probot->GetXir(4));
		CMenu::Printxyf(11, 8, 3, false, "%d", probot->GetXir(5));
		CMenu::Printxyf(16, 8, 3, false, "%d", probot->GetXir(6));
		CMenu::Printxyf(21, 8, 3, false, "%d", probot->GetXir(7));
		}
	}

void DigitalValues(int val, int arg0, int arg1)
	{
	unsigned short data;
	CHumrRobot1 *probot = (CHumrRobot1 *)val;
	char *one = "1";
	char *zero = "0";
	char *temp;

	probot->SetMode(0x00);
	probot->SetGpioDataDir(0x00);

	CMenu::Printxyf(6, 3, 3, true, " 1 ");
	CMenu::Printxyf(11, 3, 3, true, " 2 ");
	CMenu::Printxyf(16, 3, 3, true, " 3 ");
	CMenu::Printxyf(21, 3, 3, true, " 4 ");
	CMenu::Printxyf(6, 7, 3, true, " 5  ");
	CMenu::Printxyf(11, 7, 3, true, " 6 ");
	CMenu::Printxyf(16, 7, 3, true, " 7 ");
	CMenu::Printxyf(21, 7, 3, true, " 8 ");

	while(!CMenu::Return())
		{
		data = probot->GetGpioData();
		temp = data&0x0002 ? one : zero;
		CMenu::Printxyf(6, 4, 1, false, temp);
		temp = data&0x0001 ? one : zero;
		CMenu::Printxyf(7, 4, 1, false, temp);
		temp = data&0x0008 ? one : zero;
		CMenu::Printxyf(11, 4, 1, false, temp);
		temp = data&0x0004 ? one : zero;
		CMenu::Printxyf(12, 4, 1, false, temp);
		temp = data&0x0020 ? one : zero;
		CMenu::Printxyf(16, 4, 1, false, temp);
		temp = data&0x0010 ? one : zero;
		CMenu::Printxyf(17, 4, 1, false, temp);
		temp = data&0x0080 ? one : zero;
		CMenu::Printxyf(21, 4, 1, false, temp);
		temp = data&0x0040 ? one : zero;
		CMenu::Printxyf(22, 4, 1, false, temp);
		temp = data&0x0200 ? one : zero;
		CMenu::Printxyf(6, 8, 1, false, temp);
		temp = data&0x0100 ? one : zero;
		CMenu::Printxyf(7, 8, 1, false, temp);
		temp = data&0x0800 ? one : zero;
		CMenu::Printxyf(11, 8, 1, false, temp);
		temp = data&0x0400 ? one : zero;
		CMenu::Printxyf(12, 8, 1, false, temp);
		temp = data&0x2000 ? one : zero;
		CMenu::Printxyf(16, 8, 1, false, temp);
		temp = data&0x1000 ? one : zero;
		CMenu::Printxyf(17, 8, 1, false, temp);
		temp = data&0x8000 ? one : zero;
		CMenu::Printxyf(21, 8, 1, false, temp);
		temp = data&0x4000 ? one : zero;
		CMenu::Printxyf(22, 8, 1, false, temp);
		}
	}

void AnalogValues(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	CMenu::Printxyf(6, 3, 3, true, " 1 ");
	CMenu::Printxyf(11, 3, 3, true, " 2 ");
	CMenu::Printxyf(16, 3, 3, true, " 3 ");
	CMenu::Printxyf(21, 3, 3, true, " 4 ");
	CMenu::Printxyf(6, 7, 3, true, " 5  ");
	CMenu::Printxyf(11, 7, 3, true, " 6 ");
	CMenu::Printxyf(16, 7, 3, true, " 7 ");
	CMenu::Printxyf(21, 7, 3, true, " 8 ");

	while(!CMenu::Return())
		{
		CMenu::Printxyf(6, 4, 3, false, "%03x", probot->GetAnalog(0));
		CMenu::Printxyf(11, 4, 3, false, "%03x", probot->GetAnalog(1));
		CMenu::Printxyf(16, 4, 3, false, "%03x", probot->GetAnalog(2));
		CMenu::Printxyf(21, 4, 3, false, "%03x", probot->GetAnalog(3));
		CMenu::Printxyf(6, 8, 3, false, "%03x", probot->GetAnalog(4));
		CMenu::Printxyf(11, 8, 3, false, "%03x", probot->GetAnalog(5));
		CMenu::Printxyf(16, 8, 3, false, "%03x", probot->GetAnalog(6));
		CMenu::Printxyf(21, 8, 3, false, "%03x", probot->GetAnalog(7));
		}
	}

void BatteryVoltage(int val, int arg0, int arg1)
	{
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	CMenu::Printxyf(8, 8, 0, false, "Battery voltage");
	while(!CMenu::Return())
		CMenu::Printxyf(13, 9, 0, false, "%3.2fV", (float)probot->GetAnalog(7)*0.00214);
	}


void RecordBase(int val, int arg0, int arg1)
	{
	char c;
	CHumrRobot1 *probot = (CHumrRobot1 *)val;

	while(1)
		{
		ptd->Clear();
		CMenu::Printxyf(5, 1, 19, false, "Press 'B' to record");
		CMenu::Printxyf(6, 2, 19, false, "Press 'A' to exit");
		while(1)
			{
			c = CMenu::GetInput();
			if (c=='A' || c=='B')
				break;
			}
		if (c=='A')
			break;

		probot->Record();
		CMenu::Printxyf(10, 0, 19, false, "Recording");
		CMenu::Printxyf(5, 1, 19, false, " Press 'B' to play");
		while(probot->Recording())
			{
			c = CMenu::GetInput();
			if (c=='A' || c=='B')
				break;
			CMenu::Printxyf(4, 6, 0, false, "Left (raw)");
			CMenu::Printxyf(18, 6, 0, false, "Right (raw)");
			CMenu::Printxyf(18, 7, 8, false, "%d", probot->CAxesOpen::GetPosition(0));
			CMenu::Printxyf(4, 7, 8, false, "%d", probot->CAxesOpen::GetPosition(1));
			}
		if (c=='A')
			break;

		probot->Play();
		CMenu::Printxyf(10, 0, 19, false, " Playing");
		CMenu::Printxyf(5, 1, 19, false, "");
		while(probot->Playing())
			{
			c = CMenu::GetInput();
			if (c=='A' || c=='B')
				break;
			CMenu::Printxyf(18, 7, 8, false, "%d", probot->CAxesOpen::GetPosition(0));
			CMenu::Printxyf(4, 7, 8, false, "%d", probot->CAxesOpen::GetPosition(1));
			}
		probot->RStop();
		if (c=='A')
			break;
		}
	}

