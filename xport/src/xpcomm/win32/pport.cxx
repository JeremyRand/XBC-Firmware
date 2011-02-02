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

#include <windows.h>
#include <stdio.h>
#include "pport.h"

unsigned long  CXPpport::m_refcount = 0;
unsigned short CXPpport::m_portAddress = PPORT_DEFAULT_ADDRESS;

unsigned long (*CXPpport::InitializeWinIo)(char) = 0;
void (*CXPpport::ShutdownWinIo)(void) = 0;
bool (*CXPpport::GetParallelPortAddr)(unsigned long *) = 0;

CXPpport::CXPpport() 
	{
	OSVERSIONINFO osvi;
	
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if(GetVersionEx(&osvi))
		m_nt = osvi.dwPlatformId==VER_PLATFORM_WIN32_NT;
	}

int CXPpport::Open(unsigned char parallelPort)
	{
	DWORD dwAddr;
	DWORD res;
	int i;

	if (parallelPort>7)
		return -1;
	if (m_nt)
		{
		if (m_refcount==0 )
			{
			m_dllHandle = LoadLibrary("winio.dll");

			if (m_dllHandle==0)
				{
				fprintf(stderr, "ERROR: WinIo.dll is not found.\n");
				return -1;
				}

			InitializeWinIo = (unsigned long (*)(char))GetProcAddress(m_dllHandle, "InitializeWinIo");
			ShutdownWinIo = (void (*)(void))GetProcAddress(m_dllHandle, "InitializeWinIo");
			GetParallelPortAddr = (bool (*)(unsigned long *))GetProcAddress(m_dllHandle, "GetParallelPortAddr");

			if (InitializeWinIo==0 || ShutdownWinIo==0 || GetParallelPortAddr==0)
				return -2;

			for (i=0, res=ERROR_SUCCESS+1; res!=ERROR_SUCCESS && i<20; res=InitializeWinIo(parallelPort));
			if (res!=ERROR_SUCCESS)
				return res;
			if (GetParallelPortAddr(&dwAddr))
				m_portAddress = (unsigned short)dwAddr;
			}
		m_refcount++;
		}

	// Set PS2 mode (assuming port is configured ECP)
	Write(0x402, 0x34);

	return 0; // success
	}

void CXPpport::Close()
	{
	if (m_nt && m_refcount>0)
		{
		m_refcount--;
		if (m_refcount==0)
			{
			if (ShutdownWinIo)
				ShutdownWinIo();
			FreeLibrary(m_dllHandle);
			}
		}
	}

CXPpport::~CXPpport()
	{
	Close();
	}
