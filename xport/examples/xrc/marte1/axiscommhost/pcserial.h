#ifndef _PCSERIAL_H
#define _PCSERIAL_H

#include <windows.h>
#include "serialstream.h"
#include "../../librpc/comm.h"

class CPCSerial : public IComm
	{
public:
	virtual ~CPCSerial();
	int Open(unsigned int comport, unsigned int baud);
	int Close();

	// IComm methods
	virtual int Send(const char *data, unsigned int len, unsigned short timeoutMs);
	virtual int Receive(char *data, unsigned int len, unsigned short timeoutMs);

private:
	SerialStream serialPort;
	HANDLE m_hComm;

	DWORD m_timeout;
	};

#endif
