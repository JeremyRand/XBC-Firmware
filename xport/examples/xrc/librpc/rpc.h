#ifndef _RPC_H
#define _RPC_H

#include "comm.h"
#include "rpcargs.h"

#define DRPC_TABLE_SIZE							0x80
#define DPRC_HEADER_SIZE						3 // byte + short
#define DRPC_ERROR_GENERAL						-1
#define DRPC_ERROR_BUFSIZE						-2
#define DRPC_ERROR_CHECKSUM						-3
#define DRPC_ERROR_TIMEOUT						-4

#define DRPC_STARTCODE							0x1e3c5a78
#define DRPC_RESPONSE_SUCCESS					0xff
#define DRPC_RESPONSE_ERROR_CHECKSUM			0xfe
#define DRPC_RESPONSE_ERROR_NO_FUNCTION			0xfd
#define DRPC_DEFAULT_TIMEOUT					500
#define DRPC_DEFAULT_RETRIES					4

enum EPacketType
	{
	PT_CALL_HEADER,
	PT_CALL_DATA,
	PT_RESPONSE_HEADER,
	PT_RESPONSE_DATA
	};

class CRpc
	{
public:
	CRpc(IComm *pComm, unsigned char retries = DRPC_DEFAULT_RETRIES);
	~CRpc();

	void SetComm(IComm *pComm);

	int Call(unsigned char rpcIndex, CRpcArgs *args);
	int Register(unsigned char rpcIndex, void (*rpcFunction)(CRpcArgs *));
	int Dispatch(CRpcArgs *args, unsigned short startTimeoutMs=0);
	
	void SetTimeout(unsigned short timeout);
	unsigned short GetTimeout();

protected:
	int Send(EPacketType type, CRpcArgs *data);
	int Receive(EPacketType type, CRpcArgs *data, unsigned short len, 
		unsigned short startTimeoutMs=0, unsigned short timeoutMs=DCOMM_TIMEOUT_FOREVER);
	int SendHeader(unsigned char rpcIndex, unsigned short len);

	IComm *m_pComm;
	void (*m_functionTable[DRPC_TABLE_SIZE])(CRpcArgs *);
	unsigned short m_timeout;
	CRpcArgs m_header;
	unsigned char m_retries;
	};

#endif
