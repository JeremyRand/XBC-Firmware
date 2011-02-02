#include <string.h>
#ifdef GBA
#include <textdisp.h>
#endif
#include "rpc.h"

CRpc::CRpc(IComm *pComm, unsigned char retries) : m_header(DPRC_HEADER_SIZE)
	{
	m_retries = retries;
	SetComm(pComm);
	// zero out function table
	memset(m_functionTable, 0, sizeof(m_functionTable));
	m_timeout = DRPC_DEFAULT_TIMEOUT;
	}

CRpc::~CRpc()
	{
	}

void CRpc::SetComm(IComm *pComm)
	{
	m_pComm = pComm;
	}

int CRpc::Call(unsigned char rpcIndex, CRpcArgs *args)
	{
	int res;
	unsigned short len;
	unsigned char retries = 0;
	unsigned short pushIndexSave;
	unsigned char rpcIndexReceive;

	pushIndexSave = args->m_pushIndex;

	while(1)
		{
		// send header
		SendHeader(rpcIndex, args->GetLen());

		// send data
		Send(PT_CALL_DATA, args);
		
		// get response header
		m_header.Reset();
		res=Receive(PT_RESPONSE_HEADER, &m_header, DPRC_HEADER_SIZE, m_timeout, m_timeout);
		if (res>=0)
			{
			m_header.Pop((char *)&rpcIndexReceive);
			m_header.Pop((short *)&len);
			if (rpcIndexReceive==DRPC_RESPONSE_ERROR_CHECKSUM) 
				continue;
			if (rpcIndexReceive!=DRPC_RESPONSE_SUCCESS)
				{
				res = DRPC_ERROR_GENERAL;
				break;
				}
			// get response data
			res = Receive(PT_RESPONSE_DATA, args, len, m_timeout, m_timeout);
			}

		if (res==DRPC_ERROR_CHECKSUM || res==DRPC_ERROR_TIMEOUT) // only retransmit if there is a checksum error
			{
			retries++;
			if (retries>=m_retries)
				break;
			else
				{
				// restore push index
				args->m_pushIndex = pushIndexSave;
				continue;
				}
			}
		else
			break;
		}
	return res;
	}

int CRpc::Register(unsigned char rpcIndex, void (*rpcFunction)(CRpcArgs *))
	{
	if (rpcIndex>=DRPC_TABLE_SIZE)
		return DRPC_ERROR_GENERAL;

	m_functionTable[rpcIndex] = rpcFunction;
	return 0;
	}

int CRpc::Dispatch(CRpcArgs *args, unsigned short startTimeoutMs)
	{
	int res;
	int dispatches = 0;
	unsigned char rpcIndex; 
	unsigned short len;
	
	// dispatch as long as there is data available
	while(1)
		{
		// get header
		m_header.Reset();
		res = Receive(PT_CALL_HEADER, &m_header, DPRC_HEADER_SIZE, startTimeoutMs, m_timeout);
		if (res>=0)
			{
			m_header.Pop((char *)&rpcIndex);
			m_header.Pop((short *)&len);
			if (rpcIndex>=DRPC_TABLE_SIZE)
				continue; // ignore

			// get data
			args->Reset();
			res = Receive(PT_CALL_DATA, args, len, m_timeout, m_timeout);

			// make RPC call
			if (res>=0)
				{
				if (m_functionTable[rpcIndex])
					{
					(m_functionTable[rpcIndex])(args);
					// result is in args, send it back as the response
					// first, send response header
					SendHeader(DRPC_RESPONSE_SUCCESS, args->GetLen());
					// next, send data
					Send(PT_RESPONSE_DATA, args);
					dispatches++;
					}
				else
					SendHeader(DRPC_RESPONSE_ERROR_NO_FUNCTION, 0);
				}
			else if (res==DRPC_ERROR_CHECKSUM)
				SendHeader(DRPC_RESPONSE_ERROR_CHECKSUM, 0);
			}
		
		if (res==DRPC_ERROR_CHECKSUM)
			SendHeader(DRPC_RESPONSE_ERROR_CHECKSUM, 0);
		else if (res==DRPC_ERROR_TIMEOUT)
			break;
		}

	return dispatches;
	}

void CRpc::SetTimeout(unsigned short timeout)
	{
	m_timeout = timeout;
	}

unsigned short CRpc::GetTimeout()
	{
	return m_timeout;
	}

int CRpc::Send(EPacketType type, CRpcArgs *data)
	{
	unsigned short i;
	unsigned short len;
	unsigned short checksum;
	char *buf, c;
	unsigned long startCode = DRPC_STARTCODE;

	// send start code
	if (type==PT_RESPONSE_HEADER || type==PT_CALL_HEADER)
		{
		for (i=0; i<sizeof(startCode); i++)
			{
			c = (char)(startCode&0xff);
			m_pComm->Send(&c, 1, DCOMM_TIMEOUT_FOREVER);
			startCode >>= 8;
			}
		}

	// send data
	len = data->GetLen();
	buf = data->GetData();
	m_pComm->Send(buf, len, DCOMM_TIMEOUT_FOREVER);

	// calculate and send checksum
	for (i=0, checksum=0; i<len; i++)
		checksum += (unsigned char)buf[i];
	for (i=0; i<sizeof(checksum); i++)
		{
		c = (char)(checksum&0xff);
		m_pComm->Send(&c, 1, DCOMM_TIMEOUT_FOREVER);
		checksum >>= 8;
		}
	
	return 0;
	}

int CRpc::Receive(EPacketType type, CRpcArgs *data, unsigned short len, 
				  unsigned short startTimeoutMs, unsigned short timeoutMs)
	{
	unsigned short i;
	unsigned short checksum;
	unsigned short checksumCalc;
	unsigned long startCode = 0;

	int res;
	char *buf;
	char c;

	// reset popindex -- this is the data we want the caller to retrieve by calling Pop()
	data->m_popIndex = data->m_pushIndex;
	if (data->CheckPush(len)<0)
		return DRPC_ERROR_BUFSIZE;

	// get start code
	if (type==PT_RESPONSE_HEADER || type==PT_CALL_HEADER)
		{
		res = m_pComm->Receive(&c, 1, startTimeoutMs);
		if (res!=1)
			return DRPC_ERROR_TIMEOUT;
		while(1)
			{
			startCode >>= 8;
			startCode |= (unsigned long)c<<24;
			if (startCode==DRPC_STARTCODE)
				break;
			res = m_pComm->Receive(&c, 1, timeoutMs);
			if (res!=1)
				return DRPC_ERROR_TIMEOUT;
			}
		}

	// put data after existing data
	buf = data->GetData() + data->m_pushIndex;
	
	// receive data
	res = m_pComm->Receive(buf, len, timeoutMs);	
	if (res!=len)
		return DRPC_ERROR_TIMEOUT;
	// advance push (write) index
	data->m_pushIndex += len;

	// receive checksum
	for (i=0; i<sizeof(checksum); i++)
		{
		res = m_pComm->Receive(&c, 1, timeoutMs);
		if (res!=1)
			return DRPC_ERROR_TIMEOUT;
		checksum >>= 8;
		checksum |= (unsigned short)c<<8;
		}

	// calculate checksum
	for (i=0, checksumCalc=0; i<len; i++)
		checksumCalc += (unsigned char)buf[i];

	// compare checksum
	if (checksumCalc!=checksum)
		return DRPC_ERROR_CHECKSUM;

	return 0;
	}

 
int CRpc::SendHeader(unsigned char rpcIndex, unsigned short len)
	{
	m_header.Reset();
	m_header.Push((char)rpcIndex);
	m_header.Push((short)len);
	return Send(PT_RESPONSE_HEADER, &m_header);
	}
