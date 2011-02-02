#ifndef _PCSERIAL_H
#define _PCSERIAL_H

#include <string>
#include "serialstream.h"
#include "../../librpc/comm.h"

using namespace std;

class CPCSerial : public IComm
	{
public:
	virtual ~CPCSerial();
	int Open(string port, unsigned int baud);
	int Close();

	// IComm methods
	virtual int Send(const char *data, unsigned int len, unsigned short timeoutMs);
	virtual int Receive(char *data, unsigned int len, unsigned short timeoutMs);

private:
	SerialStream serialPort;
	};

#endif
