#ifndef CBLUETOOTHDEVICE_H
#define CBLUETOOTHDEVICE_H

#include "btqueue.h"
#include "CCommunicationDevice.h"
#include "../../../logic/botball1.h"

class CBluetoothDevice :public CBluetoothQueue, public CCommunicationDevice
{
  public:
	CBluetoothDevice();
	virtual ~CBluetoothDevice();
	virtual void Connect(int msTimeout = 0);
	virtual void MConnect(char * addr, int msTimeout = 0);
	virtual int Read(char *inData, unsigned short len, int timeout = 0);
	virtual int Write(const char *outData, unsigned short len, int timeout = 0);
  	virtual int QueryReadCount();
  	virtual int SetBaudRate(int baud);
	virtual int GetBaudRate();
};

#endif
