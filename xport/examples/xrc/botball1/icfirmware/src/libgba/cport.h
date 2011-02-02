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

#ifndef _CPORT_H
#define _CPORT_H

#include "xport.h"

class CCport
	{
	public:
	CCport(); 
	short Send(const char *outData, unsigned long len);
	short Receive(char *inData, unsigned long len);
	inline void SendChar(char out)
		{
		while(XmitBusy());
		XP_REG_CPORT_DATA = (unsigned short)out;
		}

	inline char ReceiveChar()
		{
		while(RecvEmpty());
		return (char)XP_REG_CPORT_DATA;
		}

	inline unsigned short XmitBusy()
		{
		return XP_REG_CPORT_STAT&0x0002;
		}

	inline unsigned short RecvEmpty()
		{
		return XP_REG_CPORT_STAT&0x0001;
		}

	inline unsigned short Host() // is host bit set?
		{
		return XP_REG_CPORT_STAT&0x0008;
		}
	};

#endif
