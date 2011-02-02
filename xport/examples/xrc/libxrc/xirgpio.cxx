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

#include "xirgpio.h"

CXirGpio::CXirGpio(unsigned long baseAddr, unsigned char mode)
	{
	unsigned char i;

	m_regXir = (unsigned short *)baseAddr;
	m_regXirStatus = m_regXir + 8;
	m_regXirConfig = m_regXir + 9;
	m_regMode = m_regXir + 10;
	m_regGpioDataDir = m_regXir + 11;
	m_regGpioData = m_regXir + 12;

	SetMode(mode);

	// set max power by default
	for (i=0; i<8; i++)
		SetXirPower(i, 0xff);

	m_divisor = 1;
	}

CXirGpio::~CXirGpio()
	{
	}

void CXirGpio::SetMode(unsigned char mode)
	{
	*m_regMode = mode;
	}

void CXirGpio::SetXirPower(unsigned char index, unsigned char power)
	{
	m_regXir[index] = (unsigned short)power;
	}

unsigned short CXirGpio::GetXir(unsigned char index)
	{
	unsigned short val;

	// since XIR sensors are asynchonous with CPU, we need 
	// to do this to make sure we have a valid reading
	while(1)
		{
		val = m_regXir[index];
		if (m_regXir[index]==val)
			break;
		}
	if (m_divisor>1)
		return val/m_divisor;
	else
		return val;
	}

void CXirGpio::SetXirAverager(unsigned char window)
	{
	if (window>0x0f)
		window = 0x0f;

	m_divisor = (window<<2) + 1;

	window <<= 4;
	*m_regXirConfig &= 0x0f;
	*m_regXirConfig |= window;
	}

void CXirGpio::SetXirRingLength(unsigned char ringLength)
	{
	ringLength--;
	if (ringLength>0x07)
		ringLength = 0x07;

	*m_regXirConfig &= 0xf0;
	*m_regXirConfig |= ringLength;
	}

unsigned short CXirGpio::GetXirStatus()
	{
	return *m_regXirStatus;
	}

void CXirGpio::SetGpioDataDir(unsigned short dataDir)
	{
	*m_regGpioDataDir = dataDir;
	}

void CXirGpio::SetGpioData(unsigned short data)
	{
	*m_regGpioData = data;
	}

unsigned short CXirGpio::GetGpioData()
	{
	return *m_regGpioData;
	}

