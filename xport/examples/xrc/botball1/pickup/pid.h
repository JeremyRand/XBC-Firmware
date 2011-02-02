#ifndef _PID_H
#define _PID_H

class Cpid
	{
public:
	Cpid(int pGain, int iGain, int dGain, unsigned char scale=8);
	
	int Compensate(int error);

private:
	int m_pGain, m_iGain, m_dGain;
	int m_prevError;
	int m_integral;
	unsigned char m_scale;
	};

#endif
