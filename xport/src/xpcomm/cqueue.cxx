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

#include "cqueue.h"
#include <stdio.h>

CcQueue::CcQueue(unsigned int size)
	{
	m_size = size;
	m_buf = new char[m_size];
	m_count = 0;
	m_indexWrite = 0;
	m_indexRead = 0;
	}

CcQueue::~CcQueue()
	{
	delete [] m_buf;
	}

int CcQueue::Empty()
	{
	return m_size-m_count; 
	}

int CcQueue::Full()
	{
	return m_count;
	}

int CcQueue::EnQueue(unsigned char *buf, unsigned int len)
	{
	unsigned int i;

	for (i=0; i<len; i++)
		{
		if (m_count<m_size)
			{
			m_buf[m_indexWrite] = buf[i];
			m_indexWrite++;
			if (m_indexWrite==m_size)
				m_indexWrite = 0;
			m_count++;
			}
		else
			break;
		}
	return i;
	}

int CcQueue::DeQueue(unsigned char *buf, unsigned int len)
	{
	unsigned int i;

	for (i=0; i<len; i++)
		{
		if (m_count>0)
			{
			buf[i] = m_buf[m_indexRead];
			m_indexRead++;
			if (m_indexRead==m_size)
				m_indexRead = 0;
			m_count--;
			}
		else
			break;
		}
	return i;
	}

int CcQueue::Clear()
	{
	m_count = 0;
	m_indexWrite = 0;
	m_indexRead = 0;
	}

