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

#ifndef _DIFFBASE_H
#define _DIFFBASE_H

#include "axesc.h"

#define TRANS_AXIS             0
#define ROTATE_AXIS            1

class CDiffBase : public CAxesClosed
	{
public:
	CDiffBase(unsigned int translationScale, unsigned int rotationScale, 
		CInterruptCont *pIntCont, unsigned long baseAddr=DAO_BEMF_BASE, unsigned char vector=16);
	virtual ~CDiffBase();

	virtual void Move(unsigned char axis,
		AxisPosition endPosition, int velocity, unsigned int acceleration);
	virtual void MoveVelocity(unsigned char axis,
		int velocity, unsigned int acceleration);
	virtual void GetPositionVector(AxisPosition position[]);
	virtual void SetPositionClosed(unsigned char axis, AxisPosition position); 
	void Periodic();
	AxisPosition GetXCoord();
	AxisPosition GetYCoord();
	void SetXCoord(AxisPosition coord);
	void SetYCoord(AxisPosition coord);
	void SetBaseEnabled(bool enabled);
	void SetTranslationScale(int translationScale);
	void SetRotationScale(int rotationScale);

private:	
	virtual void ForwardKinematics(const AxisPosition servoVal[], AxisPosition operVal[]);
	virtual void InverseKinematics(const AxisPosition operVal[], AxisPosition servoVal[]);

	AxisPosition m_XYCoords[2];
	AxisPosition m_oldPositions[4];

	int m_convNumerator[DAO_MAX_AXES];
	int m_convDenominator[DAO_MAX_AXES];

	int m_baseEnabled;

	};

#endif
