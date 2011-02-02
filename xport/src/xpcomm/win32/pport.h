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

#ifndef _PPORT_H
#define _PPORT_H

#include <wtypes.h>

#define PPORT_DATA		0	// data register address offset
#define PPORT_STATUS	1	// status register address offset
#define PPORT_CONTROL	2	// control register address offset

#define PPORT_READ_CONSISTENCY  1 
#define PPORT_DEFAULT_ADDRESS	0x378

static __inline unsigned char
inb (unsigned short int port)
{
  unsigned char _v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (_v):"Nd" (port));
  return _v;
}

static __inline void
outb (unsigned char value, unsigned short int port)
{
  __asm__ __volatile__ ("outb %b0,%w1": :"a" (value), "Nd" (port));
}


class CXPpport
	{
	private:
	bool m_nt;	// true if NT4, Win2K, WinXP
	HINSTANCE m_dllHandle;

	static unsigned long m_refcount;
	static unsigned short m_portAddress;

	static unsigned long (*InitializeWinIo)(char);
	static void (*ShutdownWinIo)(void);
	static bool (*GetParallelPortAddr)(unsigned long *);

	public:
	CXPpport(); 
	~CXPpport();
	int Open(unsigned char parallelPort);
	void Close();

	inline void SetAddress(unsigned short addr) 
		{
		m_portAddress = addr;
		}

	inline unsigned short GetAddress()
		{
		return m_portAddress;
		}

	inline unsigned char Read(unsigned short port)
		{
		return inb(m_portAddress + port);
		}

	inline void Write(unsigned short port, unsigned char val)
		{
		outb(val, m_portAddress + port);
		}
	};


#endif
