#ifndef _RPCARGS_H
#define _RPCARGS_H

#define DRA_DEFAULT_SIZE		0x100

class CRpc;

class CRpcArgs
	{
public:
	CRpcArgs(unsigned int size=DRA_DEFAULT_SIZE);
	~CRpcArgs();

	void Reset();

	int Push(char arg);
	int Push(short arg);
	int Push(long arg);
	int Push(char *arg, unsigned short len);

	int Pop(char *arg);
	int Pop(short *arg);
	int Pop(long *arg);
	int Pop(char *arg, unsigned short *len);

	unsigned short GetLen();		// get amount of data 
	unsigned short GetSize();		// get size of buffer 
	char *GetData();				// get data pointer

private:
	friend class CRpc;

	int Renew(unsigned short len);

	inline int CheckPush(unsigned short len)
		{
		if (m_pushIndex+len > m_size)
			return Renew(m_pushIndex+len);
		else
			return 0;
		}
	inline int CheckPop(unsigned int len)
		{
		if (m_popIndex+len > m_pushIndex)
			return -1;
		else
			return 0;
		}

	char         *m_buf;
	unsigned int m_pushIndex; // index into m_buf
	unsigned int m_popIndex;  // index into m_buf
	unsigned int m_size;  // size of m_buf
	};


#endif
