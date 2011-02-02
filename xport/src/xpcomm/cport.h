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

#ifndef _CPORT_H
#define _CPORT_H

#include "cqueue.h"
#include "hal.h"

#define XPCP_ERR				(-1)
#define XPCP_YIELD_IDLE			15
#define XPCP_FILL_IDLE			5
#define XPCP_CQUEUE_SIZE		0x10000
#define XPCP_RPC_BUF_SIZE		0x80000

#define XPCP_RPC_STARTCODE		0x1e3c5a78

#define XPCP_RPC_PRINT			0x01
#define XPCP_RPC_FOPEN			0x02
#define XPCP_RPC_FCLOSE			0x03
#define XPCP_RPC_FWRITE			0x04

class CXPcport : public IXPproperty
	{
public:
	CXPcport(CXPhal *phal, unsigned long timeout=1000000000);
	~CXPcport();

	int Reset();
	int SetTimeout(unsigned long timeout);
	int Write(unsigned char *data, unsigned long len, bool yield=true);
	int WriteFill(long idle);
	int WriteChar(unsigned char c);
	int Read(unsigned char *data, unsigned long len, bool yield=true);
	int ReadFill(unsigned char *data, unsigned long len, long idle, unsigned short *checksum=0);
	int ReadChar(unsigned char *c);
	bool DataAvail();
	int TermLoop(bool cb);
	int RPCLoop();
	void SetHostControl(bool val);

	static HINSTANCE m_dllHandle;

	// IXPproperty methods
	virtual int SetProperty(EXPproperty property, unsigned long val);
	virtual int GetProperty(EXPproperty property, unsigned long *pval);

private:

	int RPCDispatch(unsigned char *buf, unsigned long rpcLen);
	int RPCPrint(unsigned char *stack);
	int RPCFopen(unsigned char *stack);
	int RPCFclose(unsigned char *stack);
	int RPCFwrite(unsigned char *stack, unsigned long len);
	int RPCclear();

	int RPCFixedResponse(unsigned char *data, unsigned long len);

	FILE *m_fileList[0x100];
	unsigned long m_rpcBufSize;
	unsigned char *m_rpcBuf;

	CXPdebug m_debug;
	CcQueue m_hostInQueue;
	CXPhal *m_phal;
	unsigned long m_timeout;
	};

#endif
