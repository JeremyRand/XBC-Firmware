#include "sccb.h"

CSccb::CSccb(unsigned long addr, unsigned char dev)
	{
	m_reg = (volatile unsigned short *)addr;
	m_dev = dev;
	m_bits = 0x0000;
	m_drive = 0x0000;
	TriState();	// set clock as output, data tristate
	PortWrite(0x02);	// set clock high (idle)

	Reset();
	}

void CSccb::Reset()
	{
	volatile unsigned long d; 

	Read(0x00);
	Write(0x12, 0xa4); // reset operation
	for (d=0; d<1000; d++);
	Read(0x00);
	}

void CSccb::Write(unsigned char addr, unsigned char val)
	{
	Start();
	Write(m_dev);
	Write(addr);
	Write(val);
	Stop();
	}

unsigned char CSccb::Read(unsigned char addr)
	{
	unsigned char i, val;

	// 2 phase
	Start();
	Write(m_dev);
	Write(addr);
	Stop();

	Start();
	Write(m_dev + 1);
	TriState();
	for (i=0, val=0; i<8; i++)
		{
		val <<= 1;
		PortWrite(0x02);
		val |= *m_reg&0x01;
		PortWrite(0x00);
		}

	// send NA bit
	PortWrite(0x01);
	Drive();
	PortWrite(0x03);
	PortWrite(0x01);

	Stop();

	return val;
	}

void CSccb::Write(unsigned char val)
	{
	unsigned char i;

	for (i=0; i<8; i++, val<<=1)
		{
		if (val&0x80) // send msb first
			{
			PortWrite(0x01);
			PortWrite(0x03);
			PortWrite(0x01);
			}
		else
			{
			PortWrite(0x00);
			PortWrite(0x02);
			PortWrite(0x00);
			}
		}
	// send "don't care" bit
	TriState();
	PortWrite(0x00);
	PortWrite(0x02);
	PortWrite(0x00);
	Drive();
	}

