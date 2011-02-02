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

#ifndef _HUMRBASE_H
#define _HUMRBASE_H

#include "axesc.h"

#define DHB_AXES	           3

#define X_AXIS                 0
#define Y_AXIS                 1
#define ROTATE_AXIS            2

class CHumrBase : public CAxesClosed
	{
public:
	CHumrBase(unsigned int translationScale, unsigned int rotationScale, 
		CInterruptCont *pIntCont, unsigned long baseAddr=DAO_BEMF_BASE, unsigned char vector=16);
	virtual ~CHumrBase();

	virtual void Move(unsigned char axis,
		AxisPosition endPosition, int velocity, unsigned int acceleration);
	virtual void GetPositionVector(AxisPosition position[]); 
	AxisPosition GetRotation();
	void SetRotation(AxisPosition rotation);
	bool GetFrisbee()
		{
		return m_frisbee;
		}
	void SetFrisbee(bool val)
		{
		m_frisbee = val;
		}

private:	
	virtual void ForwardKinematics(const AxisPosition servoVal[], AxisPosition operVal[]);
	virtual void InverseKinematics(const AxisPosition operVal[], AxisPosition servoVal[]);

	bool m_frisbee;
	bool m_prevFrisbee;

	int m_convNumerator[DHB_AXES];
	int m_convDenominator[DHB_AXES];
	AxisPosition m_prevOperVal[DHB_AXES];
	AxisPosition m_newOperVal[DHB_AXES];
	AxisPosition m_rotationOffset;
	AxisPosition m_rotation;

	};

#endif
