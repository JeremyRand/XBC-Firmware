//
// pport.cxx
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


#include <stdio.h>
#include <string.h>
#include "pport.h"



unsigned long  CXPpport::m_refcount = 0;
unsigned short CXPpport::m_portAddress = PPORT_DEFAULT_ADDRESS;

unsigned long (*CXPpport::InitializeWinIo)(char) = 0;
void (*CXPpport::ShutdownWinIo)(void) = 0;
bool (*CXPpport::GetParallelPortAddr)(unsigned long *) = 0;

CXPpport::CXPpport() 
	{	
	}

int CXPpport::Open(unsigned char parallelPort)
	{			
	
	char dev_name_str [16] = {0};
	char dev_num_str [2] = {0};
	
	if (parallelPort>9)
		return -1;
	
	strcpy(dev_name_str, "/dev/parport");
	
	dev_num_str[0] = '0' + parallelPort;
	
	strcat(dev_name_str, dev_num_str);
	
	
	pport_fd = open(dev_name_str, O_RDWR);
	if (pport_fd == -1)
		{
		perror("open");
		return -1;
		}
	
	if (ioctl(pport_fd,PPCLAIM))
		{
		perror("PPCLAIM");
		close(pport_fd);
		return -1;
		}
	
	return 0; // success
	}

void CXPpport::Close()
	{		
	ioctl (pport_fd, PPRELEASE);
	
	close(pport_fd);
	}

CXPpport::~CXPpport()
	{
	Close();
	}
