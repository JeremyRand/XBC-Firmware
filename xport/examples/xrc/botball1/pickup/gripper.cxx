#include "gripper.h"
#include "textdisp.h"

CGripper::CGripper(CAxesClosed *pac, unsigned char axis)
	{
	m_pac = pac;
	m_axis = axis;
	}

void CGripper::Open()
	{
	m_pac->SetPWM(m_axis, 120);
	}

void CGripper::Release()
	{
	m_pac->SetPWM(m_axis, 0);
	}

bool CGripper::Close()
	{
	int p0, p1, lastPos=0, pos, diff;
	volatile int d;

	// grab current position
	p0 = m_pac->GetPosition(m_axis);
	
	// close gripper
	m_pac->SetPWM(m_axis, -25);
	for (d=0; d<75000; d++);

	// wait for gripper to stop moving
	while(1)
		{
		for (d=0; d<5000; d++);
		pos = m_pac->GetPosition(m_axis);
		diff = pos - lastPos;
		if (diff<0)
			diff = -diff;
		if (lastPos!=0 && diff<10)
			break;
		lastPos = pos;
		}

	// grab current position
	p1 = m_pac->GetPosition(m_axis);
	printf("Closed %d\n", p0-p1);

	// subtract and compare
	if (p0-p1<12200)
		return true;
	else 
		return false;
	}
