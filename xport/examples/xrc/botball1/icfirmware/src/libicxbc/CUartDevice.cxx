#include "CUartDevice.h"
#include "CInterruptContSingleton.h"

CUartDevice::CUartDevice() : CUartQueue(CInterruptContSingleton::InstancePtr())
{
	m_isConnected = true;
	globBaud = DUC_BAUD_38K;
	SetBaud(globBaud);
}

CUartDevice::~CUartDevice()
{

}

int CUartDevice::QueryReadCount()
{
  return ReceiveCount();
}

void CUartDevice::Connect(int msTimeout)
{
	m_isConnected = true;
}

int CUartDevice::Read(char *buf, unsigned short len, int timeout)
{
	return ReadData(buf, len, 1);
}

int CUartDevice::Write(const char *outData, unsigned short len, int timeout)
{
	return WriteData(outData, len);
}

int CUartDevice::SetBaudRate(int baud)
{
	SetBaud(baud);
	return 1;
}

int CUartDevice::GetBaudRate()
{
	return globBaud;
}
