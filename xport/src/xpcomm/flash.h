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


#ifndef _FLASH_H
#define _FLASH_H

#include "hal.h"
#include <istream>
#include <sstream>

#define XPF_DIR					PPORT_CONTROL, 3
#define XPF_CLOCK				PPORT_CONTROL, 2
#define XPF_RESET				PPORT_CONTROL, 0
#define XPF_READY				PPORT_STATUS,  1
#define XPF_CD0					PPORT_DATA, 0
#define XPF_BUSY				0x08
#define XPF_PROGRAMLED			2
#define XPF_ERRORLED			3

#define XPF_MODE_SET_ADDR  		0, 6
#define XPF_MODE_WRITE  		1, 10
#define XPF_MODE_PROGRAM		2, 4
#define XPF_MODE_PROGRAM_NEXT	0xff, 4
#define XPF_MODE_READ	  		3
#define XPF_MODE_READ_NEXT	  	0xff

#define XPF_XPORT_TO_HOST		XPF_DIR, 0
#define XPF_HOST_TO_XPORT		XPF_DIR, 1

#define XPF_TIMEOUT_PROGRAM		100000
#define XPF_TIMEOUT_ERASE		1000
#define XPF_SIZE_SECTOR			0x10000

class CXPhal;
class CXPadvise;

typedef unsigned  long long int u64;


// All methods in CXPflash deal with 16-bit words, addresses are such that 0=beginning of flash
class CXPflash : public CXPhal
	{
public:
	CXPflash();
	~CXPflash();

	int	Initialize(bool reset=true, bool serial=false);
	virtual int Program(unsigned long addr, unsigned short *data, unsigned long len, bool erase=false, bool header=true);
	virtual int Erase(unsigned long addr, unsigned long len);
	int Program(char *filename, unsigned long addr=0);
	int Verify(unsigned long addr, unsigned short *data, unsigned long len, bool header=true);
	int Verify(char *filename, unsigned long addr=0);
	int Read(unsigned long addr, unsigned short *data, unsigned long len);
	int Read(char *filename, unsigned long len, unsigned long addr=0);
	int Unlock(unsigned long addr);
	int Lock(unsigned long addr);
	int AddHeader(unsigned long addr, unsigned short *data, unsigned long len);
	int GetCode(unsigned short *code);

	unsigned long GetInteger(std::istream &is, int len=2);
	int GetType(std::istream &is);
	bool ScanForField(std::istream &is, unsigned char searchType);
	int ConfigureLogic(std::istream &is, bool reset);
	int ConfigureLogic(char *filename, bool reset);
	int ConfigureLogic(const unsigned char *data, unsigned long len, bool reset);
	int GetName(char *name);

	// IXPproperty methods
	virtual int SetProperty(EXPproperty property, unsigned long val);
	virtual int GetProperty(EXPproperty property, unsigned long *pval);

protected:
	unsigned short	Read(unsigned char mode, bool programLED=true, bool errorLED=false);
	void			Write(unsigned char mode, int size, u64 data, bool programLED=true, bool errorLED=false);
	unsigned char	Bitcount(unsigned short data);
	void			ResetLogic();
	inline void		ResetFlash()
		{
		Write(XPF_MODE_WRITE, (u64)0xff, false);
		Write(XPF_MODE_WRITE, (u64)0x50, false);
		}
	inline unsigned short ReadAddr(unsigned long addr)
		{
		Write(XPF_MODE_SET_ADDR, (u64)addr);
		return Read(XPF_MODE_READ);
		}
	inline unsigned short ReadNext()
		{
		return Read(XPF_MODE_READ_NEXT);
		}
	inline void WriteAddr(unsigned long addr, unsigned short data)
		{
		Write(XPF_MODE_WRITE, ((u64)addr<<16)|(u64)data);
		}
	inline void SetError()
		{
		ResetLogic();
		Write(0, 0, 0, false, true);
		}

	CXPadvise		*m_padvise;
	unsigned long	m_sizeSector;	// words
	bool			m_configured;
	CXPdebug		m_debug;
	char			*m_filename;
	};

class CXPadvise
	{
public:
	CXPadvise(unsigned long size);
	~CXPadvise();

	void Program(unsigned long addr, unsigned long len);
	bool ProgramAdvise(unsigned long addr, unsigned long len);
	void Erase(unsigned long addr, unsigned long len);
	bool EraseAdvise(unsigned long addr, unsigned long len);
	void Reset();

private:
	unsigned long m_sizeDevice;
	unsigned long m_sizeSector;
	bool *m_mapErase;	
	bool *m_mapProgram;
	};

#endif //_FLASH_H
