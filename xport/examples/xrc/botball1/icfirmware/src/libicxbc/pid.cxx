#include "pid.h"

Cpid::Cpid(int pGain, int iGain, int dGain, unsigned char scale) 
	{
	m_pGain = pGain;
	m_iGain = iGain;
	m_dGain = dGain;
	m_scale = scale;
	m_prevError = 0;
	m_integral = 0;
	}

int Cpid::Compensate(int error)
	{
	int result;

	m_integral += error;

	result = (m_pGain*error + m_iGain*m_integral + m_dGain*(error-m_prevError))>>m_scale;

	m_prevError = error;

	return result;
	}
