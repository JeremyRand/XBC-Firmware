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

#include "rcservo.h"
#include "../../include/gba.h"

CRCServo::CRCServo(unsigned char num, unsigned short *addr, bool enable)
	{
	int i;

	m_num = num;
	m_addr = addr;
	m_boundLower = new unsigned char[m_num];
	m_boundUpper = new unsigned char[m_num];
	m_shadowPos = new unsigned char[m_num];
	m_shadow = new unsigned short[m_num>>1];
	for (i=0; i<num; i++)
		{
		m_boundLower[i] = 0x00;
		m_boundUpper[i] = 0xff;
		}
	for (i=0; i<num>>1; i++)  
		{
		m_addr[i] = 0;
		m_shadow[i] = 0;
		}
	if (enable)
		Enable();
	}

CRCServo::~CRCServo()
	{
	delete [] m_boundLower;
	delete [] m_boundUpper;
	delete [] m_shadowPos;
	delete [] m_shadow;
	}

void CRCServo::Enable()
	{
	GBA_REG_WSCNT &= ~GBA_PHI_MASK;
	GBA_REG_WSCNT |= GBA_PHI_4_19MHZ;
	}

void CRCServo::Disable()
	{
	GBA_REG_WSCNT &= ~GBA_PHI_MASK;
	GBA_REG_WSCNT |= GBA_PHI_NONE;
	}

unsigned char CRCServo::GetPosition(unsigned char index)
	{
	if (index>=m_num)
		return 0;

	return m_shadowPos[index];
	}

void CRCServo::SetPosition(unsigned char index, unsigned char pos)
	{
	unsigned char scaledPos;

	if (index>=m_num)
		return;

	m_shadowPos[index] = pos;

	// scale position such that it lies between bounds, but maintain full input range [0..255]
	scaledPos = (unsigned char)(((unsigned short)(m_boundUpper[index]-m_boundLower[index])*(unsigned short)pos)>>8) + m_boundLower[index];
	if (index&1)
		{
		index >>= 1;
		m_shadow[index] = scaledPos<<8 | m_shadow[index]&0x00ff;
		}
	else
		{
		index >>= 1;
		m_shadow[index] = scaledPos | m_shadow[index]&0xff00;
		}
	m_addr[index] = m_shadow[index];
	}

void CRCServo::SetBounds(unsigned char index, unsigned char lower, unsigned char upper)
	{
	if (index<m_num && lower<upper)
		{
		m_boundUpper[index] = upper;
		m_boundLower[index] = lower;
		}
	}
