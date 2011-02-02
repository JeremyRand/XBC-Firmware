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
#include <stdarg.h>
#include <string.h>
#include "rpc.h"
#include "textdisp.h"


CRPCtarget::CRPCtarget(unsigned char mode, unsigned long timeout)
	{
	m_timeout = timeout;
	m_checksumErrors = 0;
	m_mode = mode;
	if (m_mode==RPC_LCD_AND_CPORT)
		TDInit(TDM_LCD);
	}

short CRPCtarget::Print(char *string)
	{
	unsigned long len;
	unsigned short checksum;

	if (m_mode==RPC_LCD_AND_CPORT)
		TDPrint(string);

	len = strlen(string) + 2; 
	
	SendHeader(RPC_PRINT, len);

	checksum = RPC_PRINT;

	// send buffer
	while(*string)
		{
		m_cport.SendChar(*string);
		checksum += (unsigned char)*string;
		string++;
		}

	m_cport.SendChar(0);
	
	SendShort(checksum+len);

	return 0;
	}

short CRPCtarget::Printf(char *format, ...)
	{
	char buf[128];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	Print(buf);
	return 0;
	}

unsigned char CRPCtarget::Fopen(char *filename, char *format)
	{
	unsigned long len;
	unsigned short checksum;
	unsigned char result;

	len = strlen(filename) + 2; 
	
	SendHeader(RPC_FOPEN, len);

	checksum = RPC_FOPEN;

	// send buffer
	while(*filename)
		{
		m_cport.SendChar(*filename);
		checksum += (unsigned char)*filename;
		filename++;
		}

	m_cport.SendChar(0);
	
	SendShort(checksum+len);

	// get response
	if (FixedResponse(&result, 1)<0)
		return 0;

	return result;
	}

short CRPCtarget::Fclose(unsigned char fp)
	{
	SendHeader(RPC_FCLOSE, 2);
	m_cport.SendChar(fp);
	SendShort(RPC_FCLOSE+fp+2);

	return 0;
	}

unsigned long CRPCtarget::Fwrite(void *a_ptr, unsigned long len, unsigned char fp)
	{
	unsigned short checksum;
	unsigned long i;
	unsigned char c;
	unsigned long result;

	SendHeader(RPC_FWRITE, len+2);
	
	checksum = RPC_FWRITE;

	m_cport.SendChar(fp);
	checksum += fp;

	for (i=0; i<len; i++)
		{
		c = ((unsigned char *)a_ptr)[i];
		m_cport.SendChar(c);
		checksum += c;
		}

	SendShort(checksum + len+2);

	if (FixedResponse((unsigned char *)&result, 4)<0)
		return 0;

	return result;
	}


void CRPCtarget::SendLong(unsigned long val)
	{
	char *cVal = (char *)&val;

	m_cport.SendChar(cVal[0]);
	m_cport.SendChar(cVal[1]);
	m_cport.SendChar(cVal[2]);
	m_cport.SendChar(cVal[3]);
	}

void CRPCtarget::SendShort(unsigned short val)
	{
	char *cVal = (char *)&val;

	m_cport.SendChar(cVal[0]);
	m_cport.SendChar(cVal[1]);
	}

void CRPCtarget::SendLength(unsigned long len)
	{
	unsigned char checksum;
	unsigned char c;
	char *cLen = (char *)&len;

	checksum = (unsigned char)cLen[0];
	m_cport.SendChar((char)checksum);

	c = (unsigned char)cLen[1];
	checksum += c;
	m_cport.SendChar((char)c);

	c = (unsigned char)cLen[2];
	checksum += c;
	m_cport.SendChar((char)c);

	m_cport.SendChar((char)checksum);
	}

void CRPCtarget::SendHeader(unsigned char index, unsigned long len)
	{
	// send start code
	SendLong(RPC_STARTCODE);

	// send length
	SendLength(len);

	// send RPC index
	m_cport.SendChar(index);
	}

short CRPCtarget::FixedResponse(unsigned char *data, unsigned long len)
	{
	unsigned long i;
	unsigned long timeout;
	unsigned char c;
	unsigned short runningChecksum;
	unsigned short checksum;
	unsigned char *cChecksum = (unsigned char *)&checksum;

	if (!m_cport.Host())
		return RPC_ERROR_TIMEOUT;

	for (i=0, runningChecksum=0, timeout=0; i<len; i++)
		{
		while(m_cport.RecvEmpty())
			{
			if (timeout++>m_timeout)
				return RPC_ERROR_TIMEOUT;
			}
		c = (unsigned char)XP_REG_CPORT_DATA;
		runningChecksum += c;
		data[i] = c;
		}
	runningChecksum += len;

	// get checksum
	while(m_cport.RecvEmpty())
		{
		if (timeout++>m_timeout)
			return RPC_ERROR_TIMEOUT;
		}
	cChecksum[0] = (unsigned char)XP_REG_CPORT_DATA;

	while(m_cport.RecvEmpty())
		{
		if (timeout++>m_timeout)
			return RPC_ERROR_TIMEOUT;
		}
	cChecksum[1] = (unsigned char)XP_REG_CPORT_DATA;

	// compare
	if (runningChecksum!=checksum)
		{
		m_checksumErrors++;
		return RPC_ERROR_CHECKSUM;
		}

	return RPC_OK;
	}
