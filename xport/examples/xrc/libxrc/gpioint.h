#ifndef _GPIOINT_H
#define _GPIOINT_H

#include <iinterrupt.h>

#define DG_BASE					0x9ffd200

class CInterruptCont;

class CGpioInt : public IInterrupt
	{
public:
	CGpioInt(CInterruptCont *pIntCont, unsigned long base=DG_BASE, unsigned char vector=21);
	virtual ~CGpioInt();

	// from IInterrupt interface
	virtual void Interrupt(unsigned char vector);

	// Each bit in this register reflects the state of corresponding pins.  
	// If it is input, the bit value reflects the inputted state on the corresponding pin
	// If it is output, the corresponding pin state reflects the bit value.
	volatile unsigned short *m_data;

	// Each bit in this register determines whether the corresponding pin is input or output.
	// If the bit value is 0, the corresponding pin is input.
	// If the bit value is 1, the corresponding pin is output.
	volatile unsigned short *m_dataDir;

	// Each bit in this register determines whether the corresponding pin will trigger an interrupt.
	// If the bit value is 0, the corresponding pin will not trigger an interrupt.
	// If the bit value is 1, the corresponding pin will trigger an interrupt on each rising or falling edge.
	// An interrupt is only generated for pins configured as input.   
	volatile unsigned short *m_intMask;

	// Each bit in this register determines which edge (rising or falling) the corresponding bit will trigger an interrupt.
	// If the bit value is 0, the corresponding pin will trigger on each negative edge of the corresponding pin.
	// If the bit value is 1, the corresponding pin will trigger on each positive edge of the corresponding pin.
	// An interrupt is only generated for pins that are unmasked and configured as input.
	volatile unsigned short *m_intEdge;

	// Each bit in this register reflects whether the corresponding pin has generated an interrupt.
	// If the bit value is 1, the corresponding pin has triggered an interrupt.
	// If the bit value is 0, the corresponding pin has not triggered an interrupt.
	// This register is intended to be read from the interrupt service routine.
	// This register is automatically reset by the interrupt controller (m_pIntCont). 
	volatile unsigned short *m_intStatus;

protected:
	CInterruptCont *m_pIntCont;
	unsigned char m_vector;
	};

#endif
