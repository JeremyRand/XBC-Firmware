#ifndef _COMM_H
#define _COMM_H

#define DCOMM_TIMEOUT_FOREVER		0xffff

class IComm
	{
public:
	// the timeoutMs is a timeout value in milliseconds.  The timeout timer should expire
	// when the data channel has been continuously idle for the specified amount of time
	// not the summation of the idle times.
	virtual int Send(const char *data, unsigned int len, unsigned short timeoutMs) = 0;
	virtual int Receive(char *data, unsigned int len, unsigned short timeoutMs) = 0;
	};

#endif
