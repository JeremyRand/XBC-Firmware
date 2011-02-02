#ifndef _AXESOPENQUAD_H
#define _AXESOPENQUAD_H

#include "timer.h"

#define DAO_MAX_PWM                 0xff
#define DAO_MAX_AXES				16

//typedef long long AxisPosition;
typedef long AxisPosition;

class CAxesOpenQuad : public CTimer
	{
public:
	CAxesOpenQuad(CInterruptCont *pIntCont, unsigned short freq, unsigned char servoAxes);
	virtual ~CAxesOpenQuad();

	virtual AxisPosition GetPosition(unsigned char axis);             
	virtual void         GetPositionVector(AxisPosition position[]);  
	void                 SetPWM(unsigned char axis, int pwm);
	int                  GetPWM(unsigned char axis);

	// other functions...
	void         SetPosition(unsigned char axis, AxisPosition pos);

protected:
	virtual void Periodic();
	// IInterrupt 
	virtual void Interrupt(unsigned char vector);

	char m_encPrev[DAO_MAX_AXES];
	AxisPosition m_position[DAO_MAX_AXES];
	int m_pwm[DAO_MAX_AXES];
	unsigned char m_servoAxes;
	int m_lut[256];
	};

#endif
