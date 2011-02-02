#include <string.h>
#include "rpcargs.h"

CRpcArgs::CRpcArgs(unsigned int size)
	{	
	m_buf = new char[size];
	m_size = size;
	Reset();
	}

CRpcArgs::~CRpcArgs()
	{
	delete [] m_buf;
	}

int CRpcArgs::Renew(unsigned short len)
	{
	char *newBuf;
	
	newBuf = new char[len];
	if (!newBuf)
		return -1;
	memcpy(newBuf, m_buf, m_pushIndex);
	delete [] m_buf;
	m_buf = newBuf;
	m_size = len;
	return 0;
	}

void CRpcArgs::Reset()
	{
	m_pushIndex = 0;
	m_popIndex = 0;
	}

int CRpcArgs::Push(char arg)
	{
	int res = 0; 

	if ((res=CheckPush(1))<0)
		return res;

	m_buf[m_pushIndex++] = arg;

	return res;
	}

int CRpcArgs::Push(short arg)
	{
	char *parg = (char *)&arg;
	int res = 0; 

	if ((res=CheckPush(2))<0)
		return res;

#ifdef BIG_ENDIAN
	m_buf[m_pushIndex++] = parg[1];
	m_buf[m_pushIndex++] = parg[0];
#else
	m_buf[m_pushIndex++] = parg[0];
	m_buf[m_pushIndex++] = parg[1];
#endif

	return res;
	}

int CRpcArgs::Push(long arg)
	{
	char *parg = (char *)&arg;
	int res = 0; 

	if ((res=CheckPush(4))<0)
		return res;

#ifdef BIG_ENDIAN
	m_buf[m_pushIndex++] = parg[3];
	m_buf[m_pushIndex++] = parg[2];
	m_buf[m_pushIndex++] = parg[1];
	m_buf[m_pushIndex++] = parg[0];
#else
	m_buf[m_pushIndex++] = parg[0];
	m_buf[m_pushIndex++] = parg[1];
	m_buf[m_pushIndex++] = parg[2];
	m_buf[m_pushIndex++] = parg[3];
#endif

	return res;
	}

int CRpcArgs::Push(char *arg, unsigned short len)
	{
	int res = 0; 

	if ((res=Push((short)len))<0)
		return res;
	if ((res=CheckPush(len))<0)
		return res;

	memcpy(m_buf+m_pushIndex, arg, len);
	m_pushIndex += len;
	
	return res;
	}

int CRpcArgs::Pop(char *arg)
	{
	int res = 0;

	if ((res=CheckPop(1))<0)
		return res;

	*arg = m_buf[m_popIndex++];

	return 0;
	}

int CRpcArgs::Pop(short *arg)
	{
	char *parg = (char *)arg;
	int res = 0;

	if ((res=CheckPop(2))<0)
		return res;

#ifdef BIG_ENDIAN
	parg[1] = m_buf[m_popIndex++];
	parg[0] = m_buf[m_popIndex++];
#else
	parg[0] = m_buf[m_popIndex++];
	parg[1] = m_buf[m_popIndex++];
#endif

	return 0;
	}

int CRpcArgs::Pop(long *arg)
	{
	char *parg = (char *)arg;
	int res = 0;

	if ((res=CheckPop(4))<0)
		return res;

#ifdef BIG_ENDIAN
	parg[3] = m_buf[m_popIndex++];
	parg[2] = m_buf[m_popIndex++];
	parg[1] = m_buf[m_popIndex++];
	parg[0] = m_buf[m_popIndex++];
#else
	parg[0] = m_buf[m_popIndex++];
	parg[1] = m_buf[m_popIndex++];
	parg[2] = m_buf[m_popIndex++];
	parg[3] = m_buf[m_popIndex++];
#endif

	return 0;
	}

int CRpcArgs::Pop(char *arg, unsigned short *len)
	{
	char *parg = (char *)arg;
	int res = 0;
	unsigned short readLen;

	if ((res=Pop((short *)&readLen))>0)
		return res;
	if ((res=CheckPop(readLen))<0)
		return res;
	if (readLen>*len)
		return -1;

	memcpy(arg, m_buf + m_popIndex, readLen);
	m_popIndex += readLen;
	*len = readLen;

	return res;
	}


unsigned short CRpcArgs::GetLen()
	{
	return m_pushIndex; 
	}

unsigned short CRpcArgs::GetSize()
	{
	return m_size; 
	}

char *CRpcArgs::GetData()
	{
	return m_buf;
	}
