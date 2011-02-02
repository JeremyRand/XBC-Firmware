#if 0
#include "CTimer.h"
#include "CInterruptContSingleton.h"
#include "CPrintBuffer.h"

CTimer::CTimer() : m_secondsSinceBoot(0), m_currentTimerType(TIMER_CLOCK)
{
	InterruptContSingleton.Register(this, CInterruptContSingleton::TIMER1);
	InterruptContSingleton.Unmask(CInterruptContSingleton::TIMER1);
	StartSystemTimer();
}

CTimer::~CTimer()
{
}


/*
	Timer structs defined in RegStructs.h
	Timer 0 is the millisecond counter. It starts at 48758 and 
	counts on every clock tick from the 16hz clock, or 16778 
	ticks until overflow. So, it overflows once every 
	millisecond. On overflow to Timer 0, Timer 1 increments. 
	This gives us our millisecond aggregator. Once Timer 1 
	overflows (which it does after 1000 ticks), the second 
	aggregator will increment. How this is done is decided
	and explained in the SwitchSystemTimerType function. 
*/

void CTimer::StartSystemTimer()
{
	//g_Timers[1].clockCount = 0xFC18;
	GBA_REG_TM1D = 0xFC18;
	//g_Timers[0].clockSpan = TIMER_FREQ_1;
	BFSET(GBA_REG_TM0CNT, clockSpan, TIMER_FREQ_1);
	//g_Timers[0].intOnOverflow = 0x0;
	BFSET(GBA_REG_TM0CNT, intOnOverflow, 0x0);
	//g_Timers[0].incOnOverflow = 0x0;
	BFSET(GBA_REG_TM0CNT, incOnOverflow, 0x0);
	//g_Timers[0].enableTimer = 0x1;
	BFSET(GBA_REG_TM0CNT, enableTimer, 0x1);
	//g_Timers[1].intOnOverflow = 0x0;
	BFSET(GBA_REG_TM1CNT, intOnOverflow, 0x0);
	//g_Timers[1].incOnOverflow = 0x1;
	BFSET(GBA_REG_TM1CNT, incOnOverflow, 0x1);
	//g_Timers[1].enableTimer = 0x1;
	BFSET(GBA_REG_TM1CNT, enableTimer, 0x1);
	//g_Timers[2].intOnOverflow = 0x0;
	BFSET(GBA_REG_TM2CNT, intOnOverflow, 0x0);
	//g_Timers[2].incOnOverflow = 0x1;
	BFSET(GBA_REG_TM2CNT, incOnOverflow, 0x1);
	//g_Timers[2].enableTimer = 0x1;
	BFSET(GBA_REG_TM2CNT, enableTimer, 0x1);
	//g_Timers[2].clockCount = 0x0;
	GBA_REG_TM2D = 0x0;
	//g_Timers[0].clockCount = 0xBE76;
	GBA_REG_TM0D = 0xBE76;
}

/*
	This function switched how second aggregation is done.
	By default, GBA Timer 2 is used to count the number
	of seconds elapsed in the system. However, some 
	functions (such as playing some types of sound files)
	requires two timers to be open. Therefore, if we need
	to have two timers open for some reason, we move the 
	second count into a variable, and have GBA Timer 1 
	interrupt every time it overflows, which increments 
	our second counter.
*/
void CTimer::SwitchSystemTimerType(int iType)
{
	if(iType == m_currentTimerType) return;
	if(iType == TIMER_CLOCK)
	{
		InterruptContSingleton.UnRegister(CInterruptContSingleton::TIMER1);
		InterruptContSingleton.Mask(CInterruptContSingleton::TIMER1);

		//g_Timers[2].clockCount = m_secondsSinceBoot;
		GBA_REG_TM2D = m_secondsSinceBoot;
		//g_Timers[2].enableTimer = 0x1;
		BFSET(GBA_REG_TM2CNT, enableTimer, 0x1);
	}
	else if (iType == TIMER_INTERRUPT)
	{
		InterruptContSingleton.Register(this, CInterruptContSingleton::TIMER1);
		InterruptContSingleton.Unmask(CInterruptContSingleton::TIMER1);
		//m_secondsSinceBoot = g_Timers[2].clockCount;
		m_secondsSinceBoot = GBA_REG_TM2D;
		//g_Timers[2].enableTimer = 0x0;
		BFSET(GBA_REG_TM2CNT, enableTimer, 0x0);
	}
}

void CTimer::StopSystemTimer()
{
	//g_Timers[0].enableTimer = 0x0;
	BFSET(GBA_REG_TM0CNT, enableTimer, 0x0);
	//g_Timers[1].enableTimer = 0x0;
	BFSET(GBA_REG_TM1CNT, enableTimer, 0x0);
}
#endif
