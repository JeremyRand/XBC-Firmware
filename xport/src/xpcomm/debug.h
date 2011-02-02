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

#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>
#include <stdio.h>

#ifndef XPD_LEVEL_DEFAULT
#define XPD_LEVEL_DEFAULT	1
#endif

class CXPdebug
	{
	public:
	CXPdebug(unsigned long level=XPD_LEVEL_DEFAULT)
		{
		SetLevel(level);
		}

	void SetLevel(unsigned long level)
		{
		m_level = level;
		}

	int printf(unsigned long level, char *format, ...)
		{

		if (level<=m_level)
			{
			int res;
			va_list args;
			va_start(args, format);
			res = vprintf(format, args);
			fflush(stdout);
			return res;
			}
		return 0;
		}

	// for console redirection
	int printc(char c)
		{
		putchar((int)c);
		fflush(stdout);
		return 0;
		}

	private:
	unsigned long m_level;
	};

#endif //_DEBUG_H
