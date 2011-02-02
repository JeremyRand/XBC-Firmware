#ifndef CUARTDEVICE_H
#define CUARTDEVICE_H

#include "uartqueue.h"
#include "CCommunicationDevice.h"

class CUartDevice : public CUartQueue, public CCommunicationDevice
{
public:
	CUartDevice();
	virtual ~CUartDevice();
	virtual void Connect(int msTimeout = 0);
	virtual int Read(char *inData, unsigned short len, int timeout = 0);
	virtual int Write(const char *outData, unsigned short len, int timeout = 0);
  virtual int QueryReadCount();
	virtual int SetBaudRate(int baud);
	virtual int GetBaudRate();
};

#endif
