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

#ifndef _BITSTREAM_H
#define _BITSTREAM_H
#include <istream>

#define BS_MNUMBER		0xcafe

struct SBitstreamInfo
	{
	char Name[32];
	char Device[16];
	char Date[16];
	char Time[16];
	unsigned long length;
	unsigned short checksum;
	unsigned short mnumber;
	};

class CBitstream
	{
public:
	static int ParseBitstream(std::istream &is, unsigned char *bitstream, unsigned long *plen, SBitstreamInfo *pinfo);

private:
	static unsigned long GetInteger(std::istream &is, int len=2);
	static int ScanForByte(std::istream &is, unsigned char searchType);
	static int CopyField(std::istream &is, char *pfield, unsigned long len);
	static unsigned char ReverseByte(unsigned char byte);
	};


#endif //_BITSTREAM_H
