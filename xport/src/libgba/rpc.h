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

#ifndef _RPC_H
#define RPC_H

#include "cport.h"

#define	RPC_CPORT			0
#define RPC_LCD_AND_CPORT	1

#define RPC_STARTCODE		0x1e3c5a78

#define RPC_PRINT			0x01
#define RPC_FOPEN			0x02
#define RPC_FCLOSE			0x03
#define RPC_FWRITE			0x04

#define RPC_OK				0
#define RPC_ERROR_TIMEOUT	-1
#define RPC_ERROR_CHECKSUM	-2

class CRPCtarget
	{
public:
	CRPCtarget(unsigned char mode=RPC_CPORT, unsigned long timeout=100000);

	short         Print(char *string);
	short         Printf(char *format, ...);

	unsigned char Fopen(char *filename, char *format);  
	short         Fclose(unsigned char fp);
	unsigned long Fwrite(void *a_ptr, unsigned long len, unsigned char fp);

	// add more!  

private:
	short FixedResponse(unsigned char *data, unsigned long len);
	// short VariableResponse();
	void SendHeader(unsigned char index, unsigned long len);
	void SendLong(unsigned long val);
	void SendShort(unsigned short val);
	void SendLength(unsigned long len);

	CCport m_cport;
	unsigned long m_timeout;
	unsigned long m_checksumErrors;
	unsigned char m_mode;
	};

#endif
