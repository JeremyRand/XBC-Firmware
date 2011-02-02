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

#ifndef _MODEL_H
#define _MODEL_H

enum EXPMdevice
    {
	XPM_UNKNOWN,
	XPM_XC2S50PQ208,
	XPM_XC2S150PQ208
	};

class CXPmodel
	{
	public:
	CXPmodel(unsigned char model);

	int SetModel(unsigned char model);
	int SetInfo();
	bool Supported();
	char *Name();
	char *Description();
	char *Version();
	unsigned long SizeFlash();
	unsigned long SizeLogic();
	unsigned long SizeLogicStuff();
	EXPMdevice Device();
	const unsigned char *Progbit();
	unsigned long ProgbitSize();
	bool CheckDevice(char *device);

	private:
	unsigned char		m_model;
	unsigned long		m_sizeFlash;
	unsigned long		m_sizeLogic;
	unsigned long		m_sizeLogicStuff;
	EXPMdevice			m_device;

	const unsigned char *m_progbit;
	unsigned long		m_progbitSize;

	bool m_supported;
	char m_name[16];
	char m_description[128];
	char m_version[16];
	};

#endif
