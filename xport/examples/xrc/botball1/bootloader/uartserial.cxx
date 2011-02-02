#include "uartserial.h"

CUartSerial::CUartSerial(CInterruptCont *pIntCont, long base, unsigned char vector) : m_uartQueue(pIntCont, base, vector)
	{
	}

CUartSerial::~CUartSerial()
	{
	}

void CUartSerial::SetBaud(unsigned short divisor)
	{
	m_uartQueue.SetBaud(divisor);
	}

int CUartSerial::Send(const char *data, unsigned int len, unsigned short timeoutMs)
	{
	return m_uartQueue.WriteData(data, len);
	}

int CUartSerial::Receive(char *data, unsigned int len, unsigned short timeoutMs)
	{
	return m_uartQueue.ReadData(data, len, timeoutMs);
	}
