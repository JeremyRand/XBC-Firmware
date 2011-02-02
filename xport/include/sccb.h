#ifndef _SCCB_H
#define _SCCB_H

#define SCCB_DELAY		2

class CSccb
	{
public:
	CSccb(unsigned long addr, unsigned char dev);
	void Write(unsigned char addr, unsigned char val);
	unsigned char Read(unsigned char addr);
	void Reset();

private:
	void Write(unsigned char val);
	inline void PortWrite(unsigned char val)
		{
		volatile unsigned long d;
		m_bits = val;
		*m_reg = m_drive | m_bits;
		for (d=0; d<SCCB_DELAY; d++);
		}

	inline void Drive()
		{
		volatile unsigned long d;
		m_drive = 0x0004;
		*m_reg = m_drive | m_bits;
		for (d=0; d<SCCB_DELAY; d++);
		}

	inline void TriState()
		{
		volatile unsigned long d;
		m_drive = 0x0000;
		*m_reg = m_drive | m_bits;
		for (d=0; d<SCCB_DELAY; d++);
		}

	inline void Start()
		{
		PortWrite(0x03);	// data, clk high
		Drive();	// take data out of tristate
		PortWrite(0x02);	// data low
		PortWrite(0x00);	// data low
		}

	inline void Stop()
		{
		PortWrite(0x02);	// clk high, data low
		PortWrite(0x03);	// clk, data high
		TriState();	// put data into tristate
		}

	unsigned char m_dev;
	unsigned short m_drive;
	unsigned short m_bits;
	volatile unsigned short *m_reg;
	};

#endif
