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

#include <textdisp.h>
#include <axesc.h>

#define MINPWM 20

CTextDisp td(TDM_LCD);

class CHaptic : public CAxesClosed
	{
public:
	CHaptic();
	virtual void Periodic();

private:
	CInterruptCont m_intCont;
	};

CHaptic::CHaptic() : CAxesClosed(0, DAO_BEMF_BASE, 16, 2, 2)
	{
	// Must set interrupt controller *after* all objects have been created,
	// which is now...
	SetInterruptCont(&m_intCont);

	// Set gains -- high P gains cause oscillations
	SetPIDGains(300, 0, 300);

	// Hold axis 1 (slave) position
	Hold(1, true);
	Execute();
	}

// Note, this is an interrupt routine -- do not insert printf calls, etc
void CHaptic::Periodic()
	{
	int pwm1;

	// Set axis 1 (slave) position to axis 0 (master) position
	// and scale position by position scale
	m_desiredPosition[1] = m_position[0] << m_scalePos;

	// Control axis 1 (slave) with PID compensator
	PIDControl();

	// Reflect PWM to axis 0...
	pwm1 = GetPWM(1);
	// ...and prevent small oscillations by zeroing small PWM values 
	if (pwm1>MINPWM || pwm1<-MINPWM)
		SetPWM(0, -pwm1);
	else 
		SetPWM(0, 0);
	}

int main(void)
	{
	CHaptic haptic;

	// Print positions
	while(1)
		printf("%d %d\n", haptic.GetPosition(0), haptic.GetPosition(1));
	}
