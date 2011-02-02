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
#include "uartcomm.h"
#include "textdisp.h"

extern CTextDisp td;

CUartComm::CUartComm(unsigned long base)
	{
	m_regStatus = (volatile unsigned short *)base;
	m_regData = m_regStatus+1;
	m_regBaud = m_regStatus+2;
	m_regReceiveCount = m_regStatus+3;

	// empty any data
	while(!ReadBusy())
		ReadByte();
	}

void CUartComm::SetBaud(unsigned short divisor)
	{
	*m_regBaud = divisor;
	}

unsigned int CUartComm::ReadData(char *buf, unsigned int len, unsigned short timeoutMs)
	{
	unsigned int t = 0;
	unsigned short i = 0;

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

unsigned int CUartComm::WriteData(const char *data, unsigned int len)
	{
	unsigned int i = 0;

	while(i<len)
		{
		while(WriteBusy());
		WriteByte(data[i++]);
		}
	return i;
	}

unsigned int CUartComm::WriteString(const char *string)
	{
	unsigned int i = 0;

	while(string[i])
		{
		while(WriteBusy());
		WriteByte(string[i++]);
		}
	return i;
	}


void CUartComm::DelayMs(unsigned short delay)
	{
	unsigned short i;
	volatile unsigned short j;

	for (i=0; i<delay; i++)
		for (j=0; j<1100; j++); 
	}

