#include "timer.h"
#include "gba.h"

CTimer::CTimer(CInterruptCont *pIntCont, unsigned short freq)
	{
	m_pIntCont = pIntCont;
	m_vector = 4; // 4 is the vector for TIMER1
	m_pIntCont->Register(this, m_vector);  

	SetFreq(freq);
	}

CTimer::~CTimer()
	{
	Disable();
	m_pIntCont->UnRegister(m_vector); 
	}

void CTimer::SetFreq(unsigned short freq)
	{
	unsigned int period;

	m_freq = freq;
	period = DT_FREQ/m_freq;

	GBA_REG_TM1D = period>0xffff ? 0 : 0xffff-period;
	}

unsigned short CTimer::GetFreq()
	{
	unsigned int period = DT_FREQ/m_freq;
	return (unsigned short)(DT_FREQ/period); 
	}

void CTimer::Enable()
	{
	GBA_REG_TM1CNT = 0x00c1;
	m_pIntCont->Unmask(m_vector);
	}

void CTimer::Disable()
	{
	GBA_REG_TM1CNT = 0x00000;
	m_pIntCont->Mask(m_vector);
	}

void CTimer::Interrupt(unsigned char vector)
	{
	}

