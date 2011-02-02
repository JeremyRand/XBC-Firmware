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



#include <sys/unistd.h>
#include <sys/signal.h>
#include <dirent.h>
#include "objects.h"
#include <signal.h>
#include <stdio.h>
#include <ctype.h>
#include <iostream>
#include <fstream>


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

// There should be a simpler way to do this	
int Cplatform::FindProcess(char *name)
	{
	int pid, mypid;
	DIR *proc_dir;
	struct dirent *dentry;
	
	mypid = getpid();
	
	proc_dir = opendir("/proc");
	if (proc_dir != NULL)
		{	
		dentry = readdir(proc_dir);		
		while (dentry != NULL)
			{			
			if (isdigit(dentry->d_name[0]))
				{
				
				// Open the file
				char filename[256] = {0};
				
				pid = atoi (dentry->d_name);
				
				strcat (filename,"/proc/");
				strcat (filename, dentry->d_name);
				strcat (filename, "/cmdline");
								
				FILE *fp = fopen (filename,"rb");
				if (fp)
					{
					char proc_name[256] = {0};
					fread (proc_name, 256, 1, fp);
					fclose (fp);
					
					if (!strcmp (basename(proc_name),name))
						{
							if (pid != mypid)
								return pid;
						}				
					}							
				}
			dentry = readdir(proc_dir);
			}
		closedir((DIR *)proc_dir);
		}
	
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

