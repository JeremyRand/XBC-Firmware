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

#ifndef _XPCOMM_H
#define _XPCOMM_H

#include "property.h"

#define XPC_VERSION			"1.4.1"
#define XPC_ENV_VARIABLE	"XPCOMM_ARGS"

#define XPC_COMMAND_MAX		100
#define XPC_PROPERTY_MAX	100

enum EXPcommand
	{
	COMM_CPU_RESET,
	COMM_PAUSE,
	COMM_FLASH_PROGRAM,
	COMM_FLASH_VERIFY,
	COMM_FLASH_READ,
	COMM_CONFIG,
	COMM_CONFIG_PROGRAM,
	COMM_CONFIG_VERIFY,
	COMM_CONFIG_CHOOSE_PROGRAM,
	COMM_CONFIG_CHOOSE_VERIFY,
	COMM_UPDATE,
	COMM_INFO
	// ...
	};

struct SXPcommand
	{
	EXPcommand		command;
	char			*sarg;
	unsigned long	iarg1;
	unsigned long	iarg2;
	};

struct SXPproperty
	{
	EXPproperty		property;
	unsigned long	val;
	};

class CXPconfiguration;
class CXPmodel;

class CXPexec
	{
	public:
	CXPexec();
	~CXPexec();

	int AddCommand(EXPcommand command, char *sarg, unsigned long iarg1, unsigned long iarg2);
	int AddProperty(EXPproperty property, unsigned long val);
	int ExecuteCommands(unsigned long iter, bool reset, bool serial);
	int ExecuteCommand(SXPcommand *pcommand);
	int SetProperties(IXPproperty *pclass);
	void PrintInfo();
	unsigned long CommandsNum()
		{
		return commandsNum;
		}

	private:

	SXPcommand commands[XPC_COMMAND_MAX];
	SXPproperty properties[XPC_PROPERTY_MAX];
	unsigned long commandsNum;
	unsigned long propertiesNum;
	CXPconfiguration *m_pconfig;
	EXPresetMethod m_methodReset;
	bool m_finalExec;
	bool m_initReset;
	CXPdebug m_debug;
	};

#endif //_XPCOMM_H
