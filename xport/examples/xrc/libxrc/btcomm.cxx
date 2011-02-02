//  BEGIN LICENSE BLOCK
// 
//  Version: MPL 1.1
// 
//  The contents of this file are subject to the Mozilla Public License Version
//  1.1 (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
// 
//  Software distributed under the License is distributed on an "AS IS" basis,
//  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
//  for the specific language governing rights and limitations under the
//  License.
// 
//  The Original Code is The Xport Software Distribution.
// 
//  The Initial Developer of the Original Code is Charmed Labs LLC.
//  Portions created by the Initial Developer are Copyright (C) 2003
//  the Initial Developer. All Rights Reserved.
// 
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
// 
//  END LICENSE BLOCK 

#include <string.h>
#include "btcomm.h"
#include "textdisp.h"

#define DEBUG

extern CTextDisp td;

CBluetoothComm::CBluetoothComm(unsigned long base)
	{
	m_regStatus = (volatile unsigned short *)base;
	m_regData = m_regStatus+1;
	m_regBaud = m_regStatus+2;
	m_regReceiveCount = m_regStatus+3;
	m_initialized = false;
	}

int CBluetoothComm::Reset(unsigned short baud)
	{
	char buf[16]; 
	volatile unsigned int d;
	unsigned int size = 0;

	m_initialized = true;
	// reset radio
	*m_regStatus |= 0x0008;
	for (d=0; d<100000; d++);
	*m_regStatus &= ~0x0008;
	for (d=0; d<600000; d++);
	
	// set baud rate to 9600
	*m_regBaud = DBC_BAUD_9600;
	// set baud rate to new rate
	switch(baud)
		{
		case DBC_BAUD_921K:
		WriteString("ATSW20,3775,0\r");
		size = ReadResponse(buf, 16, 200, 1000);
		*m_regBaud = DBC_BAUD_921K;
		break;

		case DBC_BAUD_115K:
		WriteString("ATSW20,472,0\r");
		size = ReadResponse(buf, 16, 200, 1000);
		*m_regBaud = DBC_BAUD_115K;
		break;

		case DBC_BAUD_57K:
		WriteString("ATSW20,236,0\r");
		size = ReadResponse(buf, 16, 200, 1000);
		*m_regBaud = DBC_BAUD_57K;
		break;

		case DBC_BAUD_38K:
		WriteString("ATSW20,157,0\r");
		size = ReadResponse(buf, 16, 200, 1000);
		*m_regBaud = DBC_BAUD_38K;
		break;
		}

	if (size==0)
		{
#ifdef DEBUG
		printf("Error: baudrate\n");
#endif
		m_initialized = false;
		return -1;
		}

	// put radio into command mode
	buf[0] = '\0';
	WriteString("ATMC\r");
	size = ReadResponse(buf, 16, 200, 1000);
	if (size==0)
		{
#ifdef DEBUG
		printf("Error: command mode\n");
#endif
		m_initialized = false;
		return -1;
		}

	// reset pending commands
	WriteString("ATUCL\r");
	size = ReadResponse(buf, 16, 200, 1000);
	if (size==0)
		{
#ifdef DEBUG
		printf("Error: reset pending\n");
#endif
		m_initialized = false;
		return -1;
		}

	return 0;
	}

int CBluetoothComm::MasterConnect(char *addr, unsigned short timeoutMs, unsigned short baud)
	{
	unsigned int size;
	int result;
	char buf[0x100];
	
	// reset 
	if ((result=Reset(baud))<0)
		return result;

	m_initialized = true;
	WriteString("ATDM,");
	WriteString(addr);
	WriteString(",1101\r");
	// read "OK"
	size = ReadResponse(buf, 0x100, 200, 1000);
	buf[0] = '\0';

	// read "CONNECT
	size = ReadResponse(buf, 0x100, 200, timeoutMs);

	if (!strstr(buf, "CONNECT"))
		{
#ifdef DEBUG
		printf("Error D %d %s\n", size, buf);
#endif
		m_initialized = false;
		return -1;
		}

	WriteString("ATMC\r");
	size = ReadResponse(buf, 0x100, 200, 1000);
	if (size==0)
		{
#ifdef DEBUG
		printf("Error E\n");
#endif
		m_initialized = false;
		return -1;
		}
	WriteString("ATMF\r");
	size = ReadResponse(buf, 0x100, 200, 1000);
	if (size==0)
		{
#ifdef DEBUG
		printf("Error F\n");
#endif
		m_initialized = false;
		return -1;
		}
	return 0;
	}
	
int CBluetoothComm::SlaveConnect(unsigned short baud)
	{
	unsigned int size;
	int result;
	char buf[0x100];

	// reset 
	if ((result=Reset(baud))<0)
		return result;
	

	m_initialized = true;
	WriteString("ATDS\r");
	// read "OK"
	size = ReadResponse(buf, 0x100, 200, 1000);
	// read "CONNECT"
	buf[0] = '\0';
	size = ReadResponse(buf, 0x100, 200, 30000);
	if (!strstr(buf, "CONNECT"))
		{
#ifdef DEBUG
		printf("Error G %d %s\n", size, buf);
#endif
		m_initialized = false;
		return -1;
		}

	WriteString("ATMC\r");
	size = ReadResponse(buf, 0x100, 200, 1000);
	if (size==0)
		{
#ifdef DEBUG
		printf("Error H\n");
#endif
		m_initialized = false;
		return -1;
		}
	WriteString("ATMF\r");
	size = ReadResponse(buf, 0x100, 200, 1000);
	if (size==0)
		{
#ifdef DEBUG
		printf("Error I\n");
#endif
		m_initialized = false;
		return -1;
		}
	return 0;
	}

unsigned int CBluetoothComm::ReadData(char *buf, unsigned int len, unsigned short timeoutMs)
	{
	unsigned int t = 0;
	unsigned short i = 0;

	if (!m_initialized)
		return 0;

	while(i<len)
		{
		if (ReadBusy())
			{
			DelayMs(1);
			if (++t>=timeoutMs && timeoutMs!=0xffff)
				break;
			}
		else
			{
			buf[i] = ReadByte();
			i++;
			}
		}

	return i;
	}

unsigned int CBluetoothComm::WriteData(const char *data, unsigned int len)
	{
	unsigned int i = 0;

	if (!m_initialized)
		return 0;

	while(i<len)
		{
		while(WriteBusy());
		WriteByte(data[i++]);
		}
	return i;
	}

unsigned int CBluetoothComm::WriteString(const char *string)
	{
	unsigned int i = 0;

	if (!m_initialized)
		return 0;

	while(string[i])
		{
		while(WriteBusy());
		WriteByte(string[i++]);
		}
	return i;
	}

int CBluetoothComm::GetName(char *name, int size)
	{
	unsigned int result;
	char buf[0x100];
	int i, j;

	if (!m_initialized)
		return -1;

	WriteString("ATSI,2\r");
	result = ReadResponse(buf, sizeof(buf), 200, 1000);
	if (result==0)
		return -1;
	for (i=0; buf[i]=='\r' || buf[i]=='\n'; i++);
	for (j=0; buf[i]!='\0' && buf[i]!='\r' && buf[i]!='\n'; i++, j++)
		name[j] = buf[i];
	
	name[j] = '\0';
	return 0;
	}

int CBluetoothComm::SetName(const char *name)
	{
	unsigned int result;
	char buf[0x100];
 
	if (!m_initialized)
		return -1;

	WriteString("ATSN,");
	WriteString(name);
	WriteString("\r");
	result = ReadResponse(buf, sizeof(buf), 200, 1000);
	if (result==0)
		return -1;
	return 0;
	}

int CBluetoothComm::GetDeviceAddress(char *address, int size)
	{
	unsigned int result;
	char buf[0x100];
	int i, j;

	if (!m_initialized)
		return -1;

	WriteString("ATSI,1\r");
	result = ReadResponse(buf, sizeof(buf), 200, 1000);
	if (result==0)
		return -1;
	for (i=0; buf[i]=='\r' || buf[i]=='\n'; i++);
	for (j=0; buf[i]!='\0' && buf[i]!='\r' && buf[i]!='\n'; i++, j++)
		address[j] = buf[i];
	address[j] = '\0';

	return 0;
	}

void CBluetoothComm::DelayMs(unsigned short delay)
	{
	unsigned short i;
	volatile unsigned short j;

	for (i=0; i<delay; i++)
		for (j=0; j<1100; j++); 
	}

unsigned int CBluetoothComm::ReadResponse(char *response, unsigned int len, unsigned short timeoutMs0, unsigned short timeoutMs1)
	{
	unsigned int i = 0;
	unsigned short t = 0;
	char c;

	if (!m_initialized)
		return 0;

	while(i<len)
		{
		if (ReadData(&c, 1, timeoutMs0))
			response[i++] = c;	
		else
			{
			if (i)
				break;
			t += timeoutMs0;
			if (t>=timeoutMs1 && timeoutMs1!=0xffff)
				break;
			}
		}
	if (i<len)
		response[i] = '\0';
	return i;
	}
