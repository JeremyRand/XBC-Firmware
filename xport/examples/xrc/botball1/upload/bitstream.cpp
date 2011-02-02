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
#include <assert.h>
#include "bitstream.h"

unsigned char CBitstream::ReverseByte(unsigned char byte)
	{
	int i;
	unsigned char bit, res = 0;
	
	for (i=0; i<8; i++)
		{
		bit = (1<<i)&byte ? 1 : 0;
		bit <<= (7-i);
		res |= bit;
		}
	return res;
	}

int CBitstream::CopyField(std::istream &is, char *pfield, unsigned long len)
	{
	unsigned long i, fieldLength;
	char byte;

	fieldLength = GetInteger(is);
	if (fieldLength>len)
		return -1;
	for(i=0; i<fieldLength; i++)
		{ 
		is.read(&byte, 1);
		if (is.eof()!=0)
		  return -1;
		pfield[i] = byte;
		}
	return 0;
	}

int CBitstream::ParseBitstream(std::istream &is, unsigned char *bitstream, unsigned long *plen, SBitstreamInfo *pinfo)
	{
	unsigned long i, fieldLength;
	int status;
	unsigned char byte;
	
	pinfo->mnumber = BS_MNUMBER;
	// skip first field
	fieldLength = GetInteger(is);
	is.ignore(fieldLength);

	status = ScanForByte(is, 'a');
	status |= CopyField(is, pinfo->Name, sizeof(pinfo->Name));
	if (status!=0)
		return status;

	status = ScanForByte(is, 'b');
	status |= CopyField(is, pinfo->Device, sizeof(pinfo->Device));
	if (status!=0)
		return status;

	status = ScanForByte(is, 'c');
	status |= CopyField(is, pinfo->Date, sizeof(pinfo->Date));
	if (status!=0)
		return status;

	status = ScanForByte(is, 'd');
	status |= CopyField(is, pinfo->Time, sizeof(pinfo->Time));
	if (status!=0)
		return status;

	if (bitstream && plen)
		{
		status = ScanForByte(is, 'e');
		if (status!=0)
			return status;
		
		fieldLength = GetInteger(is, 4); 
		pinfo->length = fieldLength;
		if(fieldLength>*plen)
			return -1;
		for(i=0, pinfo->checksum=0; i<fieldLength; i++)
			{ // read the bytes and send the config. bits to the board
			is.read((char *)&byte,1);
			if (is.eof()!=0)
				return -1;
			byte = ReverseByte(byte);
			pinfo->checksum += byte+1;
			bitstream[i] = byte;
			}
		
		*plen = fieldLength;
		}
	return 0;
	}

unsigned long CBitstream::GetInteger(std::istream &is, int len)
	{
	unsigned char digit;
	unsigned long lui = 0;

	assert(len<=4); // can't handle lengths greater than 32 bits
	for(int i=0; i<len; i++)
		{ // process hex digits starting from MSDigit
		is.read((char *)&digit,1);
		assert(is.eof()==0);
		lui = lui*256 + (long unsigned int)digit;
		}
	return lui;
	}

int CBitstream::ScanForByte(std::istream &is, unsigned char searchType)
	{
	char type;

	while(true)
		{
		is.read(&type,1);
		if(type==searchType)
			return 0;
		if(is.eof()!=0)
			return -1;
		}
	}
