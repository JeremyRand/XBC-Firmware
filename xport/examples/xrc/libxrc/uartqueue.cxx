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
#include "uartqueue.h"
#include "intcont.h"

CUartQueue::CUartQueue(CInterruptCont *pIntCont, unsigned long base,  unsigned char vector) : CUartComm(base)
	{
	Clear();
	m_vector = vector;

	SetInterruptCont(pIntCont);
	}

CUartQueue::~CUartQueue()
	{
	if (m_pIntCont)
		m_pIntCont->UnRegister(m_vector);
	}

void CUartQueue::SetInterruptCont(CInterruptCont *pIntCont)
	{
	m_pIntCont = pIntCont;
	if (m_pIntCont)
		{
		// register interrupt
		m_pIntCont->Register(this, m_vector);
		// enable interrupt
		m_pIntCont->Unmask(m_vector);
		}
	}

void CUartQueue::Clear()
	{
	if (m_pIntCont)
		m_pIntCont->Mask(m_vector);		
	m_count = 0;
	m_indexWrite = 0;
	m_indexRead = 0;
	if (m_pIntCont)
		m_pIntCont->Unmask(m_vector);
	}

int CUartQueue::ReceiveCount()
	{
	return m_count;
	}

int CUartQueue::Read(char *buf, unsigned int len)
	{
	unsigned int i;

	for (i=0; i<len; i++)
		{
		if (m_count-i>0)
			{
			buf[i] = m_buf[m_indexRead];
			m_indexRead++;
			if (m_indexRead==DUQ_SIZE)
				m_indexRead = 0;
			}
		else
			break;
		}

	m_count -= i;

	return i;
	}

unsigned int CUartQueue::ReadData(char *buf, unsigned int len, unsigned short timeoutMs)
	{
	unsigned int read;
	unsigned int left;
	unsigned int t = 0;

	left = len;
	while(1)
		{
		if (m_pIntCont)
			m_pIntCont->Mask(m_vector);		
		read = Read(buf, left);
		if (m_pIntCont)
			m_pIntCont->Unmask(m_vector);
		
		if (read)
			{
			left -= read;
			buf += read;
			t = 0; // reset timeout counter
			}
		else
			{
			// check timeout
			if (t++>=timeoutMs && timeoutMs!=0xffff)
				break;
			DelayMs(1);
			}
		if (left==0) 
			break;
		}
	return len-left;
	}


void CUartQueue::Interrupt(unsigned char vector)
	{
	char byte;

	while(*m_regReceiveCount)
		{
		byte = ReadByte();
		if (m_count<DUQ_SIZE)
			{
			m_buf[m_indexWrite] = byte;
			m_indexWrite++;
			if (m_indexWrite==DUQ_SIZE)
				m_indexWrite = 0;
			m_count++;
			}
		}
	}

