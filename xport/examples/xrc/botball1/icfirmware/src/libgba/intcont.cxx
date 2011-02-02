#include "intcont.h"
#include "iinterrupt.h"
#include "gba.h"
#include "xport.h"

extern "C" 
	{
	void _gba_interrupt(void);
	}

IInterrupt *CInterruptCont::m_vectors[DCI_MAX_VECTORS];
unsigned short CInterruptCont::m_compare[DCI_MAX_VECTORS];

CInterruptCont::CInterruptCont()
	{
	int i;

	for (i=0; i<DCI_MAX_VECTORS; i++)
		{
		m_vectors[i] = 0;
		m_compare[i] = 0;
		}

	XP_REG_INTERRUPT_MASK = 0;
	XP_REG_INTERRUPT_STAT = 0xffff;

	GBA_REG_IF = 0xffff;		// clear all interrupt status flags
	GBA_REG_IE = GBA_INT_CART;	// enable cartridge interrupt
	GBA_REG_IME = 1;			// set master interrupt enable
	}

CInterruptCont::~CInterruptCont()
	{
	GBA_REG_IE &= ~GBA_INT_CART;
	GBA_REG_IME = 0;
	}

void _gba_interrupt(void)
	{
	unsigned char i;

	if(GBA_REG_IF&0x1fff)
		{
		for (i=0; i<13 && GBA_REG_IF; i++)
			{
			if (GBA_REG_IF&CInterruptCont::m_compare[i])
				{
				GBA_REG_IF = CInterruptCont::m_compare[i];
				CInterruptCont::m_vectors[i]->Interrupt(i);
				}
			}
		}

	if (GBA_REG_IF&GBA_INT_CART)
		{
		// clear cart interrupt
		GBA_REG_IF = GBA_INT_CART;
		while(XP_REG_INTERRUPT_STAT)
			{
			for (i=16; i<DCI_MAX_VECTORS && XP_REG_INTERRUPT_STAT; i++)
				{
				if (XP_REG_INTERRUPT_STAT&CInterruptCont::m_compare[i])
					{
					CInterruptCont::m_vectors[i]->Interrupt(i);
					XP_REG_INTERRUPT_STAT = CInterruptCont::m_compare[i];
					}
				}
			}
		}
	}

// service all higher priority vectors (all vectors smaller than you)
unsigned char CInterruptCont::ServiceCheck(unsigned char vector)
	{
	unsigned char i, maxVector;
	unsigned short mask;

	if (vector>12)
		{
		mask = 0x1fff;
		maxVector = 13;
		}
	else 
		{
		mask = (1<<vector)-1;
		maxVector = vector;
		}

	if(GBA_REG_IF&mask)
		{
		for (i=0; i<maxVector && GBA_REG_IF; i++)
			{
			if (GBA_REG_IF&CInterruptCont::m_compare[i])
				{
				GBA_REG_IF = CInterruptCont::m_compare[i];
				CInterruptCont::m_vectors[i]->Interrupt(i);
				}
			}
		}

	if (vector>16)
		{
		mask = (1<<(vector-16))-1;
		maxVector = vector;
		while(XP_REG_INTERRUPT_STAT&mask)
			{
			for (i=16; i<maxVector && XP_REG_INTERRUPT_STAT; i++)
				{
				if (XP_REG_INTERRUPT_STAT&CInterruptCont::m_compare[i])
					{
					CInterruptCont::m_vectors[i]->Interrupt(i);
					XP_REG_INTERRUPT_STAT = CInterruptCont::m_compare[i];
					}
				}
			}
		}
	}

int CInterruptCont::Register (IInterrupt *pint, unsigned char vector)
	{
	if (vector<DCI_MAX_VECTORS && pint)
		{
		m_vectors[vector] = pint;
		if (vector>=16) // Xport interrupt
			m_compare[vector] = 1<<(vector-16);
		else            // GBA interrupt
			m_compare[vector] = 1<<vector;
		return 0;
		}
	return -1;
	}

IInterrupt* CInterruptCont::GetRegistered (unsigned char vector)
{
	return m_vectors[vector];
}

int CInterruptCont::UnRegister (unsigned char vector)
	{
	if (vector<DCI_MAX_VECTORS)
		{
		m_vectors[vector] = 0;
		m_compare[vector] = 0;
		return 0;
		}
	return -1;
	}

void CInterruptCont::Interrupt(unsigned char vector)
	{
	m_vectors[vector]->Interrupt(vector);
	}

void CInterruptCont::Mask(unsigned char vector)
	{
	if (vector>=16)
		XP_REG_INTERRUPT_MASK &= ~(m_compare[vector]); 
	else
		GBA_REG_IE &= ~(m_compare[vector]);
	}

void CInterruptCont::Unmask(unsigned char vector)
	{
	if (vector>=16)
		XP_REG_INTERRUPT_MASK |= m_compare[vector];
	else
		GBA_REG_IE |= m_compare[vector];
	}

void CInterruptCont::MaskAll()
	{
	// set master interrupt enable
	GBA_REG_IME = 0;
	}

void CInterruptCont::UnmaskAll()
	{
	GBA_REG_IME = 1;
	}

unsigned short CInterruptCont::GetStatus(unsigned char vector)
	{
	if (vector>=16)
		return XP_REG_INTERRUPT_STAT & m_compare[vector];
	else
		return GBA_REG_IF & m_compare[vector];
	}
