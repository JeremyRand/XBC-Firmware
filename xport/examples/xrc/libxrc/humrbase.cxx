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

#include "humrbase.h"
#include "sinlut.h"
#include "textdisp.h"

#define ORTHOGONAL

CHumrBase::CHumrBase(unsigned int translationScale, unsigned int rotationScale, 
	CInterruptCont *pIntCont, unsigned long baseAddr, unsigned char vector) :
    CAxesClosed(pIntCont, baseAddr, vector, 4, DHB_AXES)
	{
	m_convDenominator[X_AXIS] = translationScale;   // (ticks/meter)
	m_convNumerator[X_AXIS] =   1000;               // (millimeter/meter)
	m_convDenominator[Y_AXIS] = translationScale;   // (ticks/meter)
	m_convNumerator[Y_AXIS] =   1000;               // (millimeter/meter)
	m_convDenominator[ROTATE_AXIS] = rotationScale; // (ticks/revolution)    
	m_convNumerator[ROTATE_AXIS] =   6283;          // 2*pi*1000 (milliradians/revolution

	m_frisbee = false;
	m_prevFrisbee = false;

	m_prevOperVal[X_AXIS]  = 0;
	m_prevOperVal[Y_AXIS]  = 0;
	m_prevOperVal[ROTATE_AXIS]  = 0;
	m_newOperVal[X_AXIS]  = 0;
	m_newOperVal[Y_AXIS]  = 0;
	m_newOperVal[ROTATE_AXIS]  = 0;
	m_rotationOffset = 0;

	m_rotation = 0;

	SetPIDGains(600, 0, 750);
	}

CHumrBase::~CHumrBase()
	{
	}

void CHumrBase::Move(unsigned char axis,
	AxisPosition endPosition, int velocity, unsigned int acceleration)
	{
	// convert endPosition, velocity and acceleration to "ticks"   
	endPosition  = endPosition*m_convDenominator[axis]/m_convNumerator[axis];    
	velocity     = ((long long)(velocity*m_convDenominator[axis])<<m_scalePos)/(m_controlFreq*m_convNumerator[axis]);   
	acceleration = ((long long)(acceleration*m_convDenominator[axis])<<(m_scalePos+m_scalePos))/(m_controlFreq*m_controlFreq*m_convNumerator[axis]);  

	// execute move command         
	CAxesClosed::Move(axis, endPosition, velocity, acceleration);
	}

void CHumrBase::GetPositionVector(AxisPosition position[])
	{
	unsigned char axis;

	CAxesClosed::GetPositionVector(position);
	for (axis=0; axis<m_operAxes; axis++)
		position[axis] = position[axis]*m_convNumerator[axis]/m_convDenominator[axis];
	}

AxisPosition CHumrBase::GetRotation()
	{
	AxisPosition rotation;

	rotation = (-m_position[0] - m_position[1] - m_position[2] - m_position[3])>>2;
	rotation = rotation*m_convNumerator[ROTATE_AXIS]/m_convDenominator[ROTATE_AXIS];
	return rotation;
	}

void CHumrBase::SetRotation(AxisPosition rotation)
	{
	m_rotationOffset = rotation - GetRotation();
	m_rotation = rotation;
	}

void CHumrBase::ForwardKinematics(const AxisPosition servoVal[], AxisPosition operVal[])
	{
#ifdef ORTHOGONAL
	operVal[X_AXIS] = ( servoVal[1] - servoVal[3])>>1;
	operVal[Y_AXIS] = (-servoVal[0] + servoVal[2])>>1;
	operVal[ROTATE_AXIS] = (-servoVal[0] - servoVal[1] - servoVal[2] - servoVal[3])>>2;
#else
	operVal[X_AXIS] = ( servoVal[0] + servoVal[1] - servoVal[2] - servoVal[3])>>2;
	operVal[Y_AXIS] = (-servoVal[0] + servoVal[1] + servoVal[2] - servoVal[3])>>2;
	operVal[ROTATE_AXIS] = (-servoVal[0] - servoVal[1] - servoVal[2] - servoVal[3])>>2;
#endif
	}

void CHumrBase::InverseKinematics(const AxisPosition operVal[], AxisPosition servoVal[])
	{
	int cosRotation, sinRotation;   
	AxisPosition diffRotOperVal[2];  
	AxisPosition diffOperVal[2];	

	if (m_frisbee && !m_prevFrisbee)
		m_rotationOffset = m_rotation - GetRotation();

	if (m_frisbee)   
		m_rotation = GetRotation() + m_rotationOffset; // get rotation angle

	cosRotation = CosLut(m_rotation);
	sinRotation = SinLut(m_rotation);

	// calculate velocity (difference)
	diffOperVal[X_AXIS] = operVal[X_AXIS] - m_prevOperVal[X_AXIS];
	diffOperVal[Y_AXIS] = operVal[Y_AXIS] - m_prevOperVal[Y_AXIS];

	// rotate x and y axes     
	diffRotOperVal[X_AXIS] = diffOperVal[X_AXIS]*cosRotation + diffOperVal[Y_AXIS]*sinRotation;
	diffRotOperVal[Y_AXIS] = -diffOperVal[X_AXIS]*sinRotation + diffOperVal[Y_AXIS]*cosRotation;   
	
	// scale back down      
	diffRotOperVal[X_AXIS] >>= DSL_SCALE;      
	diffRotOperVal[Y_AXIS] >>= DSL_SCALE;   

	// Add to new position
	m_newOperVal[X_AXIS] += diffRotOperVal[X_AXIS];
	m_newOperVal[Y_AXIS] += diffRotOperVal[Y_AXIS];

	// save for next iterration
	m_prevOperVal[X_AXIS] = operVal[X_AXIS];
	m_prevOperVal[Y_AXIS] = operVal[Y_AXIS];
	m_prevFrisbee = m_frisbee;

#ifdef ORTHOGONAL
	servoVal[0] = -m_newOperVal[Y_AXIS] - operVal[ROTATE_AXIS];
	servoVal[1] = m_newOperVal[X_AXIS] - operVal[ROTATE_AXIS];
	servoVal[2] = m_newOperVal[Y_AXIS] - operVal[ROTATE_AXIS];
	servoVal[3] = -m_newOperVal[X_AXIS] - operVal[ROTATE_AXIS];
#else
	servoVal[0] =  m_newOperVal[X_AXIS] - m_newOperVal[Y_AXIS] - operVal[ROTATE_AXIS];
	servoVal[1] =  m_newOperVal[X_AXIS] + m_newOperVal[Y_AXIS] - operVal[ROTATE_AXIS];
	servoVal[2] = -m_newOperVal[X_AXIS] + m_newOperVal[Y_AXIS] - operVal[ROTATE_AXIS];
	servoVal[3] = -m_newOperVal[X_AXIS] - m_newOperVal[Y_AXIS] - operVal[ROTATE_AXIS];
#endif
	}

