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

#ifndef _HUMRROBOT1_H
#define _HUMRROBOT1_H

#include "humrbase.h"
#include "xirgpio.h"
#include "btqueue.h"
#include "intcont.h"
#include "menu.h"

class CHumrRobot1 : public CHumrBase, public CXirGpio, public CBluetoothQueue
	{
public:
	CHumrRobot1(unsigned int translationScale, unsigned int rotationScale);
	virtual ~CHumrRobot1();

	CInterruptCont *GetInterruptCont();

private:

	CInterruptCont m_intCont;
	CMenu m_menu;
	};

void ShowMotorPositions(int val, int arg0, int arg1);
void HoldMotorPositions(int val, int arg0, int arg1);
void MoveForward(int val, int arg0, int arg1);
void TurnRight(int val, int arg0, int arg1);
void TurnLeft(int val, int arg0, int arg1);
void XirLowPower(int val, int arg0, int arg1);
void XirHighPower(int val, int arg0, int arg1);
void DigitalValues(int val, int arg0, int arg1);
void AnalogValues(int val, int arg0, int arg1);
void BatteryVoltage(int val, int arg0, int arg1);
void RecordBase(int val, int arg0, int arg1);

#endif
