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


#ifndef _OBJECTS_H
#define _OBJECTS_H

#include <stdlib.h>
#include <sys/time.h>

class Cplatform
	{
public:
	static inline void YieldTask()
		{
		struct timeval tval;
		tval.tv_sec = 0;
		tval.tv_usec = 1000;
		select(0, NULL, NULL, NULL, &tval);
		}
	static inline unsigned long GetClock()
		{
		struct timeval tv;
		struct timezone tz; 
		gettimeofday(&tv, &tz);
		return tv.tv_sec*1000 + tv.tv_usec/1000;  
		}

	static void RegisterHandler(void (*handler)(void));

	static int FindProcess(char *name);
	static int KillProcess(int pid);

	static void *OpenDir(const char *path);
	static char *GetFile(void *dir);
	static void CloseDir(void *dir);

private:
	static void SigHandler(int sig);

	static void (*m_handler)(void);
	};

#endif

