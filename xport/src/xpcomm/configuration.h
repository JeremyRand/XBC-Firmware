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

#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include "flash.h"
#include "bitstream.h"

#define XPC_CONFIG_SLOTS		8
#define XPC_SIZE_STUFF			0x200

class CXPconfiguration : public CXPflash
	{
public:
	CXPconfiguration();
	int Write(unsigned char slot, std::istream &is);
	int Write(unsigned char slot, char *filename);
	int Write(unsigned char slot, unsigned char *bitstream, unsigned long len);
	int Verify(unsigned char slot, std::istream &is);
	int Verify(unsigned char slot, char *filename);
	int Verify(unsigned char slot, unsigned char *bitstream, unsigned long len);
	int Verify(unsigned char slot);
	int GetInfo(unsigned char slot, SBitstreamInfo *pinfo);
	int Update(bool force=false);
	int Compatible(std::istream &is, bool *comp);
	int Compatible(char *filename, bool *comp);
	int ChooseBitstream(char *directory, char *bitstream);
	int PrintInfo();
	int PrintInfo(unsigned char slot);
	virtual int Program(unsigned long addr, unsigned short *data, unsigned long len, bool erase=false);
	virtual int Erase(unsigned long addr, unsigned long len);

private:
	bool m_override;
	};

#endif //_CONFIGURATION_H

