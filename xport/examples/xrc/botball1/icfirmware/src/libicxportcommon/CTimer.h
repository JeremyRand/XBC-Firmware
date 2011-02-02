#ifndef CTIMER_H
#define CTIMER_H

#error "CTimer.h needs to be converted to not use regstructs.h definitions"
#include "singleton.h"
#include "iinterrupt.h"
#include "regbits.h"

#define TimerSingleton CTimer::Instance()

class CTimer : public CSingleton<CTimer>, public IInterrupt
{
	unsigned int m_secondsSinceBoot;
	unsigned short m_currentTimerType;
public:
	enum TimerFrequency
	{
		TIMER_FREQ_1 = 0,
		TIMER_FREQ_64 = 1,
		TIMER_FREQ_128 = 2,
		TIMER_FREQ_1024 = 3
	};
	enum TimerType {
		TIMER_CLOCK = 0,
		TIMER_INTERRUPT
	};
	CTimer();
	virtual ~CTimer();
	void StartSystemTimer();
	void StopSystemTimer();
	void SwitchSystemTimerType(int iType);
	virtual void Interrupt(unsigned char vector) { ++m_secondsSinceBoot; }
	/*
		The math to return the seconds count is as follows:
		(second_count)*1000+(millisecond_count)
		However, to avoid the multiplication, it comes out
		as this in the shift operations:
		((second_count*1024)-(second_count*16)-(second_count*8))+(millisecond_count)
		or
		((second_count<<10)-(second_count<<4)-(second_count<<3))+(millisecond_count)
	*/
	unsigned int GetMillisecondsSinceBoot()
	{
		if(m_currentTimerType == TIMER_CLOCK)
		{
			//return ((g_Timers[2].clockCount << 10) - (g_Timers[2].clockCount << 4) - (g_Timers[2].clockCount << 3)) + (g_Timers[1].clockCount - 0xFC18);
			return ((GBA_REG_TM2D << 10) - (GBA_REG_TM2D << 4) - (GBA_REG_TM2D << 3)) + (GBA_REG_TM1D - 0xFC18);
		}
		//return ((m_secondsSinceBoot << 10) - (m_secondsSinceBoot << 4) - (m_secondsSinceBoot << 3)) + (g_Timers[1].clockCount - 0xFC18);
		return ((m_secondsSinceBoot << 10) - (m_secondsSinceBoot << 4) - (m_secondsSinceBoot << 3)) + (GBA_REG_TM1D - 0xFC18);
	}
};

#endif
