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

#include "axesc.h"
#include <string.h>

CAxesClosed::CAxesClosed(CInterruptCont *pIntCont, unsigned long baseAddr, unsigned char vector,
	unsigned char servoAxes, unsigned char operAxes, unsigned char scalePos, unsigned char scaleCommand,
	unsigned short version) :
	CAxesOpen(pIntCont, baseAddr, vector, servoAxes, version),
	CRecordAxes(0x4000, servoAxes)
	{
	unsigned char axis;

	m_operAxes = operAxes;

	m_scaleCommand = scaleCommand;
	m_scalePos = scalePos;
	m_maxErrorIntegral = 0x7fffffff;

	m_hold.val = 0;
	m_hold.buf = 0;
	m_trajectory.val = 0;
	m_trajectory.buf = 0;
	m_velocityTrajectory.val = 0;
	m_velocityTrajectory.buf = 0;
	m_stop.val = 0;
	m_stop.buf = 0;
	m_pGain = 0;
	m_iGain = 0;
	m_dGain = 0;

	for (axis=0; axis<DAO_MAX_AXES; axis++)
		{
		m_generatedTrajectoryPosition[axis] = 0;
		m_generatedTrajectoryVelocity[axis] = 0;
		m_generatedTrajectoryVelocityUs[axis] = 0;
		m_accIntegral[axis] = 0;
		m_errorIntegral[axis] = 0;
		m_pwm[axis] = 0;
		m_trajectoryEndPosition[axis] = 0;
		m_operMap[axis] = DAC_BIT(axis);
		}
		
//	m_RCServoTraj = 0;
	}

CAxesClosed::~CAxesClosed()
	{
	}

void CAxesClosed::InverseKinematics(const AxisPosition operVal[], AxisPosition servoVal[])
	{
	unsigned char axis;

	for (axis=0; axis<m_servoAxes; axis++)
		servoVal[axis] = operVal[axis];
	}

void CAxesClosed::ForwardKinematics(const AxisPosition servoVal[], AxisPosition operVal[])
	{
	unsigned char axis;

	for (axis=0; axis<m_operAxes; axis++)
		operVal[axis] = servoVal[axis];
	}


void CAxesClosed::Move(unsigned char axis,
	AxisPosition endPosition, int velocity, unsigned int acceleration)
	{
	m_pIntCont->Mask(m_vector);

	// abs(velocity)
	if (velocity<0)
		velocity = -velocity;

	m_trajectoryEndPosition[axis] = endPosition << m_scalePos;
	if (m_trajectoryEndPosition[axis]>m_generatedTrajectoryPosition[axis])
		m_trajectoryVelocity[axis] = velocity << m_scalePos;
	else
		m_trajectoryVelocity[axis] = -(velocity << m_scalePos);

	m_trajectoryAcceleration[axis] = acceleration;

	m_trajectory.buf |= DAC_BIT(axis);

	// reset velocity trajectory bits
	m_velocityTrajectory.buf &= ~DAC_BIT(axis);
	// reset stop, in case it's been issued
	m_stop.val &= ~DAC_BIT(axis);
	m_stop.buf &= ~DAC_BIT(axis);

	m_generatedTrajectoryVelocity[axis] = 0;
	m_generatedTrajectoryVelocityUs[axis] = 0;
	m_accIntegral[axis] = 0;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosed::MoveVelocity(unsigned char axis, int velocity, unsigned int acceleration)
	{
	m_pIntCont->Mask(m_vector);

	m_trajectoryVelocity[axis] = velocity << m_scalePos;
	m_trajectoryAcceleration[axis] = acceleration;

	m_velocityTrajectory.buf |= DAC_BIT(axis);

	// reset position trajectory bits
	m_trajectory.buf &= ~DAC_BIT(axis);
	// reset stop, in case it's been issued
	m_stop.val &= ~DAC_BIT(axis);
	m_stop.buf &= ~DAC_BIT(axis);

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosed::SetPIDGains(unsigned int pGain, unsigned int iGain, unsigned int dGain)
	{
	m_pIntCont->Mask(m_vector);

	m_pGain = pGain;
	m_iGain = iGain;
	m_dGain = dGain;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosed::SetMaxError(unsigned int error)
	{
	m_maxError = error;
	}

void CAxesClosed::SetMaxErrorIntegral(int errorIntegral)
	{
	m_maxErrorIntegral = errorIntegral;		
	}

void CAxesClosed::Stop(unsigned char axis)
	{
	m_pIntCont->Mask(m_vector);

	m_stop.buf |= DAC_BIT(axis);

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosed::SyncTrajectory(unsigned char axis)
	{
	AxisPosition pos[DAO_MAX_AXES];

	m_pIntCont->Mask(m_vector);

	CAxesOpen::GetPositionVector(m_position);
	ForwardKinematics(m_position, pos);
	m_generatedTrajectoryPosition[axis] = (long long)pos[axis] << m_scalePos;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosed::Execute()
	{
	m_pIntCont->Mask(m_vector);

	m_trajectory.val = m_trajectory.buf;
	m_velocityTrajectory.val = m_velocityTrajectory.buf;
	m_stop.val = m_stop.buf;
	m_hold.val = m_hold.buf;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosed::PIDControl()
	{
	int pwm;
	int error;
	unsigned char axis;
	
	CAxesOpen::GetPositionVector(m_position);
	
	for (axis=0; axis<m_servoAxes; axis++)
		{
		if ((m_trajectory.val|m_velocityTrajectory.val|m_hold.val)&m_operMap[axis] ||
			RecordServo())
			{
			error = m_desiredPosition[axis] - m_position[axis];
			
			pwm = m_pGain*error + m_dGain*(error - m_errorPrevious[axis]);
			if (m_iGain)
				{
				pwm += m_iGain*(m_errorIntegral[axis] >> m_scalePos);
				m_errorIntegral[axis] += error;
				if (m_errorIntegral[axis]>(int)m_maxErrorIntegral)
					m_errorIntegral[axis] = m_maxErrorIntegral;
				else if (m_errorIntegral[axis]<-(int)m_maxErrorIntegral)
					m_errorIntegral[axis] = -m_maxErrorIntegral;
				}
			
			m_errorPrevious[axis] = error;
			
			m_pwm[axis] = pwm >> m_scaleCommand;
			SetPWM(axis, m_pwm[axis]);
			}
		else if (m_pwm[axis])
			{
			m_pwm[axis] = 0;
			SetPWM(axis, 0);
			}
		}
	}

void CAxesClosed::TrapezoidTrajectory()
	{
	AxisPosition decelDiff;
	bool decelerate;
	bool finish;
	unsigned char axis;

	for (axis=0; axis<m_operAxes; axis++)
		{
		decelerate = false;
		finish = false;

		if (!(m_trajectory.val&DAC_BIT(axis)) && !(m_velocityTrajectory.val&DAC_BIT(axis)))
			continue;

		if (m_trajectory.val&DAC_BIT(axis))
			{
			decelDiff = m_trajectoryEndPosition[axis]-m_generatedTrajectoryPosition[axis]-m_accIntegral[axis];
		
			if (m_generatedTrajectoryVelocity[axis]>0)
				decelerate = decelDiff<=0;// && decelDiff>=-(m_trajectoryVelocity[axis] >> (m_scalePos-1));
				// Note by Jeremy 2007 10 14: should relace these 0's with (vf)^2/(2*a); not sure about sign and other minor things
			else if (m_generatedTrajectoryVelocity[axis]<0)
				decelerate = decelDiff>=0;// && decelDiff<=-(m_trajectoryVelocity[axis] >> (m_scalePos-1));
			}

		// if we can decelerate within good tolerance...
		if (decelerate)
			{
			// decelerate
			// Note by Jeremy 2007 10 14: m_generatedTrajectoryVelocity[axis] may need to be
			//                            m_generatedTrajectoryVelocity[axis] - vf
			if (m_generatedTrajectoryVelocity[axis]>0)
				{
				if (m_generatedTrajectoryVelocity[axis]>=(int)(m_trajectoryAcceleration[axis]<<1))
					m_generatedTrajectoryVelocity[axis] -= m_trajectoryAcceleration[axis];
				finish = m_generatedTrajectoryPosition[axis]>=(int)m_trajectoryEndPosition[axis]; 
				}
			else 
				{
				if (m_generatedTrajectoryVelocity[axis]<=-(int)(m_trajectoryAcceleration[axis]<<1))
					m_generatedTrajectoryVelocity[axis] += m_trajectoryAcceleration[axis];
				finish = m_generatedTrajectoryPosition[axis]<=(int)m_trajectoryEndPosition[axis];
				}
			
			m_generatedTrajectoryVelocityUs[axis] = m_generatedTrajectoryVelocity[axis] >> m_scalePos;
			m_accIntegral[axis] -= m_generatedTrajectoryVelocityUs[axis];
			// if our position error is within tolerance, we've arrived 
			if (finish)
				{
				m_generatedTrajectoryPosition[axis] = m_trajectoryEndPosition[axis];
				// Note by Jeremy 2007 10 14: This HandleStop should probably be MoveVelocity
				HandleStop(axis);
				continue;
				}
			}
		else if (m_generatedTrajectoryVelocity[axis]<m_trajectoryVelocity[axis])
			{
			m_accIntegral[axis] += m_generatedTrajectoryVelocityUs[axis];
			m_generatedTrajectoryVelocity[axis] += m_trajectoryAcceleration[axis];
			if (m_generatedTrajectoryVelocity[axis]>m_trajectoryVelocity[axis])
				m_generatedTrajectoryVelocity[axis] = m_trajectoryVelocity[axis];
			m_generatedTrajectoryVelocityUs[axis] = m_generatedTrajectoryVelocity[axis] >> m_scalePos;
			}
		else if (m_generatedTrajectoryVelocity[axis]>m_trajectoryVelocity[axis])
			{
			m_accIntegral[axis] += m_generatedTrajectoryVelocityUs[axis];
			m_generatedTrajectoryVelocity[axis] -= m_trajectoryAcceleration[axis];
			if (m_generatedTrajectoryVelocity[axis]<m_trajectoryVelocity[axis])
				m_generatedTrajectoryVelocity[axis] = m_trajectoryVelocity[axis];
			m_generatedTrajectoryVelocityUs[axis] = m_generatedTrajectoryVelocity[axis] >> m_scalePos;
			}
		// else do nothing (hold current velocity)

		// velocity stop = done
		if (m_velocityTrajectory.val&DAC_BIT(axis) && m_trajectoryVelocity[axis]==0 && m_generatedTrajectoryVelocity[axis]==0)
			{
			HandleStop(axis);
			continue;
			}

		m_generatedTrajectoryPosition[axis] += m_generatedTrajectoryVelocityUs[axis];
		m_generatedTrajectoryPositionUs[axis] = (AxisPosition)(m_generatedTrajectoryPosition[axis] >> m_scalePos);
		}
	}

void CAxesClosed::HandleStop(unsigned char axis)
	{
	m_generatedTrajectoryVelocity[axis] = 0;
	m_generatedTrajectoryVelocityUs[axis] = 0;
	m_accIntegral[axis] = 0;
	
	m_stop.val &= ~DAC_BIT(axis);
	m_stop.buf &= ~DAC_BIT(axis);
	m_trajectory.val &= ~DAC_BIT(axis);
	m_trajectory.buf &= ~DAC_BIT(axis);
	m_velocityTrajectory.val &= ~DAC_BIT(axis);
	m_velocityTrajectory.buf &= ~DAC_BIT(axis);
	}

void CAxesClosed::HandleStop()
	{
	unsigned char axis;

	// handle stop
	if (m_stop.val)
		{
		for (axis=0; axis<m_operAxes; axis++)
			{
			if (m_stop.val&DAC_BIT(axis))
				{
				HandleStop(axis);
				SyncTrajectory(axis);
				}
			}
		}
	}

void CAxesClosed::Periodic()
{
	if (RecordActive())
		RecordUpdate(m_position, m_desiredPosition);
	else
		{
		HandleStop();
		TrapezoidTrajectory();
		InverseKinematics(m_generatedTrajectoryPositionUs, m_desiredPosition);
		}
	PIDControl();
		
/*	if(m_RCServoTraj)
	{
		m_RCServoTraj->ServoTrajectory();
	}*/
	
}

bool CAxesClosed::Done(unsigned char axis)
	{
	return ((m_trajectory.val&DAC_BIT(axis))==0 && (m_velocityTrajectory.val&DAC_BIT(axis))==0);
	}

void CAxesClosed::GetPositionVector(AxisPosition position[])
	{
	m_pIntCont->Mask(m_vector);
	ForwardKinematics(m_position, position);
	m_pIntCont->Unmask(m_vector);	
	}

AxisPosition CAxesClosed::GetPosition(unsigned char axis)
	{
	AxisPosition pos[DAO_MAX_AXES];
	GetPositionVector(pos);
	return pos[axis];
	}

void CAxesClosed::SetPositionClosed(unsigned char axis, AxisPosition position)
{
	AxisPosition operPositions[m_operAxes];
	AxisPosition servoPositions[m_servoAxes];
	
	GetPositionVector(operPositions);
	
	operPositions[axis] = position;
	
	InverseKinematics(operPositions, servoPositions);
	
	for(unsigned char axis_count = 0; axis_count<m_servoAxes; axis_count++)
	{
		SetPosition(axis_count, servoPositions[axis_count]);
	}
}
	
void CAxesClosed::Hold(unsigned char axis, bool val)
	{
	if (val)
		m_hold.buf |= DAC_BIT(axis);
	else
		m_hold.buf &= ~DAC_BIT(axis);
	}

void CAxesClosed::GetTrajectoryPositionVector(AxisPosition trajectoryPosition[])
	{
	m_pIntCont->Mask(m_vector);
    memcpy((void *)trajectoryPosition, (void *)m_generatedTrajectoryPositionUs, sizeof(m_generatedTrajectoryPositionUs));
	m_pIntCont->Unmask(m_vector);	
	}

void CAxesClosed::GetTrajectoryVelocityVector(int trajectoryVelocity[])
	{
	m_pIntCont->Mask(m_vector);
    memcpy((void *)trajectoryVelocity, (void *)m_generatedTrajectoryVelocityUs, sizeof(m_generatedTrajectoryVelocityUs));
	m_pIntCont->Unmask(m_vector);	
	}


/*
void CAxesClosed::SetRCServoTraj(CRCServoTraj *controller)
{
	m_RCServoTraj = controller;
}*/