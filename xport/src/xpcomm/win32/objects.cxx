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
#include <sys/unistd.h>
#include <sys/cygwin.h>
#include <sys/signal.h>
#include <dirent.h>
#include "objects.h"
#include <signal.h>
#include <stdio.h>

Cmutex::Cmutex()
	{
	InitializeCriticalSection(&m_mutex);
	}

Cmutex::~Cmutex()
	{
	DeleteCriticalSection(&m_mutex);
	}

CautoMutex::CautoMutex(Cmutex *pmutex)
	{
	m_pmutex = pmutex;
	m_pmutex->Wait();
	}

CautoMutex::~CautoMutex()
	{
	m_pmutex->Release();
	}

void Cmutex::Wait()
	{
	EnterCriticalSection(&m_mutex);
	}

void Cmutex::Release()
	{
	LeaveCriticalSection(&m_mutex);
	}

Csignal::Csignal()
	{
	m_signal = CreateEvent(NULL, FALSE, FALSE, NULL);
	}

Csignal::~Csignal()
	{
	CloseHandle(m_signal);
	}

void Csignal::Wait()
	{
	WaitForSingleObject(m_signal, INFINITE);
	}

void Csignal::Signal()
	{
	SetEvent(m_signal);
	}

void (*Cplatform::m_handler)(void) = 0;

void Cplatform::RegisterHandler(void (*handler)(void))
	{
	signal(SIGINT, SigHandler);
	m_handler = handler;
	}

void Cplatform::SigHandler(int sig)
	{
	signal(SIGINT, SigHandler);
	if (m_handler)
		m_handler();
	}

int Cplatform::FindProcess(char *name)
	{
	external_pinfo *pinfo;
	cygwin_getinfo_types query = CW_GETPINFO;
	int pid, mypid;
	
	mypid = getpid();
	cygwin_internal(CW_LOCK_PINFO, 1000);
	for (pid=0; (pinfo=(external_pinfo *)cygwin_internal(CW_GETPINFO, pid | CW_NEXTPID)); pid=pinfo->pid)
		{
		if (pinfo->pid!=mypid && strstr(pinfo->progname, name))
			return pinfo->pid;
		}

	cygwin_internal(CW_UNLOCK_PINFO);
	return 0;
	}

int Cplatform::KillProcess(int pid)
	{
	kill(pid, SIGKILL);
	sleep(2);
	}

void *Cplatform::OpenDir(const char *path)
	{
	return (void *)opendir(path);
	}

char *Cplatform::GetFile(void *dir)
	{
	struct dirent *entry;

	entry = readdir((DIR *)dir);
	if (entry)
		return entry->d_name;
	else
		return NULL;
	}

void Cplatform::CloseDir(void *dir)
	{
	closedir((DIR *)dir);
	}

