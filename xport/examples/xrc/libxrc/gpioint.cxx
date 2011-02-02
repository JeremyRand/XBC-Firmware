#include <intcont.h>
#include "gpioint.h"


CGpioInt::CGpioInt(CInterruptCont *pIntCont, unsigned long base, unsigned char vector)
	{
	m_data = (volatile unsigned short *)base;
	m_dataDir = m_data + 1;
	m_intMask = m_data + 2;;
	m_intEdge = m_data + 3;;
	m_intStatus = m_data + 4;

	m_pIntCont = pIntCont;
	m_vector = vector;

	m_pIntCont->Register(this, m_vector);
	m_pIntCont->Unmask(m_vector);
	}

void CGpioInt::Interrupt(unsigned char vector)
	{
	}

CGpioInt::~CGpioInt()
	{
	m_pIntCont->UnRegister(m_vector);
	}


