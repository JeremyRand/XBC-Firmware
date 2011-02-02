#ifndef _UARTSERIAL_H
#define _UARTSERIAL_H

#include "comm.h"
#include "uartqueue.h"

class CUartSerial : public IComm
	{
public:
	CUartSerial(CInterruptCont *pIntCont, long base=DUQ_BASE, unsigned char vector=16);
	virtual ~CUartSerial();
	void SetBaud(unsigned short divisor);

	// IComm methods
	virtual int Send(const char *data, unsigned int len, unsigned short timeoutMs);
	virtual int Receive(char *data, unsigned int len, unsigned short timeoutMs);

private:
	CUartQueue m_uartQueue;
	};

#endif
