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

#include <stdio.h>
#include <string.h>
#include "model.h"
#include "progbit150.inl"
#include "progbit50.inl"

CXPmodel::CXPmodel(unsigned char model)
	{
	SetModel(model);
	}

int CXPmodel::SetModel(unsigned char model)
	{
	m_model = model;
	return SetInfo();
	}

int CXPmodel::SetInfo()
	{
	int res = 0;
	m_supported = true;
	switch(m_model)
		{
		case 0x11:
			sprintf(m_name, "502A-4");
			sprintf(m_description, "Xilinx Spartan II 50K, 4Mbytes flash, no RAM");
			sprintf(m_version, "2.0A/B");
			m_device = XPM_XC2S50PQ208;
			m_sizeFlash = 0x200000;
			m_sizeLogic = 0x10000;
			m_sizeLogicStuff = 0x200;
			m_progbit = progbit50;
			m_progbitSize = sizeof(progbit50);
			break;

		case 0x22:
			sprintf(m_name, "502A-4-16");
			sprintf(m_description, "Xilinx Spartan II 50K, 4Mbytes flash, 16Mbytes RAM");
			sprintf(m_version, "2.0A/B");
			m_device = XPM_XC2S50PQ208;
			m_sizeFlash = 0x200000;
			m_sizeLogic = 0x10000;
			m_sizeLogicStuff = 0x200;
			m_progbit = progbit50;
			m_progbitSize = sizeof(progbit50);
			break;

		case 0x33:
			sprintf(m_name, "1502A-4-16");
			sprintf(m_description, "Xilinx Spartan II 150K, 4Mbytes flash, 16Mbytes RAM");
			sprintf(m_version, "2.0A/B");
			m_device = XPM_XC2S150PQ208;
			m_sizeFlash = 0x200000;
			m_sizeLogic = 0x10000;
			m_sizeLogicStuff = 0x1c0;
			m_progbit = progbit150;
			m_progbitSize = sizeof(progbit150);
			break;

		default:
			sprintf(m_name, "xxx-%x", m_model);
			sprintf(m_description, "Unknown");
			sprintf(m_version, "Unknown");
			m_device = XPM_UNKNOWN;
			m_sizeFlash = 0;
			m_sizeLogic = 0;
			m_supported = false;
			m_progbit = NULL;
			m_progbitSize = 0;
			res = -1;
		}

	return res;
	}

const unsigned char *CXPmodel::Progbit()
	{
	return m_progbit;
	}

unsigned long CXPmodel::ProgbitSize()
	{
	return m_progbitSize;
	}

bool CXPmodel::Supported()
	{
	return m_supported;
	}

char *CXPmodel::Name()
	{
	return m_name;
	}

char *CXPmodel::Description()
	{
	return m_description;
	}

char *CXPmodel::Version()
	{
	return m_version;
	}

unsigned long CXPmodel::SizeFlash()
	{
	return m_sizeFlash;
	}

unsigned long CXPmodel::SizeLogic()
	{
	return m_sizeLogic;
	}

unsigned long CXPmodel::SizeLogicStuff()
	{
	return m_sizeLogicStuff;
	}

EXPMdevice CXPmodel::Device()
	{
	return m_device;
	}

bool CXPmodel::CheckDevice(char *device)
	{
	if ((m_device==XPM_XC2S50PQ208 && !strcmp(device, "2s50pq208")) ||
		(m_device==XPM_XC2S150PQ208 && !strcmp(device, "2s150pq208")))
		return true;
	else 
		return false;
	}
