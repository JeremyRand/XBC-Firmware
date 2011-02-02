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

#ifndef _AXISO_H
#define _AXISO_H

#include <iinterrupt.h>
#include <intcont.h>

#define DAO_RELEASE

#define DAO_MAX_AXES				4
#define DAO_BEMF_BASE				0x9ffc400

//typedef long long AxisPosition;
typedef long AxisPosition;


class CAxesOpen : public IInterrupt
	{
public:
	CAxesOpen(CInterruptCont *pIntCont, unsigned long baseAddr=DAO_BEMF_BASE, unsigned char vector=16, unsigned char servoAxes=DAO_MAX_AXES, 
		unsigned short version=2);
	virtual ~CAxesOpen();

	void SetInterruptCont(CInterruptCont *pIntCont);

	// main functions...
	virtual AxisPosition GetPosition(unsigned char axis);             
	virtual void         GetPositionVector(AxisPosition position[]);  
	void                 SetPWM(unsigned char axis, int pwm);
	int                  GetPWM(unsigned char axis);

	// other functions...
	void         SetPosition(unsigned char axis, AxisPosition pos);
	short        GetCurrent(unsigned char axis);
	void         GetCurrentVector(short current[]);
	long	     GetVelocity(unsigned char axis);
	void         GetVelocityVector(long velocity[]);
	void         SetCalibrate(unsigned char axis, unsigned short cal);
	void         SetTiming(unsigned char waitInterval, unsigned char period);

	unsigned short GetAnalog(unsigned char channel);

protected:
	virtual void Periodic();
	// IInterrupt 
	virtual void Interrupt(unsigned char vector);

	CInterruptCont *m_pIntCont;
	unsigned char m_vector;
	unsigned char m_servoAxes;
	unsigned int m_controlFreq;

private:
	char p[130];

	};

#endif
