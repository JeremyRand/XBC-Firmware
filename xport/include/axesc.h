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

#ifndef _AXISC_H
#define _AXISC_H

#include "axeso.h"
#include "recaxes.h"
//#include "rcservotraj.h"

#define DAC_BIT(n)			(1<<n)

typedef unsigned short BooleanMap;	// one bit per axis

class CInterruptCont;
 
struct SAxesDBuf
	{
	BooleanMap val;
	BooleanMap buf;
	};

class CAxesClosed : public CAxesOpen, public CRecordAxes
	{
public:
	CAxesClosed(CInterruptCont *pIntCont, unsigned long baseAddr=DAO_BEMF_BASE, unsigned char vector=16, 
		unsigned char servoAxes=DAO_MAX_AXES, unsigned char operAxes=DAO_MAX_AXES, unsigned char scalePos=8, unsigned char scaleCommand=8,
		unsigned short version=2);
	virtual ~CAxesClosed();

	virtual void Periodic();
	bool Done(unsigned char axis);
	void SetPIDGains(unsigned int pGain, unsigned int iGain, unsigned int dGain);

	void Stop(unsigned char axis);
	void Hold(unsigned char axis, bool val);
	virtual void Move(unsigned char axis,
		AxisPosition endPosition, int velocity, unsigned int acceleration);
	virtual void MoveVelocity(unsigned char axis,
		int velocity, unsigned int acceleration);

	virtual void GetPositionVector(AxisPosition position[]); 
	virtual AxisPosition GetPosition(unsigned char axis);
	virtual void SetPositionClosed(unsigned char axis, AxisPosition position);

	void GetTrajectoryPositionVector(AxisPosition trajectoryPosition[]);
	void GetTrajectoryVelocityVector(int trajectoryVelocity[]);

	void SetMaxError(unsigned int error);
	void SetMaxErrorIntegral(int errorIntegral);
	void Execute();

	void SyncTrajectory(unsigned char axis);
	
//	void SetRCServoTraj(CRCServoTraj *);

protected:
	virtual void ForwardKinematics(const AxisPosition servoVal[], AxisPosition operVal[]);
	virtual void InverseKinematics(const AxisPosition operVal[], AxisPosition servoVal[]);

	void TrapezoidTrajectory();
	void PIDControl();
	void HandleStop();
	void HandleStop(unsigned char axis);

	unsigned char  m_operAxes;
	AxisPosition   m_position[DAO_MAX_AXES];	
	unsigned char  m_scalePos;

    // Trajectory input parameters
	AxisPosition m_trajectoryEndPosition[DAO_MAX_AXES];	 // enc << posScale
	int m_trajectoryVelocity[DAO_MAX_AXES];	 // enc/period << posScale
	unsigned int m_trajectoryAcceleration[DAO_MAX_AXES]; // enc/period/period
	SAxesDBuf    m_trajectory;	
	SAxesDBuf    m_velocityTrajectory;	
	SAxesDBuf    m_stop;

    // Trajectory generator output   
	int          m_generatedTrajectoryVelocity[DAO_MAX_AXES];   
	int          m_generatedTrajectoryVelocityUs[DAO_MAX_AXES];   
	long long    m_generatedTrajectoryPosition[DAO_MAX_AXES];
	AxisPosition m_generatedTrajectoryPositionUs[DAO_MAX_AXES];

	// PID controller variables   
	int           m_pGain, m_iGain, m_dGain;       
	AxisPosition  m_desiredPosition[DAO_MAX_AXES];   
	int           m_errorIntegral[DAO_MAX_AXES];    
	int           m_errorPrevious[DAO_MAX_AXES];   
	SAxesDBuf     m_hold;	
	
	int           m_pwm[DAO_MAX_AXES];

	unsigned int  m_maxErrorIntegral;
	unsigned int  m_maxError;
	unsigned char m_scaleCommand;
	int           m_accIntegral[DAO_MAX_AXES]; // enc << posScale
	unsigned char m_operMap[DAO_MAX_AXES];     // maps servo axis (index) to operational axis (value)         
	
	//CRCServoTraj *m_RCServoTraj;
	};

#endif
