#ifndef _TIMER_H
#define _TIMER_H

#include "iinterrupt.h"
#include "intcont.h"

#define DT_FREQ		262192L

class CTimer : public IInterrupt
{
public:
	CTimer(CInterruptCont *pIntCont, unsigned short freq);
	virtual ~CTimer();

	void SetFreq(unsigned short freq);
	unsigned short GetFreq();

	void Enable();
	void Disable();

	virtual void Interrupt(unsigned char vector);

protected:
	unsigned short m_freq;
	CInterruptCont *m_pIntCont;
	unsigned char m_vector;
	};


#endif
