#include "simptimer.h"

unsigned short CSimpTimer::m_currentTimerType = TIMER_FPGA;
unsigned int CSimpTimer::m_secondsSinceBoot = 0;

CSimpTimer::CSimpTimer(unsigned long base)
{
	m_base = base;
	//m_currentTimerType = TIMER_FPGA;
}

void CSimpTimer::GetCount(unsigned long *count)
{
	if(m_currentTimerType == TIMER_FPGA)
	{
		*count = *(volatile unsigned long *)m_base;
	}
	else
	{
		*count = (unsigned long) (GetMillisecondsSinceBoot() * 1000 + ((0xFFFF-GBA_REG_TM0D)>>4));
	}
}

void CSimpTimer::GetCount(unsigned long long *count)
{
	if(m_currentTimerType == TIMER_FPGA)
	{
		volatile unsigned long *lcount = (volatile unsigned long *)count;
		volatile unsigned short *scount = (volatile unsigned short *)count + 2;
		lcount[0] = *(volatile unsigned long *)m_base;
		scount[0] = *((volatile unsigned short *)m_base + 2);
		scount[1] = 0; // set upper 16 bits to 0
	}
	else
	{
		volatile unsigned long *lcount = (volatile unsigned long *)count;
		volatile unsigned short *scount = (volatile unsigned short *)count + 2;
		lcount[0] = (unsigned long) (GetMillisecondsSinceBoot() * 1000 + ((0xFFFF-GBA_REG_TM0D)>>4));
		scount[0] = 0;
		scount[1] = 0; // set upper 16 bits to 0
	}
}

void CSimpTimer::StartSystemTimer()
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
	
	m_currentTimerType = TIMER_CLOCK;
}

void CSimpTimer::StopSystemTimer()
{
	//g_Timers[0].enableTimer = 0x0;
	BFSET(GBA_REG_TM0CNT, enableTimer, 0x0);
	//g_Timers[1].enableTimer = 0x0;
	BFSET(GBA_REG_TM1CNT, enableTimer, 0x0);
	
	m_currentTimerType = TIMER_FPGA;
}

unsigned int CSimpTimer::GetMillisecondsSinceBoot()
{
	if(m_currentTimerType == TIMER_CLOCK)
	{
		//return ((g_Timers[2].clockCount << 10) - (g_Timers[2].clockCount << 4) - (g_Timers[2].clockCount << 3)) + (g_Timers[1].clockCount - 0xFC18);
		return ((GBA_REG_TM2D << 10) - (GBA_REG_TM2D << 4) - (GBA_REG_TM2D << 3)) + (GBA_REG_TM1D - 0xFC18);
	}
	//return ((m_secondsSinceBoot << 10) - (m_secondsSinceBoot << 4) - (m_secondsSinceBoot << 3)) + (g_Timers[1].clockCount - 0xFC18);
	return ((m_secondsSinceBoot << 10) - (m_secondsSinceBoot << 4) - (m_secondsSinceBoot << 3)) + (GBA_REG_TM1D - 0xFC18);
}