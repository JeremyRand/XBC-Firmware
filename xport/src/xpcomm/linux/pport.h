//
// pport.h
// Copyright (C) 2003 Charmed Labs LLC
//
// This file is part of The Xport Software Distribution
// 
// The Xport Software Distribution is free software; you can redistribute it and/or 
// modify it under the terms of the GNU General Public License as published 
// by the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// The Xport Software Distribution is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with The Xport Software Distribution; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// Log:
// Revision 1.0: 4/16/03 by Rich LeGrand (rich@charmedlabs.com)
// Initial release
//

#ifndef _PPORT_H
#define _PPORT_H



#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <fcntl.h>

typedef unsigned int HINSTANCE;

#define PPORT_DATA		0	// data register address offset
#define PPORT_STATUS	1	// status register address offset
#define PPORT_CONTROL	2	// control register address offset

#define PPORT_READ_CONSISTENCY  1 
#define PPORT_DEFAULT_ADDRESS	0x378


class CXPpport
	{
	private:
	bool m_nt;	// true if NT4, Win2K, WinXP
	//HINSTANCE m_dllHandle;
	int pport_fd;

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
		 char c;
		 int data_direction = 1;
		 
		 ioctl (pport_fd, PPDATADIR, &data_direction);
		 
		  switch (port)
		  {
		   case PPORT_DATA:		  	
		  	ioctl (pport_fd,PPRDATA, &c);			
			break;			
		   case PPORT_STATUS:		   
		    	ioctl (pport_fd,PPRSTATUS, &c);			
		   	break;
		   case PPORT_CONTROL:
			ioctl (pport_fd,PPRCONTROL, &c);				   
		    	break;		   
		  	}		 
		 return c;
		}

	inline void Write(unsigned short port, unsigned char val)
		{
		  int data_direction = 0;
		  
		  ioctl (pport_fd,PPDATADIR, &data_direction);
		  switch (port)
		  {
		   case PPORT_DATA:		  	
			ioctl (pport_fd,PPWDATA,&val);						
			break;			
		   case PPORT_CONTROL:
		   	ioctl (pport_fd,PPWCONTROL,&val);		   
		    	break;		   
		   }		
		}
	};


#endif
