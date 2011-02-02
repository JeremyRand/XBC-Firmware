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


#include <stdio.h>
#include <assert.h>
#include "hal.h"
#include "flash.h"
#include "reader.h"
#include <sstream>

const unsigned char GBAheader[] = 
	{
	0x2e, 0x00, 0x00, 0xea, 0x24, 0xff, 0xae, 0x51, //00
	0x69, 0x9a, 0xa2, 0x21,	0x3d, 0x84, 0x82, 0x0a, 
	0x84, 0xe4, 0x09, 0xad, 0x11, 0x24, 0x8b, 0x98, //10 
	0xc0, 0x81, 0x7f, 0x21,	0xa3, 0x52, 0xbe, 0x19,
	0x93, 0x09, 0xce, 0x20, 0x10, 0x46, 0x4a, 0x4a, //20
	0xf8, 0x27, 0x31, 0xec, 0x58, 0xc7, 0xe8, 0x33, 
	0x82, 0xe3, 0xce, 0xbf, 0x85, 0xf4, 0xdf, 0x94, //30 
	0xce, 0x4b, 0x09, 0xc1, 0x94, 0x56, 0x8a, 0xc0, 
	0x13, 0x72, 0xa7, 0xfc, 0x9f, 0x84, 0x4d, 0x73, //40 
	0xa3, 0xca, 0x9a, 0x61, 0x58, 0x97, 0xa3, 0x27, 
	0xfc, 0x03, 0x98, 0x76, 0x23, 0x1d, 0xc7, 0x61, //50 
	0x03, 0x04, 0xae, 0x56, 0xbf, 0x38, 0x84, 0x00, 
	0x40, 0xa7, 0x0e, 0xfd, 0xff, 0x52, 0xfe, 0x03, //60 
	0x6f, 0x95, 0x30, 0xf1, 0x97, 0xfb, 0xc0, 0x85, 
	0x60, 0xd6, 0x80, 0x25, 0xa9, 0x63, 0xbe, 0x03, //70 
	0x01, 0x4e, 0x38, 0xe2, 0xf9, 0xa2, 0x34, 0xff, 
	0xbb, 0x3e, 0x03, 0x44, 0x78, 0x00, 0x90, 0xcb, //80 
	0x88, 0x11, 0x3a, 0x94, 0x65, 0xc0, 0x7c, 0x63, 
	0x87, 0xf0, 0x3c, 0xaf, 0xd6, 0x25, 0xe4, 0x8b, //90 
	0x38, 0x0a, 0xac, 0x72, 0xa5, 0xd4, 0xf8, 0x07, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //a0 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x30, 0x31, 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, //b0 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00
	};

CXPflash::CXPflash()
	{
	m_sizeSector = XPF_SIZE_SECTOR;
	m_filename = NULL;
	m_padvise = 0;
	}

int CXPflash::Initialize(bool reset, bool serial)
	{
	int i, res;

	if ((res=CXPhal::Initialize())!=RES_OK)
		return res;

	if (serial)
		{
		// force reset early to read model number
		Reset(reset, RESET_LOGIC_SERIAL);
		m_configured = ConfigureSerial(Progbit(), ProgbitSize(), false)==RES_OK;
		}
	else 
		m_configured = Reset(reset, RESET_LOGIC_PC1)==RES_OK;

	m_padvise = new CXPadvise(SizeFlash());

	ResetLogic();
	// clear out write buffer (in case we were interrupted in the middle)
	for (i=0; i<16; i++)
		Write(XPF_MODE_WRITE, (u64)0, false);
	ResetLogic();
	ResetFlash();
	return m_configured ? RES_OK : RES_FAIL;
	}

CXPflash::~CXPflash()
	{
	unsigned long sector;

	if (m_padvise)
		{
		// lock sectors that we wrote to
		for (sector=0; sector<SizeFlash(); sector+=m_sizeSector)
			{
			if (m_padvise->EraseAdvise(sector, m_sizeSector)==false)
				{
				m_debug.printf(2, "Locking sector 0x%x\n", sector);
				Lock(sector);
				}
			}
		delete m_padvise;
		}
	}

unsigned short CXPflash::Read(unsigned char mode, bool programLED, bool errorLED)
	{
	int i;
	unsigned short temp, dataRead = 0;

	if (mode != 0xff)
		{
		HostToXport();
		CXPhal::Write(PPORT_DATA, mode&0x03 | programLED<<XPF_PROGRAMLED | errorLED<<XPF_ERRORLED);
		ToggleBit(XPF_CLOCK);
		}

	XportToHost();
	for (i=0; i<4; i++)
		{
		ToggleBit(XPF_CLOCK);
		temp = CXPhal::Read(PPORT_DATA)&0x0f;
		temp <<= i<<2;
		dataRead |= temp;
		}
	return dataRead;
	}

void CXPflash::Write(unsigned char mode, int size, u64 data, bool programLED, bool errorLED)
	{
	int i;

	HostToXport();
	if (mode != 0xff)
		{
		CXPhal::Write(PPORT_DATA, mode&0x03 | programLED<<XPF_PROGRAMLED | errorLED<<XPF_ERRORLED);
		ToggleBit(XPF_CLOCK);
		}

	for (i=0; i<size; i++)
		{
		CXPhal::Write(PPORT_DATA, (unsigned char)data&0x0f);
		ToggleBit(XPF_CLOCK);
		data >>= 4;
		}
	}

void CXPflash::ResetLogic()
	{
	SetNC0();
	WriteBit(XPF_RESET, 1);
	WriteBit(XPF_RESET, 0);
	}

unsigned char CXPflash::Bitcount(unsigned short data)
	{
	unsigned char res = 0;
	int i;

	for (i=0; i<16; i++)
		{
		res += data&0x01;
		data >>= 1;
		}
	return res;
	}

int CXPflash::Program(unsigned long addr, unsigned short *data, unsigned long len, bool erase, bool header)
	{
	unsigned short currData;
	unsigned long i, j, currAddr;
	unsigned char res, count = 0;
	
	assert(m_configured);

	if (addr+len > SizeFlash())
		{
		m_debug.printf(0, "Program operation exceeds size of of flash.\n");
		return RES_FAIL;
		}

	// erase
	if (erase)
		{
		int ires;
		ires = Erase(addr, len);
		if (ires!=RES_OK && ires!=RES_DENY)
			return ires;
		}

	// programming memory that hasn't been erased is incorrect
	assert(m_padvise->ProgramAdvise(addr, len));  
	m_padvise->Program(addr, len);

	ResetLogic();
	ResetFlash();

	// copy GBA header if it doesn't exist
	if (header)
		AddHeader(addr, data, len);

	for (i=0, currAddr=(addr+0xf)&~0xf; currAddr<((addr+len)&~0xf); currAddr++, i++)
		{
		currData = data[currAddr-addr];
		if (i==0)
			{
			Write(XPF_MODE_SET_ADDR, (u64)currAddr);
			Write(XPF_MODE_PROGRAM, (u64)currData);
			}
		else 
			Write(XPF_MODE_PROGRAM_NEXT, (u64)currData);

		// checksum
		count += Bitcount(currData)+1;

		// wait for completion every 16 writes
		if ((currAddr&0xf)==0xf)
			{
			for (j=0, res=XPF_BUSY; j<XPF_TIMEOUT_PROGRAM && !ReadBit(XPF_READY); j++);

			XportToHost();
			res = CXPhal::Read(PPORT_DATA)&0x0f;
			if (res!=(count&0x0f) || j==XPF_TIMEOUT_PROGRAM)
				{
				m_debug.printf(2, "Checksum error 0x%x 0x%x 0x%x\n", currAddr, res, count&0x0f);
				SetError();
				return RES_FAIL;
				}
			}
		if (CXPhal::m_break)
			break;
		}

	ResetLogic(); 
	ResetFlash();

	if (CXPhal::m_break)
		throw(EXCEPT_CTRL_C);

	// program and verify remaining
	for (currAddr=addr; currAddr<((addr+0xf)&~0xf); currAddr++)
		{
		currData = data[currAddr-addr];
		WriteAddr(currAddr, 0x40);
		WriteAddr(currAddr, currData);
		ReadAddr(0);
		for (j=0, res=XPF_BUSY; j<XPF_TIMEOUT_PROGRAM && !(ReadNext()&0x0080); j++);
		ResetLogic();
		ResetFlash();
		if (ReadAddr(currAddr)!=currData || j==XPF_TIMEOUT_PROGRAM)
			{
			m_debug.printf(2, "Verify 0 failed\n");
			SetError();
			return RES_FAIL;
			}
		ResetLogic();
		}
	for (currAddr=(addr+len)&~0xf; currAddr<(addr+len); currAddr++)
		{
		currData = data[currAddr-addr];
		WriteAddr(currAddr, 0x40);
		WriteAddr(currAddr, currData);
		ReadAddr(0);
		for (j=0, res=XPF_BUSY; j<XPF_TIMEOUT_PROGRAM && !(ReadNext()&0x0080); j++);
		ResetLogic();
		ResetFlash();
		if (ReadAddr(currAddr)!=currData || j==XPF_TIMEOUT_PROGRAM)
			{
			m_debug.printf(2, "Verify 1 failed\n");
			SetError();
			return RES_FAIL;
			}
		ResetLogic();
		}

	return RES_OK;
	}

int CXPflash::Program(char *filename, unsigned long addr)
	{
	unsigned short *data;
	unsigned long currAddr, len;
	int res = RES_OK;
	bool header;

	IXPreader *preader = CreateReader(filename);
	if (preader==NULL)
		{
		m_debug.printf(0, "File format not supported or cannot open file.\n");
		return RES_FAIL;
		}

	header = !preader->IsGameboyClassic();

	// Testing by Jeremy; probably will screw stuff up... yep, didn't fix a thing :-(
	//if(addr > 0)
	//	header = false;

	m_debug.printf(1, "Programming flash (%s)\n", filename);

	m_filename = filename;

	data = new unsigned short[m_sizeSector];
	while(res==RES_OK && !CXPhal::m_break)
		{
		res = preader->Read(data, m_sizeSector, &currAddr, &len);
		currAddr &= SizeFlash()-1;
		if (res==RES_OK || res==RES_END)
			{
			m_debug.printf(2, "Erasing 0x%x -> 0x%x..", currAddr+addr, currAddr+addr+len);
			m_debug.printf(1, ".");
			m_debug.printf(2, "\n");
			if (Erase(currAddr+addr, len)==RES_FAIL)
				{
				res = RES_FAIL;
				break;
				}

			m_debug.printf(2, "Programming 0x%x -> 0x%x..", currAddr+addr, currAddr+addr+len);
			m_debug.printf(1, ".");
			m_debug.printf(2, "\n");
			if (Program(currAddr+addr, data, len, false, header)!=RES_OK)
				{
				m_debug.printf(0, "\nFailed.\n");
				res = RES_FAIL;
				break;
				}
			}
		}
	
	DestroyReader(preader);
	delete [] data;

	if (CXPhal::m_break)
		throw(EXCEPT_CTRL_C);
	m_filename = NULL;

	if (res==RES_END) // completed successfully
		{
		m_debug.printf(1, "\nSuccess.\n");
		return RES_OK;
		}

	return RES_FAIL;
	}

int CXPflash::Verify(unsigned long addr, unsigned short *data, unsigned long len, bool header)
	{
	unsigned long i, currAddr;
	
	assert(m_configured);

	ResetLogic();
	ResetFlash();

	// copy GBA header if it doesn't exist
	if (header)
		AddHeader(addr, data, len);

	for (i=0, currAddr=addr; currAddr<(addr+len); currAddr++, i++)
		{
		if (i==0)
			{
			if (ReadAddr(currAddr)!=data[i])
				{
				SetError();
				return RES_FAIL;
				}
			}
		else if (ReadNext()!=data[i])
			{
			SetError();
			return RES_FAIL;
			}
		if (CXPhal::m_break)
			break;
		}

	ResetLogic();
	if (CXPhal::m_break)
		throw(EXCEPT_CTRL_C);

	return RES_OK;
	}

int CXPflash::Verify(char *filename, unsigned long addr)
	{
	unsigned short *data;
	unsigned long currAddr, len;
	int res = RES_OK;
	bool header	;

	IXPreader *preader = CreateReader(filename);
	if (preader==NULL)
		{
		m_debug.printf(0, "File format not supported or cannot open file.\n");
		return RES_FAIL;
		}

	header = !preader->IsGameboyClassic();
	m_debug.printf(1, "Verifying flash (%s)\n", filename);
	m_filename = filename;

	data = new unsigned short[m_sizeSector];
	while(res==RES_OK && !CXPhal::m_break)
		{
		res = preader->Read(data, m_sizeSector, &currAddr, &len);
		currAddr &= SizeFlash()-1;
		if (res==RES_OK || res==RES_END)
			{
			m_debug.printf(2, "Verify 0x%x -> 0x%x..", currAddr+addr, currAddr+addr+len);
			m_debug.printf(1, ".");
			m_debug.printf(2, "\n");
			if (Verify(currAddr+addr, data, len, header)!=RES_OK)
				{
				m_debug.printf(0, "\nFailed.\n");
				res = RES_FAIL;
				break;
				}
			}
		}
	
	DestroyReader(preader);
	delete [] data;  

	if (CXPhal::m_break)
		throw(EXCEPT_CTRL_C);
	if (res==RES_END) // completed successfully
		{
		m_debug.printf(1, "\nSuccess.\n");
		return RES_OK;
		}

	return RES_OK;
	}

int CXPflash::Read(unsigned long addr, unsigned short *data, unsigned long len)
	{
	unsigned long i, currAddr;
	
	assert(m_configured);

	ResetLogic();
	ResetFlash();

	for (i=0, currAddr=addr; currAddr<(addr+len); currAddr++, i++)
		{
		if (i==0)
			data[i] = ReadAddr(currAddr);
		else 
			data[i] = ReadNext();
		if (CXPhal::m_break)
			break;
		}

	ResetLogic();
	if (CXPhal::m_break)
		throw(EXCEPT_CTRL_C);

	return RES_OK;
	}

int CXPflash::Read(char *filename, unsigned long len, unsigned long addr)
	{
	unsigned short *dataBuf;
	unsigned long currAddr;

	std::ofstream of(filename, std::ios::binary);

	if (of.fail())
		return RES_FAIL;

	m_debug.printf(1, "Reading flash (%s)\n", filename);

	dataBuf = new unsigned short[m_sizeSector];

	for (currAddr=addr; currAddr<(addr+len); currAddr+=m_sizeSector)
		{
		m_debug.printf(2, "Read 0x%x -> 0x%x..", currAddr, currAddr+m_sizeSector);
		m_debug.printf(1, ".");
		m_debug.printf(2, "\n");
		Read(currAddr, dataBuf, m_sizeSector);
		of.write((char *)dataBuf, m_sizeSector*sizeof(unsigned short));
		}
	if (addr+len-currAddr)
		{
		m_debug.printf(2, "Read 0x%x -> 0x%x..", currAddr, addr+len);
		m_debug.printf(1, ".");
		m_debug.printf(2, "\n");
		Read(currAddr, dataBuf, addr+len-currAddr);
		of.write((char *)dataBuf, (addr+len-currAddr)*sizeof(unsigned short));
		}

	of.close();
	delete [] dataBuf;
	return RES_OK;
	}

int CXPflash::Erase(unsigned long addr, unsigned long len)
	{
	unsigned long sector, endAddr;
	int i, res = RES_OK;

	assert(m_configured);

	if (addr+len > SizeFlash())
		{
		m_debug.printf(0, "Erase operation exceeds size of of flash.\n");
		return RES_FAIL;
		}

	ResetLogic();
	ResetFlash();

	endAddr = addr+len;
	addr &= ~(m_sizeSector-1);
	endAddr += m_sizeSector-1;
	endAddr &= ~(m_sizeSector-1);

	for (sector=addr; sector<endAddr; sector+=m_sizeSector)
		{
		// don't erase over something we've already programmed
		if (!m_padvise->EraseAdvise(sector, m_sizeSector))	
			{
			res = RES_DENY;
			continue;
			}
		m_padvise->Erase(sector, m_sizeSector);
		Unlock(sector);
		WriteAddr(sector, 0x20);
		WriteAddr(sector, 0xd0);
		ReadAddr(0);
		for (i=0; i<XPF_TIMEOUT_ERASE && !(ReadNext()&0x0080); i++)
			Delay(10);
		if (i==XPF_TIMEOUT_ERASE)
			return RES_FAIL;
		ResetLogic();
		if (CXPhal::m_break)
			break;
		}

	if (CXPhal::m_break)
		throw(EXCEPT_CTRL_C);

	ResetFlash();

	return res;
	}

int CXPflash::Unlock(unsigned long addr)
	{
	int i;

    WriteAddr(addr, 0x60);
    WriteAddr(addr, 0xD0);
	ReadAddr(0);
	for (i=0; i<XPF_TIMEOUT_PROGRAM && !(ReadNext()&0x0080); i++);
	if (i==XPF_TIMEOUT_PROGRAM)
		return RES_FAIL;

	ResetLogic();
	ResetFlash();
	return RES_OK;
	}

int CXPflash::Lock(unsigned long addr)
	{
	int i;

    WriteAddr(addr, 0x60);
    WriteAddr(addr, 0x01);
	ReadAddr(0);
	for (i=0; i<XPF_TIMEOUT_PROGRAM && !(ReadNext()&0x0080); i++);
	if (i==XPF_TIMEOUT_PROGRAM)
		return RES_FAIL;

	ResetLogic();
	ResetFlash();
	return RES_OK;
	}

int CXPflash::GetCode(unsigned short *code)
	{
	unsigned short manufacturer, size;

	Write(XPF_MODE_WRITE, (u64)0x90, false);
	manufacturer = ReadAddr(0);
	ResetLogic();
	Write(XPF_MODE_WRITE, (u64)0x90, false);
	size = ReadAddr(1);
	ResetLogic();
	ResetFlash();
	*code = manufacturer<<8 | size;

	return RES_OK;
	}

int CXPflash::AddHeader(unsigned long addr, unsigned short *data, unsigned long len)
	{
	int i, j;
	char buf[64];
	char *name = buf;
	unsigned char sum;

	m_debug.printf(1, "AddHeader\n");

	if (addr<=0 && ((addr+len)>=(sizeof(GBAheader)/sizeof(unsigned short))) && // lies within header region?
		*((unsigned char *)data+4)!=GBAheader[4]) // check for existing header
		{
		memcpy((void *)data, (void *)GBAheader, sizeof(GBAheader));
		if (m_filename)
			{
			for (i=strlen(m_filename); i>=0; i--)
				{
				if (m_filename[i]=='\\' || m_filename[i]=='/')
					break;
				name[i] = m_filename[i];
				}
			name += i+1;
			for (i=0; i<0x12 && name[i]; i++)
				*((unsigned char *)data+0xa0+i) = name[i];
			for (i=0xa0, sum=0x19; i<0xbd; sum+=*((unsigned char *)data+i), i++);
			sum = -sum;
			*((unsigned char *)data+0xbd) = sum;
			}
		}
	return RES_OK;
	}

int CXPflash::GetName(char *name)
	{
	Read(0xa0>>1, (unsigned short *)name, 6);
	name[12] = '\0';

	return RES_OK;
	}


int CXPflash::SetProperty(EXPproperty property, unsigned long val)
	{
	int res;

	res = CXPhal::SetProperty(property, val);
	switch(property)
		{
		case PROP_LEVEL_DEBUG:
			m_debug.SetLevel(val);
			res = RES_OK;
			break;
		}
	return res;
	}

int CXPflash::GetProperty(EXPproperty property, unsigned long *pval)
	{
	return CXPhal::GetProperty(property, pval);
	}

CXPadvise::CXPadvise(unsigned long size)
	{
	this->m_sizeDevice = size;
	this->m_sizeSector = XPF_SIZE_SECTOR;

	m_mapErase = new bool[m_sizeDevice/m_sizeSector];
	m_mapProgram = new bool[m_sizeDevice/m_sizeSector];
	Reset();
	}

CXPadvise::~CXPadvise()
	{
	delete [] m_mapErase;
	delete [] m_mapProgram;
	}

void CXPadvise::Reset()
	{
	memset(m_mapErase, 0, m_sizeDevice/m_sizeSector*sizeof(bool));
	memset(m_mapProgram, 0, m_sizeDevice/m_sizeSector*sizeof(bool));
	}

void CXPadvise::Program(unsigned long addr, unsigned long len)
	{
	unsigned long currAddr;

	for (currAddr=addr; currAddr<(addr+len); currAddr+=m_sizeSector)
		m_mapProgram[currAddr/m_sizeSector] = true;
	}

bool CXPadvise::ProgramAdvise(unsigned long addr, unsigned long len)
	{
	// only program if erased
	unsigned long currAddr;

	for (currAddr=addr; currAddr<(addr+len); currAddr+=m_sizeSector)
		if (m_mapErase[currAddr/m_sizeSector]==false)
			return false;

	return true;
	}

void CXPadvise::Erase(unsigned long addr, unsigned long len)
	{
	unsigned long currAddr;

	for (currAddr=addr; currAddr<(addr+len); currAddr+=m_sizeSector)
		m_mapErase[currAddr/m_sizeSector] = true;
	}

bool CXPadvise::EraseAdvise(unsigned long addr, unsigned long len)
	{
	// only erase if not programmed
	unsigned long currAddr;

	for (currAddr=addr; currAddr<(addr+len); currAddr+=m_sizeSector)
		if (m_mapProgram[currAddr/m_sizeSector]==true)
			return false;

	return true;
	}


