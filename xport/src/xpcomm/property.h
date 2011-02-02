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

#ifndef _PROPERTY_H
#define _PROPERTY_H

#define RES_OK						0
#define RES_END						1
#define RES_FAIL					-1
#define RES_TIMEOUT					-2
#define RES_DENY					-3
#define RES_PROPERTY_NOT_SUPPORTED	-4
#define RES_MODEL_NOT_SUPPORTED		-5
#define RES_NOT_CONNECTED			-6
#define RES_NO_POWER				-7

#define EXCEPT_CTRL_C				0 // ctrl-c exception

enum EXPproperty
	{
	PROP_LEVEL_DEBUG = 0,
	PROP_METHOD_RESET,
	PROP_PPORT_DELAY_READ,
	PROP_PPORT_DELAY_WRITE,
	PROP_PPORT_ADDRESS,
	PROP_PPORT_NUM,
	PROP_MODEL,
	PROP_READ_CONSISTENCY,
	PROP_TEST,
	// ... 
	};

enum EXPresetMethod
	{
	RESET_POWER_TOGGLE = 0,
	RESET_AUTO
	};

class IXPproperty
	{
	public:
	virtual int SetProperty(EXPproperty property, unsigned long val) = 0;
	virtual int GetProperty(EXPproperty property, unsigned long *pval) = 0;
	};

#endif //_PROPERTY_H
