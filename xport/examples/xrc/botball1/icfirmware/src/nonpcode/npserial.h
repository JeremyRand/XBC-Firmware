#ifndef NPSERIAL_H
#define NPSERIAL_H

#ifndef NO_NPSERIAL
#include "CUartDevice.h"


class NPSerial
{
public:
	NPSerial();
	~NPSerial();

	//Gets the new code over serial
	char * GetNewNPC();

protected:

	char * space;
	CUartDevice * uartdev;

	void WriteBit(char * data);

};
#endif /* #ifndef NO_NPSERIAL */

#endif
