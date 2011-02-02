#include <stdio.h>
#include "pcserial.h"

CPCSerial::~CPCSerial()
{
	Close();
}

int CPCSerial::Open(string port, unsigned int baud)
{
	/*
	DCB dcb;
	char buf[0x100];

	sprintf(buf, "COM%d", comport);
	m_hComm = CreateFile( buf, 
	GENERIC_READ | GENERIC_WRITE, 
	0, 
	0, 
	OPEN_EXISTING,
	0,
	0);
	if (m_hComm==INVALID_HANDLE_VALUE)
	return -1;

	ZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	if (!GetCommState(m_hComm, &dcb))     // get current DCB
	return -2;

	// Update DCB 
	sprintf(buf, "baud=%d parity=N data=8 stop=1", baud);
	if (!BuildCommDCB(buf, &dcb)) 
	// Couldn't build the DCB. Usually a problem
	// with the communications specification string.
	return -3;
	//	dcb.fTXContinueOnXoff = 0;
	//	dcb.fAbortOnError = 0;

	// Set new state.
	if (!SetCommState(m_hComm, &dcb))
	return -4;

	m_timeout = -1;
	// success
	*/
	serialPort.setFormat(8, 'n', 1);
	serialPort.setSpeed(baud);
	serialPort.connect(port);
	return 0;
}

int CPCSerial::Close()
{
	/*
	if (CloseHandle(m_hComm)==0)
		return -1;
		*/
	serialPort.disconnect();
	return 0;
}

int CPCSerial::Send(const char *data, unsigned int len, unsigned short timeoutMs)
{
	/*
	DWORD dwWrite;

	if (!WriteFile(m_hComm, data, len, &dwWrite, 0)) 
		return -1;
	*/
	serialPort.write((const unsigned char*) data, len);
	return len;
}

int CPCSerial::Receive(char *data, unsigned int len, unsigned short timeoutMs)
{
	/*
	COMMTIMEOUTS timeout;
	DWORD dwRead;

	if (timeoutMs != m_timeout)
	{
		timeout.ReadIntervalTimeout = 0;
		timeout.ReadTotalTimeoutConstant = timeoutMs;
		timeout.ReadTotalTimeoutMultiplier = 0;
		timeout.WriteTotalTimeoutConstant = 0;
		timeout.WriteTotalTimeoutMultiplier = 0;
		if (!SetCommTimeouts(m_hComm, &timeout))
			return -1;
		m_timeout = timeoutMs;
	}

	if (!ReadFile(m_hComm, data, len, &dwRead, 0))
		return -1;

	if (dwRead!=len)
		int q = 0;
	*/
	return serialPort.readTimeout((unsigned char*)data, len, timeoutMs);

}

