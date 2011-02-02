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
#include "btqueue.h"
#include "intcont.h"

CBluetoothQueue::CBluetoothQueue(CInterruptCont *pIntCont, unsigned long base,  unsigned char vector) : CBluetoothComm(base)
	{
	Clear();
	m_vector = vector;

	SetInterruptCont(pIntCont);
	}

CBluetoothQueue::~CBluetoothQueue()
	{
	if (m_pIntCont)
		m_pIntCont->UnRegister(m_vector);
	}

void CBluetoothQueue::SetInterruptCont(CInterruptCont *pIntCont)
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

void CBluetoothQueue::Clear()
	{
	if (m_pIntCont)
		m_pIntCont->Mask(m_vector);
	m_count = 0;
	m_indexWrite = 0;
	m_indexRead = 0;
	if (m_pIntCont)
		m_pIntCont->Unmask(m_vector);
	}

int CBluetoothQueue::ReceiveCount()
	{
	return m_count;
	}

int CBluetoothQueue::Read(char *buf, unsigned int len)
	{
	unsigned int i;

	for (i=0; i<len; i++)
		{
		if (m_count-i>0)
			{
			buf[i] = m_buf[m_indexRead];
			m_indexRead++;
			if (m_indexRead==DCQ_SIZE)
				m_indexRead = 0;
			}
		else
			break;
		}

	m_count -= i;

	return i;
	}

void CBluetoothQueue::Fill()
	{
	while(*m_regReceiveCount)
		{
		if (m_count<DCQ_SIZE)
			{
			m_buf[m_indexWrite] = ReadByte();
			m_indexWrite++;
			if (m_indexWrite==DCQ_SIZE)
				m_indexWrite = 0;
			m_count++;
			}
		else
			break;
		}
	}

int CBluetoothQueue::Reset()
	{
	int result;
	result = CBluetoothComm::Reset();
	Clear();
	return result;
	}

unsigned int CBluetoothQueue::ReadData(char *buf, unsigned int len, unsigned short timeoutMs)
	{
	unsigned int res;

	if (m_pIntCont)
		m_pIntCont->Mask(m_vector);
	// read from queue first
	if (m_count)
		{
		res = Read(buf, len);
		if (res<len)  // Do we need more data that is not in the queue?
			res += CBluetoothComm::ReadData(buf+res, len-res, timeoutMs);
		}
	else 
		res = CBluetoothComm::ReadData(buf, len, timeoutMs);
	if (m_pIntCont)
		m_pIntCont->Unmask(m_vector);

	return res;
	}


void CBluetoothQueue::Interrupt(unsigned char vector)
	{
	Fill();
	// if we haven't emptied it enough, disable interrupt
	if (m_pIntCont->GetStatus(m_vector))
		m_pIntCont->Mask(m_vector);
	}

