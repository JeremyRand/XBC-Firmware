#ifndef _ADC_H
#define _ADC_H

class CAdc
	{
public:
	CAdc(unsigned long base);
	
	unsigned short GetChannel(unsigned char channel);

private:
	volatile unsigned short *m_status;
	volatile unsigned short *m_data;
	};

#endif
