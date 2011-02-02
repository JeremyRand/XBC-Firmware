#include "intcont.h"
#include "axesopenquad.h"
#include "../logic/marte1.h"

CAxesOpenQuad::CAxesOpenQuad(CInterruptCont *pIntCont, unsigned short freq, unsigned char servoAxes) :
	CTimer(pIntCont, freq)
	{
	int i;

	Enable(); // enable timer
	m_servoAxes = servoAxes;

	for (i=0; i<128; i++)
		m_lut[i] = i;

	for (i=128; i<256; i++)
		m_lut[i] = i - 256;

	for (i=0; i<m_servoAxes; i++)
		{
		m_encPrev[i] = 0;
		m_position[i] = 0;
		m_pwm[i] = 0;
		}
	}

CAxesOpenQuad::~CAxesOpenQuad()
	{
	}

AxisPosition CAxesOpenQuad::GetPosition(unsigned char axis)             
	{
	return m_position[axis];
	}

void CAxesOpenQuad::GetPositionVector(AxisPosition position[])  
	{
	unsigned char axis;

	m_pIntCont->Mask(m_vector);
	for (axis=0; axis<m_servoAxes; axis++)
		position[axis] = m_position[axis];
	m_pIntCont->Unmask(m_vector);
	}

void CAxesOpenQuad::SetPWM(unsigned char axis, int pwm)
	{
	if (pwm<0)
		{
		pwm = -pwm;
		DIR_REG |= 1<<axis;
		}
	else
		DIR_REG &= ~(1<<axis);

	if (pwm>DAO_MAX_PWM)
		pwm = DAO_MAX_PWM;
	PWM_REG(axis) = pwm;

	m_pwm[axis] = pwm;
	}

int CAxesOpenQuad::GetPWM(unsigned char axis)
	{
	return m_pwm[axis];
	}

void CAxesOpenQuad::SetPosition(unsigned char axis, AxisPosition pos)
	{
	m_pIntCont->Mask(m_vector);
	m_position[axis] = pos;
	m_pIntCont->Unmask(m_vector);
	}

void CAxesOpenQuad::Periodic()
	{
	}

void CAxesOpenQuad::Interrupt(unsigned char vector)
	{
	unsigned char axis;
	char enc0, enc1, diff;

	for (axis=0; axis<m_servoAxes; axis++)
		{
		// there's no synchronizer, so do this instead
		while(1)
			{
			enc0 = (char)QUADRATURE_REG(axis);
			enc1 = (char)QUADRATURE_REG(axis);
			if (enc0==enc1)
				break;
			}
		diff = enc0 - m_encPrev[axis];
		m_position[axis] += m_lut[(unsigned char)diff];

		m_encPrev[axis] = enc0;
		}

	Periodic();
	}
