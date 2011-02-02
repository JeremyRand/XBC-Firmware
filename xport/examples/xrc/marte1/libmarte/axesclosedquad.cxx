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

#include "axesclosedquad.h"
#include <string.h>

CAxesClosedQuad::CAxesClosedQuad(CInterruptCont *pIntCont, unsigned short freq,
	unsigned char servoAxes, unsigned char operAxes, unsigned char scalePos, unsigned char scaleCommand) :
	CAxesOpenQuad(pIntCont, freq, servoAxes),
	CRecordAxesQuad(0x4000, servoAxes, scalePos)
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

	for (axis=0; axis<DAO_MAX_AXES; axis++)
		{
		m_pGain[axis] = 0;
		m_iGain[axis] = 0;
		m_dGain[axis] = 0;
		m_generatedTrajectoryPosition[axis] = 0;
		m_generatedTrajectoryVelocity[axis] = 0;
		m_generatedTrajectoryVelocityUs[axis] = 0;
		m_accIntegral[axis] = 0;
		m_errorIntegral[axis] = 0;
		m_trajectoryEndPosition[axis] = 0;
		}
	}

CAxesClosedQuad::~CAxesClosedQuad()
	{
	}

void CAxesClosedQuad::InverseKinematics(const AxisPosition operVal[], AxisPosition servoVal[])
	{
	unsigned char axis;

	for (axis=0; axis<m_servoAxes; axis++)
		servoVal[axis] = operVal[axis];

	m_operational = false;  // operational (virtual) axes are not used
	}

void CAxesClosedQuad::ForwardKinematics(const AxisPosition servoVal[], AxisPosition operVal[])
	{
	unsigned char axis;

	for (axis=0; axis<m_operAxes; axis++)
		operVal[axis] = servoVal[axis];

	m_operational = false;  // operational (virtual) axes are not used
	}


void CAxesClosedQuad::Move(unsigned char axis,
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

	m_generatedTrajectoryVelocity[axis] = 0;
	m_generatedTrajectoryVelocityUs[axis] = 0;
	m_accIntegral[axis] = 0;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosedQuad::MoveVelocity(unsigned char axis, int velocity, unsigned int acceleration)
	{
	m_pIntCont->Mask(m_vector);

	m_trajectoryVelocity[axis] = velocity << m_scalePos;
	m_trajectoryAcceleration[axis] = acceleration;

	m_velocityTrajectory.buf |= DAC_BIT(axis);

	// reset position trajectory bits
	m_trajectory.buf &= ~DAC_BIT(axis);

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosedQuad::SetPIDGains(unsigned int pGain, unsigned int iGain, unsigned int dGain)
	{
	unsigned char axis;

	m_pIntCont->Mask(m_vector);

	for (axis=0; axis<m_servoAxes; axis++)
		{
		m_pGain[axis] = pGain;
		m_iGain[axis] = iGain;
		m_dGain[axis] = dGain;
		}

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosedQuad::SetPIDGains(unsigned char axis, unsigned int pGain, unsigned int iGain, unsigned int dGain)
	{
	m_pIntCont->Mask(m_vector);

	m_pGain[axis] = pGain;
	m_iGain[axis] = iGain;
	m_dGain[axis] = dGain;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosedQuad::SetMaxError(unsigned int error)
	{
	m_maxError = error;
	}

void CAxesClosedQuad::SetMaxErrorIntegral(int errorIntegral)
	{
	m_maxErrorIntegral = errorIntegral;		
	}

void CAxesClosedQuad::Stop(unsigned char axis)
	{
	m_pIntCont->Mask(m_vector);

	m_stop.buf |= DAC_BIT(axis);

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosedQuad::SyncTrajectory(unsigned char axis)
	{
	AxisPosition pos[DAO_MAX_AXES];

	m_pIntCont->Mask(m_vector);

	CAxesOpenQuad::GetPositionVector(m_position);
	ForwardKinematics(m_position, pos);
	m_generatedTrajectoryPosition[axis] = pos[axis] << m_scalePos;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosedQuad::Execute()
	{
	m_pIntCont->Mask(m_vector);

	m_trajectory.val = m_trajectory.buf;
	m_velocityTrajectory.val = m_velocityTrajectory.buf;
	m_stop.val = m_stop.buf;
	m_hold.val = m_hold.buf;

	m_pIntCont->Unmask(m_vector);
	}

void CAxesClosedQuad::PIDControl()
	{
	int pwm;
	int error;
	unsigned char axis;
	
	CAxesOpenQuad::GetPositionVector(m_position);
	
	for (axis=0; axis<m_servoAxes; axis++)
		{
		if (m_operational && (m_trajectory.val || m_velocityTrajectory.val || m_hold.val) ||
			!m_operational && (m_trajectory.val&DAC_BIT(axis) || m_velocityTrajectory.val&DAC_BIT(axis) || m_hold.val&DAC_BIT(axis)) ||
			RecordServo())
			{
			error = (m_desiredPosition[axis] >> m_scalePos) - m_position[axis];
			
			pwm = m_pGain[axis]*error + m_dGain[axis]*(error - m_errorPrevious[axis]);
			if (m_iGain[axis])
				{
				pwm += m_iGain[axis]*(m_errorIntegral[axis] >> m_scalePos);
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

void CAxesClosedQuad::TrapezoidTrajectory()
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
			else if (m_generatedTrajectoryVelocity[axis]<0)
				decelerate = decelDiff>=0;// && decelDiff<=-(m_trajectoryVelocity[axis] >> (m_scalePos-1));
			}

		// if we can decelerate within good tolerance...
		if (decelerate)
			{
			// decelerate
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
		m_generatedTrajectoryPositionUs[axis] = m_generatedTrajectoryPosition[axis] >> m_scalePos;
		}
	}

void CAxesClosedQuad::HandleStop(unsigned char axis)
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

void CAxesClosedQuad::HandleStop()
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

void CAxesClosedQuad::Periodic()
	{
	if (RecordActive())
		RecordUpdate(m_position, m_desiredPosition);
	else
		{
		HandleStop();
		TrapezoidTrajectory();
		m_operational = true;  // set operational flag which may be reset by CAxesClosedQuad::InverseKinematics
		InverseKinematics(m_generatedTrajectoryPosition, m_desiredPosition);
		}
	PIDControl();
	}

bool CAxesClosedQuad::Done(unsigned char axis)
	{
	return ((m_trajectory.val&DAC_BIT(axis))==0 && (m_velocityTrajectory.val&DAC_BIT(axis))==0);
	}

void CAxesClosedQuad::GetPositionVector(AxisPosition position[])
	{
	m_pIntCont->Mask(m_vector);
	ForwardKinematics(m_position, position);
	m_pIntCont->Unmask(m_vector);	
	}

AxisPosition CAxesClosedQuad::GetPosition(unsigned char axis)
	{
	AxisPosition pos[DAO_MAX_AXES];
	GetPositionVector(pos);
	return pos[axis];
	}
	
void CAxesClosedQuad::Hold(unsigned char axis, bool val)
	{
	if (val)
		m_hold.buf |= DAC_BIT(axis);
	else
		m_hold.buf &= ~DAC_BIT(axis);
	}

void CAxesClosedQuad::GetTrajectoryPositionVector(AxisPosition trajectoryPosition[])
	{
	m_pIntCont->Mask(m_vector);
    memcpy((void *)trajectoryPosition, (void *)m_generatedTrajectoryPositionUs, sizeof(m_generatedTrajectoryPositionUs));
	m_pIntCont->Unmask(m_vector);	
	}

void CAxesClosedQuad::GetTrajectoryVelocityVector(int trajectoryVelocity[])
	{
	m_pIntCont->Mask(m_vector);
    memcpy((void *)trajectoryVelocity, (void *)m_generatedTrajectoryVelocityUs, sizeof(m_generatedTrajectoryVelocityUs));
	m_pIntCont->Unmask(m_vector);	
	}

int CAxesClosedQuad::GetGoalPosition(unsigned char axis)
{
  int pos;
  m_pIntCont->Mask(m_vector);
  pos = m_trajectoryEndPosition[axis] >> m_scalePos;
  m_pIntCont->Unmask(m_vector);	
  return pos;
}
