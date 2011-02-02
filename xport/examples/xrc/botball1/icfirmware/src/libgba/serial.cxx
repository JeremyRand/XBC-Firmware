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

#include "gba.h"
#include "textdisp.h"
#include "serial.h"

short CSerial::Init(unsigned char flags)
	{
	GBA_REG_R &= 0x7fff;
	GBA_REG_SCCNT_L = 0x3d80 | flags&0x7;
	}

CSerial::CSerial(unsigned char flags)
	{
	Init(flags);
	}

short CSerial::Send(char *outData, unsigned short len)
	{
	unsigned short i;

	for (i=0; i<len; i++)
		{
		while(GBA_REG_SCCNT_L&0x0010);
		GBA_REG_SCCNT_H = (unsigned short)outData[i];
		}
	}

short CSerial::Receive(char *inData, unsigned short len)
	{
	unsigned short i;

	for (i=0; i<len; i++)
		{
		while(GBA_REG_SCCNT_L&0x0020);
		inData[i] = (char)(GBA_REG_SCCNT_H&0xff);
		}
	}
