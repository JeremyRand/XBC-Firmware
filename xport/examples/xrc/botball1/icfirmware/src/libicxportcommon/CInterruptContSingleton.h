#ifndef INTSINGLETON_H
#define INTSINGLETON_H

#define InterruptContSingleton CInterruptContSingleton::Instance()

#include "intcont.h"
#include "singleton.h"

class CInterruptContSingleton : public CInterruptCont, public CSingleton<CInterruptContSingleton>
{
public:
	enum
	{
		VBLANK = 0,
		HBLANK,
		VCOUNT,
		TIMER0,
		TIMER1,
		TIMER2,
		TIMER3,
		SERIAL,
		DMA0,
		DMA1,
		DMA2,
		DMA3,
		KEY,
		CART
	};
	CInterruptContSingleton() {}
	~CInterruptContSingleton() {}
};
#endif
