#include "adc.h"

CAdc::CAdc(unsigned long base)
	{
	m_status = (volatile unsigned short *)base;
	m_data = m_status + 1;
	}

unsigned short CAdc::GetChannel(unsigned char channel)
	{
	// indicate channel -- can also indicate differential values
	*m_data = 8 + channel;

	// wait for conversion
	while(*m_status);

	return *m_data>>3;
	}
