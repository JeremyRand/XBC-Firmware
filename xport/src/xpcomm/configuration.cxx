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

#include <fstream>
#include <string.h>
#include "configuration.h"

CXPconfiguration::CXPconfiguration() 
	{
	m_override = false;
	}

int CXPconfiguration::Write(unsigned char slot, std::istream &is)
	{
	unsigned char bs[XPH_SIZE_BUFFER];
	unsigned short stuff[XPC_SIZE_STUFF];
	unsigned long len = XPH_SIZE_BUFFER;
	SBitstreamInfo info;
	int res, i;

	for (i=0; i<XPC_SIZE_STUFF; i++)
		stuff[i] = 0xffff;

	if ((res=CBitstream::ParseBitstream(is, bs, &len, &info))!=RES_OK)
		{
		m_debug.printf(0, "Parse error.\n");
		return res;
		}

	// Make sure bitstream is compatible with our device
	if (!CheckDevice(info.Device))
		{
		m_debug.printf(0, "\nThis bitstream is intended for an XC%s device.\n", info.Device);
		m_debug.printf(0, "Failed.\n");
		return RES_FAIL;
		}

	m_override = true; 

	if ((res=Program(SizeFlash() - (slot + 1)*SizeLogic(), stuff, SizeLogicStuff(), true))!=RES_OK)
		{
		m_debug.printf(0, "Program error.\n");
		m_override = false;
		return res;
		}

	if ((res=Program(SizeFlash() - (slot + 1)*SizeLogic() + SizeLogicStuff(), (unsigned short *)bs, len>>1, true))!=RES_OK)
		{
		m_debug.printf(0, "Program error.\n");
		m_override = false;
		return res;
		}

	if ((res=Program(SizeFlash() - slot*SizeLogic() - (sizeof(SBitstreamInfo)>>1), (unsigned short *)&info, sizeof(SBitstreamInfo)>>1, true))!=RES_OK)
		{
		m_debug.printf(0, "Program error.\n");
		m_override = false;
		return res;
		}


	m_override = false;
	return RES_OK;
	}

int CXPconfiguration::Write(unsigned char slot, char *filename)
	{
	int res;
	std::ifstream is(filename, std::ios::binary);//|ios::nocreate); 

	if(is.fail() || is.eof())
		{
		m_debug.printf(0, "Cannot open file %s.\n", filename);
		return RES_FAIL;
		}
	
	m_debug.printf(1, "Writing configuration (%s) to slot %d...", filename, slot);
	res = Write(slot, is);
	if (res==RES_OK) 
		m_debug.printf(1, "\nSuccess.\n", slot);
	return res;
	}

int CXPconfiguration::Write(unsigned char slot, unsigned char *bitstream, unsigned long len)
	{

	std::string str ((char *)bitstream, len);
	std::istringstream  is(str);
	return Write(slot, is);
	}

int CXPconfiguration::Verify(unsigned char slot, std::istream &is)
	{
	unsigned char bs[XPH_SIZE_BUFFER];;
	unsigned long len = XPH_SIZE_BUFFER;
	SBitstreamInfo info, infoRead;
	int res;

	if ((res=CBitstream::ParseBitstream(is, bs, &len, &info))!=RES_OK)
		return res;

	if ((res=CXPflash::Verify(SizeFlash() - (slot + 1)*SizeLogic() + SizeLogicStuff(), (unsigned short *)bs, len>>1))!=RES_OK)
		return res;

	if ((res=GetInfo(slot, &infoRead))!=RES_OK)
		return res;
	if (info.checksum!=infoRead.checksum ||
		info.length!=infoRead.length ||
		info.mnumber!=infoRead.mnumber ||
		strcmp(info.Name, infoRead.Name) ||
		strcmp(info.Device, infoRead.Device))
		return RES_FAIL;

	return RES_OK;
	}

int CXPconfiguration::Verify(unsigned char slot, char *filename)
	{
	int res;
	std::ifstream is(filename, std::ios::binary);//|ios::nocreate); 

	if(is.fail() || is.eof())
		{
		m_debug.printf(0, "Cannot open file %s.\n", filename);
		return RES_FAIL;
		}

	m_debug.printf(1, "Verifying configuration (%s) against slot %d...", filename, slot);
	res = Verify(slot, is);
	if (res!=RES_OK)
		m_debug.printf(1, "\nFailed.\n", slot);
	else
		m_debug.printf(1, "\nSuccess.\n", slot);
	return res;
	}

int CXPconfiguration::Verify(unsigned char slot, unsigned char *bitstream, unsigned long len)
	{

	std::string str ((char *)bitstream, len);
	std::istringstream  is(str);
	return Verify(slot, is);
	}

int CXPconfiguration::Verify(unsigned char slot)
	{
	unsigned char bs[XPH_SIZE_BUFFER];;
	SBitstreamInfo info;
	unsigned short checksum;
	int i, res;

	if ((res=GetInfo(slot, &info))!=RES_OK)
		return res;

	if ((res=Read(SizeFlash() - (slot + 1)*SizeLogic() + SizeLogicStuff(), (unsigned short *)bs, info.length>>1))!=RES_OK)
		return res;

	for (i=0, checksum=0; i<(int)info.length; i++)
		checksum += bs[i] + 1;

	if (checksum!=info.checksum)
		return RES_FAIL;

	return RES_OK;
	}

int CXPconfiguration::GetInfo(unsigned char slot, SBitstreamInfo *pinfo)
	{
	int res;

	if ((res=Read(SizeFlash() - slot*SizeLogic() - (sizeof(SBitstreamInfo)>>1), (unsigned short *)pinfo, sizeof(SBitstreamInfo)>>1))!=RES_OK)
		return res;

	if (pinfo->mnumber!=BS_MNUMBER)
		return RES_FAIL;

	return RES_OK;
	}

int CXPconfiguration::Compatible(char *filename, bool *comp)
	{
	std::ifstream is(filename, std::ios::binary);//|ios::nocreate); 

	if(is.fail() || is.eof())
		{
		m_debug.printf(0, "Cannot open file %s.\n", filename);
		*comp = false;
		return RES_FAIL;
		}
	return Compatible(is, comp);
	}

int CXPconfiguration::Compatible(std::istream &is, bool *comp)
	{
	unsigned char bs[XPH_SIZE_BUFFER];;
	unsigned long len = XPH_SIZE_BUFFER;
	SBitstreamInfo info;
	int res;

	if ((res=CBitstream::ParseBitstream(is, bs, &len, &info))!=RES_OK)
		return res;

	*comp = CheckDevice(info.Device);

	return RES_OK;
	}

int CXPconfiguration::ChooseBitstream(char *directory, char *bitstream)
	{
	void *dir;
	char *filename;
	int len;
	bool comp;
	int res = RES_FAIL;
	char buf[0x400];

	dir = Cplatform::OpenDir(directory);
	while(filename = Cplatform::GetFile(dir))
		{
		len = strlen(filename);
		if (len>4 && filename[len-1]=='t' && filename[len-2]=='i' && filename[len-3]=='b' && filename[len-4]=='.')
			{
			strcpy(buf, directory);
			strcat(buf, "/");
			strcat(buf, filename);
			Compatible(buf, &comp);
			if (comp)
				{
				strcpy(bitstream, buf);
				res = RES_OK;
				break;
				}
			}
		}

	Cplatform::CloseDir(dir);
	return res;
	}

int CXPconfiguration::PrintInfo()
	{
	int i;

	for (i=0; i<XPC_CONFIG_SLOTS; i++)
		PrintInfo(i);

	return RES_OK;
	}

int CXPconfiguration::Update(bool force)
	{
	int res;
	unsigned char bs[XPH_SIZE_BUFFER];;
	unsigned long len = XPH_SIZE_BUFFER;
	SBitstreamInfo info0, info1;

	std::string str ((char *)Progbit(), ProgbitSize());
	std::istringstream  is(str);
	if ((res=CBitstream::ParseBitstream(is, bs, &len, &info0))!=RES_OK)
		{
		m_debug.printf(0, "Query error\n");
		return res;
		}
	if (!force)
		{
		if ((res=GetInfo(1, &info1))!=RES_OK)
			{
			m_debug.printf(0, "Query error\n");
			return res;
			}
		}
	if (force || info0.checksum!=info1.checksum)
		{
		if (!force)
			m_debug.printf(0, "Configuration is inconsistent.  ");				
		m_debug.printf(0, "Updating flash programming configuration...\n");	
		if ((res=Write(1, (unsigned char *)Progbit(), ProgbitSize()))!=RES_OK)
			{
			m_debug.printf(0, "Failed.\n");
			return res;
			}
		else
			m_debug.printf(0, "Success.\n");
		}
	else
		m_debug.printf(0, "No update required.\n");


	return RES_OK;
	}

int CXPconfiguration::PrintInfo(unsigned char slot)
	{
	SBitstreamInfo info;
	char *space = "            ";
	int i, res;

	if ((res=GetInfo(slot, &info))!=RES_OK)
		return res;

	// only print valid slots
	if (info.mnumber==BS_MNUMBER)
		{
		for (i=0; info.Name[i]!='.' && i<(sizeof(info.Name)-1); i++);
		info.Name[i] = '\0';

		space += strlen(info.Name);
		printf("Slot %d:  %s%s%s %s\n", slot, info.Name, space, info.Date, info.Time);
		return RES_OK;
		}
	return RES_FAIL;
	}

int CXPconfiguration::Erase(unsigned long addr, unsigned long len)
	{
	if (!m_override)
		{
		if (addr+len > SizeFlash() - 2*SizeLogic())
			{
			m_debug.printf(0, "\nWarning: aborting flash erase operation to avoid overwriting logic configuration.\n");  
			return RES_DENY;
			}
		}
	return CXPflash::Erase(addr, len);
	}

int CXPconfiguration::Program(unsigned long addr, unsigned short *data, unsigned long len, bool erase)
	{
	if (!m_override)
		{
		if (addr+len > SizeFlash() - 2*SizeLogic())
			{
			m_debug.printf(0, "\nWarning: aborting flash program operation to avoid overwriting logic configuration.\n");  
			return RES_DENY;
			}
		}
	return CXPflash::Program(addr, data, len, erase);
	}

