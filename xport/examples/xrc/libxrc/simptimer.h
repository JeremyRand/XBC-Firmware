#ifndef _SIMPTIMER_H
#define _SIMPTIMER_H

#include "regbits.h"

#define DST_BASE					0x9ffd600



	enum TimerFrequency
	{
		TIMER_FREQ_1 = 0,
		TIMER_FREQ_64 = 1,
		TIMER_FREQ_128 = 2,
		TIMER_FREQ_1024 = 3
	};
	enum TimerType {
		TIMER_CLOCK = 0,
		TIMER_INTERRUPT = 1,
		TIMER_FPGA = 2
	};
	
	
	
class CSimpTimer
	{
public:
	CSimpTimer(unsigned long base=DST_BASE);

	// The timer value is reset to 0 upon power up and counts at a fixed frequency of 
	// 1 count per microsecond. 

	// There is no risk of reading an inconsistent value due to the fact that the count 
	// spans 3 words.  The count value is latched upon reading the first word.

	// just get the first 32 bits
	void GetCount(unsigned long *count);
	// get the whole thing
	void GetCount(unsigned long long *count);
	
	void StartSystemTimer();
	void StopSystemTimer();
	void SwitchSystemTimerType(int iType);

protected:

	static unsigned int m_secondsSinceBoot;
	static unsigned short m_currentTimerType;
	unsigned int GetMillisecondsSinceBoot();

private:
	unsigned long m_base;
	};



#endif
