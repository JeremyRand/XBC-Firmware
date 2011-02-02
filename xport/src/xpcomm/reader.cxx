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

#include "reader.h"
#include <sstream>
#include <assert.h>
#include <stdio.h>

#define GETCHAR(c) \
	pifs->read(&c, 1); \
    if (pifs->gcount()!=1) \
		return NULL;

IXPreader *CreateReader(char *filename)
	{
	char buf[0x200];
	int i;
	IXPreader *pread = NULL;
	char c;
	std::ifstream *pifs;

	pifs = new std::ifstream(filename, std::ios::binary); //|ios::nocreate);
	if (pifs->fail() || pifs->eof())
		return NULL;
	
	for (i=0; i<0x200; i++)
		{
		GETCHAR(c);
		buf[i] = c;
		}

	if (buf[0]==0x2e || buf[0]==0x37)
		pread = new CXPbinreader;
	else if ((unsigned char)buf[0x104]==0xce && (unsigned char)buf[0x105]==0xed)
		pread = new CXPGBbinreader;
	else
		{
		i = 0;
		while(buf[i]=='\t' || buf[i]=='\r' || buf[i]=='\n' || buf[i]==' ')
			i++;
		if (buf[i]=='S')
			pread = new CXPsrecreader;
		}

	pifs->close();
	if (pread && pread->Open(filename)==RES_OK)
		return pread;
	return NULL;
	}

#undef GETCHAR

void DestroyReader(IXPreader *preader)
	{
	if (preader)
		{
		preader->Close();
		delete preader;
		}
	}

int CXPbinreader::Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len)
	{
	unsigned long readNum;

	m_pis->read((char *)dataBuf, sizeBuf*sizeof(unsigned short));
	
	readNum = m_pis->gcount()/sizeof(unsigned short);
	*addr = m_addr;
	*len = readNum;
	m_addr += readNum;

	if (m_pis->peek()==EOF)
		return RES_END;
	else
		return RES_OK;
	}

bool CXPGBbinreader::IsGameboyClassic()
	{
	return true;
	}

int CXPGBbinreader::Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len)
	{
	unsigned long readNum, i;
	unsigned char buf[0x10000];

	m_pis->read((char *)buf, sizeBuf);
	
	readNum = m_pis->gcount();
	for (i=0; i<readNum; i++)
		dataBuf[i] = ((unsigned short)buf[i]<<8) | buf[i];

	*addr = m_addr;
	*len = readNum;
	m_addr += readNum;

	if (m_pis->peek()==EOF)
		return RES_END;
	else
		return RES_OK;
	}

int CXPsrecreader::Hex2digit(char c)
	{
    if(c&0x40) 
		c += 9;
    return c&0x0f;
	}

const char CXPsrecreader::typelut[] =
	{
	/* S0 */ 0, /* S1 */ 4, /* S2 */ 6, /* S3 */ 8,
	/* S4 */ 0, /* S5 */ 0, /* S6 */ 0, /* S7 */ 8, 
	/* S8 */ 6, /* S9 */ 4
	};
		
#define GETCHAR(c) \
	m_pis->read(&c, 1); \
    if (m_pis->gcount()!=1) \
		return RES_FAIL;

int CXPsrecreader::Read(unsigned short *dataBuf, unsigned long sizeBuf, unsigned long *addr, unsigned long *len)
	{
    unsigned char chksum, ochksum, val, *buf;
	char c, len0, len1;
	bool first = true;
	unsigned long i, addrsize, size, offset = 0;
	int res = RES_OK;
	
	size = sizeBuf*sizeof(unsigned short);	
    buf = (unsigned char *)dataBuf;

    while(true)
		{
        if (!m_addr)	// if we've read a previous address, resume where we left off
			{
	        // Skip whitespace characters until we find an S
			do 
				{
				GETCHAR(c);
				} while(c=='\r' || c=='\n' || c==' ');
				
			// Check that this is an S record
			if(c!='S') 
				{
				printf("Invalid S-record\n");
				return false;
				}
			
			// First 4 bytes are standard S + type + len
			GETCHAR(m_type);
			GETCHAR(len0);
			GETCHAR(len1);
			
			m_type = Hex2digit(m_type);
			addrsize = typelut[m_type];
			
			m_length = Hex2digit(len0)<<4;
			m_length |= Hex2digit(len1);
			chksum = (unsigned char)m_length;
			
			// read the address
			if (m_addr==0)
				{
				for (i=0; i<addrsize; i++) 
					{
					GETCHAR(c);
					val = Hex2digit(c);
					m_addr = (m_addr<<4) | val;
					}
				}
			}
		else	// resume where we left off
			{
			addrsize = typelut[m_type];
			chksum = (unsigned char)m_length;
			}
		
		assert(m_type==S0 || m_addr>=XPR_OFFSET_ROM);

		// discontinuity, return what we have so far
		if (m_type!=S7 && m_addrLast && m_addr!=(unsigned long)(m_addrLast+1))
			break;

		// calculate the checksum, which is done by the byte, not the digit
		for (i=0; i<addrsize*4; i+=8) 
			chksum += (unsigned char)((m_addr>>i) & 0xff);
		
		// decide where to load this data
		if (first && (m_type!=S0)) 
			{
			*addr = (m_addr-XPR_OFFSET_ROM)/sizeof(unsigned short);
			first = false;
			}
		
		// read the data and put it directly into memory where it belongs
		val = 0;
		for (i=0; i<((m_length-1)*2)-addrsize; i+=2) 
			{
			GETCHAR(c);
			val = Hex2digit(c) << 4;
			GETCHAR(c);
			val |= Hex2digit(c);
			chksum += val;
			if(m_type!=S0) 
				{
				*buf++ = val;
				offset++;
				}
			}
		
		if (m_type!=S0)
			m_addrLast = m_addr+m_length-addrsize/2-2;
		m_addr = 0;

		// now get the old checksum
		GETCHAR(c);
		ochksum = Hex2digit(c) << 4;
		GETCHAR(c);
		ochksum |= Hex2digit(c);
		chksum = ~chksum;
		if (chksum!=ochksum) 
			{
			printf("Bad S-record (checksum error)\n");
			return RES_FAIL;
			}
		else if (m_type==S7)
			{
			res = RES_END;
			break;
			}
		if ((offset+m_length+256)>size && !(offset&1))
			break;
		} 

	assert(!(offset&1));	// no halfwords

	*len = offset/sizeof(unsigned short);
	m_addrLast = 0;
	return res;
	}

