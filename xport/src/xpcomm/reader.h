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

#ifndef _READER_H
#define _READER_H

#include <fstream>
#include "property.h"

#define S0				0
#define S1				1
#define S2				2
#define S3				3
#define S5				5
#define S7				7
#define S8				8
#define S9				9

#define XPR_OFFSET_ROM	0x8000000

class IXPreader;
class istream;

IXPreader *CreateReader(char *filename);
void DestroyReader(IXPreader *preader);

class IXPreader // combined interface and base class
	{
	public:
	IXPreader()
		{
		m_pis = 0;
		m_addr = 0;
		}
	~IXPreader()
		{
		Close();
		}
	virtual int Open(std::ifstream *pis)
		{
		m_pis = pis;
		return RES_OK;
		}
	virtual int Open(char *filename)
		{
		m_pis = new std::ifstream(filename, std::ios::binary); //|ios::nocreate);
		if (m_pis->fail() || m_pis->eof())
			return RES_FAIL;
		return RES_OK;
		}
	virtual int Close() 
		{
		m_pis->close();
		return RES_OK;
		}
	virtual bool IsGameboyClassic()
		{
		return false;
		}
	virtual int Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len) = 0;
	
	protected:
	unsigned long m_addr;
	std::ifstream *m_pis;
	};

class CXPbinreader : public IXPreader
	{
	public:
	virtual int Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len);
	};

class CXPGBbinreader : public IXPreader
	{
	public:
	virtual int Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len);
	virtual bool IsGameboyClassic();
	};

class CXPsrecreader : public IXPreader
	{
	public:
	CXPsrecreader()
		{
		m_addrLast = 0;
		}
	virtual int Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len);

	private:
	int Hex2digit(char c);

	static const char typelut[];
	unsigned long m_addrLast;
	unsigned long m_length;
	char m_type;
	};

#if 0
class CXPuuencreader : public IXPreader
	{
	public:
	virtual int Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len);
	};
#endif

#endif // _READER_H
