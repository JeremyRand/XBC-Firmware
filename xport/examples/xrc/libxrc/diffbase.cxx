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

#include "diffbase.h"
#include "textdisp.h"
#include "sinlut.h"

extern CTextDisp td;

CDiffBase::CDiffBase(unsigned int translationScale, unsigned int rotationScale, 
	CInterruptCont *pIntCont, unsigned long baseAddr, unsigned char vector) :
    CAxesClosed(pIntCont, baseAddr, vector)
	{
	m_convDenominator[TRANS_AXIS] = translationScale;   // (ticks/meter)
	m_convNumerator[TRANS_AXIS] =   1000;               // (millimeter/meter)
	m_convDenominator[ROTATE_AXIS] = rotationScale;		// (ticks/revolution)    
	m_convNumerator[ROTATE_AXIS] =   6283;				// 2*pi*1000 (milliradians/revolution
	m_convDenominator[2] = 1;  
	m_convNumerator[2] =   1;   
	m_convDenominator[3] = 1;  
	m_convNumerator[3] =   1;   

	// set operational map
	m_operMap[0] = 0x03;
	m_operMap[1] = 0x03;
	m_operMap[2] = 0x04;
	m_operMap[3] = 0x08;

	SetPIDGains(600, 0, 750);

	m_XYCoords[0] = 0;
	m_XYCoords[1] = 0;
	m_oldPositions[0] = 0;
	m_oldPositions[1] = 0;
	m_oldPositions[2] = 0;
	m_oldPositions[3] = 0;
	
	m_baseEnabled = true;
	
	}

CDiffBase::~CDiffBase()
	{
	}

void CDiffBase::Move(unsigned char axis,
	AxisPosition endPosition, int velocity, unsigned int acceleration)
	{
	
	if(m_baseEnabled)
	{
		// convert endPosition, velocity and acceleration to "ticks"   
		if (!m_trajectory.val)
		{
			SyncTrajectory(TRANS_AXIS);
			SyncTrajectory(ROTATE_AXIS);
		}
		
		endPosition  = (long long)endPosition*(long long)(m_convDenominator[axis])/(long long)(m_convNumerator[axis]);    
		velocity     = ((long long)(velocity*m_convDenominator[axis])<<m_scalePos)/(m_controlFreq*m_convNumerator[axis]);   
		acceleration = ((long long)(acceleration*m_convDenominator[axis])<<(m_scalePos+m_scalePos))/(m_controlFreq*m_controlFreq*m_convNumerator[axis]);  
		
		// execute move command         
		CAxesClosed::Move(axis, endPosition, velocity, acceleration);
	}
	else
	{
		CAxesClosed::Move(axis, endPosition, velocity, acceleration);
	}
}

void CDiffBase::MoveVelocity(unsigned char axis, int velocity, unsigned int acceleration)
{
	
	if(m_baseEnabled)
	{
		
		if (!m_velocityTrajectory.val)
		{
			SyncTrajectory(TRANS_AXIS);
			SyncTrajectory(ROTATE_AXIS);
		}
		
		velocity     = ((long long)(velocity*m_convDenominator[axis])<<m_scalePos)/(m_controlFreq*m_convNumerator[axis]);   
		acceleration = ((long long)(acceleration*m_convDenominator[axis])<<(m_scalePos+m_scalePos))/(m_controlFreq*m_controlFreq*m_convNumerator[axis]);  
		
		CAxesClosed::MoveVelocity(axis, velocity, acceleration);
		
	}
	else
	{
		CAxesClosed::MoveVelocity(axis, velocity, acceleration);
	}
	
}

void CDiffBase::GetPositionVector(AxisPosition position[])
{
	
	if(m_baseEnabled)
	{
		
		unsigned char axis;
		
		CAxesClosed::GetPositionVector(position);
		for (axis=0; axis<m_operAxes; axis++)
			position[axis] = (long long)(position[axis])*(long long)(m_convNumerator[axis])/(long long)(m_convDenominator[axis]);
	}
	else
	{
		CAxesClosed::GetPositionVector(position);
	}
	
}

void CDiffBase::SetPositionClosed(unsigned char axis, AxisPosition position)
{
	if(m_baseEnabled)
	{
		CAxesClosed::SetPositionClosed(axis, (long long)position * (long long)(m_convDenominator[axis]) / (long long)(m_convNumerator[axis]));
	}
	else
	{
		CAxesClosed::SetPositionClosed(axis, position);
	}
}


void CDiffBase::ForwardKinematics(const AxisPosition servoVal[], AxisPosition operVal[])
{  
	
	if(m_baseEnabled)
	{
		
		AxisPosition delta[4];
		
		// ">> 1" is equivalent to dividing by 2   
		operVal[0] = (servoVal[0] + servoVal[1])>>1;  // translation   
		operVal[1] = (servoVal[0] - servoVal[1])>>1;  // rotation;
		operVal[2] = servoVal[2];
		operVal[3] = servoVal[3];
		
		delta[0] = operVal[0] - m_oldPositions[0];
		delta[1] = operVal[1] - m_oldPositions[1];
		delta[2] = operVal[2] - m_oldPositions[2];
		delta[3] = operVal[3] - m_oldPositions[3];
		
		// This logic was roughly copied from dead_reckoning.ic, but was modified heavily to fit the existing diffbase code.
		m_XYCoords[0] += (delta[TRANS_AXIS]  * (SinLut((operVal[ROTATE_AXIS])*m_convNumerator[ROTATE_AXIS]/m_convDenominator[ROTATE_AXIS])-SinLut((m_oldPositions[ROTATE_AXIS])*m_convNumerator[ROTATE_AXIS]/m_convDenominator[ROTATE_AXIS]))  /  delta[ROTATE_AXIS]) >> 10;
		m_XYCoords[1] -= (delta[TRANS_AXIS]  * (CosLut((operVal[ROTATE_AXIS])*m_convNumerator[ROTATE_AXIS]/m_convDenominator[ROTATE_AXIS])-CosLut((m_oldPositions[ROTATE_AXIS])*m_convNumerator[ROTATE_AXIS]/m_convDenominator[ROTATE_AXIS]))  /  delta[ROTATE_AXIS]) >> 10;
		
		m_oldPositions[0] = operVal[0];
		m_oldPositions[1] = operVal[1];
		m_oldPositions[2] = operVal[2];
		m_oldPositions[3] = operVal[3];
		
	}
	else
	{
		CAxesClosed::ForwardKinematics(servoVal, operVal);
	}
	
}

void CDiffBase::InverseKinematics(const AxisPosition operVal[], AxisPosition servoVal[])
{     
	
	if(m_baseEnabled)
	{
		
		servoVal[0] = operVal[0] + operVal[1];  // left wheel   
		servoVal[1] = operVal[0] - operVal[1];  // right wheel
		servoVal[2] = operVal[2];
		servoVal[3] = operVal[3];
	}
	else
	{
		CAxesClosed::InverseKinematics(operVal, servoVal);
	}
	
}

void CDiffBase::Periodic()
{
	if(m_baseEnabled)
	{
		
		
		// We need a place to store a position in order to run GetPositionVector()
		AxisPosition dummy_pos[4];
		
		// This forces ForwardKinematics() and its associated position-tracking functions to run
		GetPositionVector(dummy_pos);
		
		// Now do everything else
		CAxesClosed::Periodic();
		
	}
	else
	{
		CAxesClosed::Periodic();
	}
}

AxisPosition CDiffBase::GetXCoord()
{
	if(m_baseEnabled)
	{
		return((long long)(m_XYCoords[0])*(long long)(m_convNumerator[TRANS_AXIS])/(long long)(m_convDenominator[TRANS_AXIS]));
	}
	else
	{
		return(0);
	}
}

AxisPosition CDiffBase::GetYCoord()
{
	if(m_baseEnabled)
	{
		return((long long)(m_XYCoords[1])*(long long)(m_convNumerator[TRANS_AXIS])/(long long)(m_convDenominator[TRANS_AXIS]));
	}
	else
	{
		return(0);
	}
}

void CDiffBase::SetXCoord(AxisPosition coord)
{
	if(m_baseEnabled)
	{
		m_XYCoords[0] = (long long)coord * (long long)(m_convDenominator[TRANS_AXIS]) / (long long)(m_convNumerator[TRANS_AXIS]);
	}
}

void CDiffBase::SetYCoord(AxisPosition coord)
{
	if(m_baseEnabled)
	{
		m_XYCoords[1] = (long long)coord * (long long)(m_convDenominator[TRANS_AXIS]) / (long long)(m_convNumerator[TRANS_AXIS]);
	}
}

void CDiffBase::SetBaseEnabled(bool enabled)
{
	m_baseEnabled = enabled;
	
	if(enabled)
	{
		// set operational map
		m_operMap[0] = 0x03;
		m_operMap[1] = 0x03;
		m_operMap[2] = 0x04;
		m_operMap[3] = 0x08;
	}
	else
	{
		// set operational map
		for (unsigned char axis=0; axis<DAO_MAX_AXES; axis++)
		{
			m_operMap[axis] = DAC_BIT(axis);
		}
	}
}

void CDiffBase::SetTranslationScale(int translationScale)
{
	m_convDenominator[TRANS_AXIS] = translationScale;   // (ticks/meter)
}

void CDiffBase::SetRotationScale(int rotationScale)
{
	m_convDenominator[ROTATE_AXIS] = rotationScale;		// (ticks/revolution)  
}
