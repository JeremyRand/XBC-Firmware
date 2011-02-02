#include <fstream>
#include <string.h>
#include "flashxferhost.h"
#include "bitstream.h"

CFlashXferHost::CFlashXferHost(IComm *pComm, int retries) : CRpc(pComm)
	{
	m_retries = retries;
	}

CFlashXferHost::~CFlashXferHost()
	{
	}

int CFlashXferHost::Write(unsigned long addr, char *buf, unsigned long len, bool erase)
	{
	std::string str(buf, len);
	std::istringstream  is(str);

	return Write(addr, is, erase);
	}


int CFlashXferHost::Write(unsigned long addr, char *filename, bool erase)
	{
	int res;
	char buf[0x100];
	std::ifstream is(filename, std::ios::binary);

	if (is.fail() || is.eof())
		return -1;

	sprintf(buf, "Uploading %s\n", filename);
	Print(buf);
	res = Write(addr, is, erase);
	Print("\n");
	if (res<0)
		Print("Error\n");
	else
		Print("Success\n");
	return res;
	}

int CFlashXferHost::Write(unsigned long addr, std::istream &is, bool erase)
	{
	int res;
	CRpcArgs args;
	unsigned long len, bufSize;
	char *buf;

	// addresses must be on 16-bit boundaries
	if (addr&1)
		return -1;

	if ((res=Call(DFX_GETBUFFERSIZE, &args))<0)
		return res;

	args.Pop((long *)&bufSize);
	
	buf = new char[bufSize];

	while(1)
		{
		is.read(buf, bufSize);
		len = is.gcount();
		if (len==0)
			break;
		if ((res=WriteBuffer(addr, buf, len, erase))<0)
			return res;
		addr += len;
		}
	delete [] buf;
	return 0;
	}

int CFlashXferHost::WriteBuffer(unsigned long addr, char *data, unsigned long len, bool erase)
	{
	int i, j, res;
	CRpcArgs args;
	unsigned short chunk, timeout, checksum, checksumCalc;
	char *dataCount;
	unsigned long writeCount = 0;

	SetLed(1, 0); // green LED	
	// upload to buffer
	for (j=0; j<m_retries; j++)
		{
		for (writeCount=0, checksumCalc=0, dataCount=data; 
		     writeCount<len; 
			 writeCount+=DFXH_PACKET_SIZE_TRANSMIT, dataCount+=chunk)
			{
			Print(".");
			chunk = DFXH_PACKET_SIZE_TRANSMIT>len-writeCount ? len-writeCount : DFXH_PACKET_SIZE_TRANSMIT;
			for (i=0; i<chunk; i++)
				checksumCalc += (unsigned char)dataCount[i];
			for (i=0; i<m_retries; i++)
				{
				args.Reset();
				args.Push((long)writeCount);
				args.Push(dataCount, chunk);
				if ((res=Call(DFX_FILLBUFFER, &args))>=0)
					break;
				}
			if (res<0)
				return res;
			}
		
		// compare checksum
		args.Reset();
		args.Push((long)len);
		if ((res=Call(DFX_BUFFERCHECKSUM, &args))<0)
			continue;
		args.Pop((long *)&res);
		if (res==0)
			{
			args.Pop((short *)&checksum);
			if (checksumCalc!=checksum)
				res = -1;
			else
				break;
			}
		}
	if (res<0)
		goto end;
	
	// only after we have uploaded and checked the entire buffer checksum do we
	// proceed with erasing and writing

	
	timeout = GetTimeout();
	SetTimeout(10000);
	// write buffer
	for (i=0; i<m_retries; i++)
		{
		// unlock
		if ((res=Unlock(addr, len))<0)
			continue;

		// erase
		if (erase && (res=Erase(addr, len))<0)
			continue;

		// write
		args.Reset();
		args.Push((long)addr);
		args.Push((long)len);
		if ((res=Call(DFX_WRITEBUFFER, &args))<0)
			continue;
		args.Pop((long *)&res);
		if (res>=0)
			break;

		// unlock
		Lock(addr, len);
		}

	SetTimeout(timeout);

	end:
	if (res<0)
		SetLed(0, 1); // red
	else
		SetLed(0, 0); // off, success
	return res;
	}

int CFlashXferHost::WriteBitstream(char *filename)
	{
	int res;
	char bs[DFXH_BITSTREAM_SIZE], buf[0x100];
	unsigned long len = DFXH_BITSTREAM_SIZE-DFXH_BITSTREAM_SIZE_STUFF;
	SBitstreamInfo info;
	std::ifstream is(filename, std::ios::binary);

	if (is.fail() || is.eof())
		return -1;

	memset(bs, 0xff, DFXH_BITSTREAM_SIZE);
	if ((res=CBitstream::ParseBitstream(is, (unsigned char *)bs+DFXH_BITSTREAM_SIZE_STUFF, &len, &info))<0)
		return res;

	// write header
	memcpy(bs+DFXH_BITSTREAM_SIZE-sizeof(SBitstreamInfo), (char *)&info, sizeof(SBitstreamInfo));

	sprintf(buf, "Uploading bitstream %s\n", filename);
	Print(buf);
	SetLed(1, 0); // green LED
	// unlock
	if ((res=Unlock(0x3c0000, DFXH_BITSTREAM_SIZE))<0)
		goto end;
	// write bitstream
	if ((res=Write(0x3c0000, bs, (unsigned long)DFXH_BITSTREAM_SIZE))<0)
		goto end;
	// relock
	res = Lock(0x3c0000, DFXH_BITSTREAM_SIZE);

	end:
	Print("\n");
	if (res<0)
		{
		SetLed(0, 1); // red LED
		Print("Error\n");
		}
	else
		{
		SetLed(0, 0); // turn off LED, success
		Print("Success\n");
		}
	return res;
	}

int CFlashXferHost::Read(unsigned long addr, char *buf, unsigned long len)
	{
	CRpcArgs args;
	int res;
	unsigned long i;
	unsigned short chunk;
	unsigned short bufLen;

	SetLed(1, 0); // green led
	for (i=0; i<len; i+=DFXH_PACKET_SIZE_RECEIVE)
		{
		chunk = len-i<DFXH_PACKET_SIZE_RECEIVE ? len-i : DFXH_PACKET_SIZE_RECEIVE;
		args.Reset();
		args.Push((long)(addr+i));
		args.Push((short)chunk);
		if ((res=Call(DFX_READ, &args))<0)
			{
			SetLed(0, 1); // red, error
			return res;
			}
		bufLen = len-i>0xffff ? 0xffff : len-i;
		if ((res=args.Pop(buf+i, &bufLen))<0)
			{
			SetLed(0, 1); // red, error
			return res;
			}
		if (bufLen!=chunk)
			{
			SetLed(0, 1); // red, error
			return -1;
			}
		}

	SetLed(0, 0); // off, success
	return len;
	}


int CFlashXferHost::Erase(unsigned long startAddr, unsigned long len)
	{
	CRpcArgs args;
	int res;
	int i;

	// addresses must be on 16-bit boundaries
	if (startAddr&1 || len&1)
		return -1;

	for (i=0; i<m_retries; i++)
		{
		args.Reset();
		args.Push((long)startAddr);
		args.Push((long)len);
		if ((res=Call(DFX_ERASE, &args))>=0)
			break;
		}

	if (res<0)
		return res;

	args.Pop((long *)&res);
	return res;
	}

int CFlashXferHost::Lock(unsigned long startAddr, unsigned long len)
	{
	CRpcArgs args;
	int res;
	int i;

	// addresses must be on 16-bit boundaries
	if (startAddr&1 || len&1)
		return -1;

	for (i=0; i<m_retries; i++)
		{
		args.Reset();
		args.Push((long)startAddr);
		args.Push((long)len);
		if ((res=Call(DFX_LOCK, &args))>=0)
			break;
		}

	if (res<0)
		return res;

	args.Pop((long *)&res);
	return res;
	}

int CFlashXferHost::Unlock(unsigned long startAddr, unsigned long len)
	{
	CRpcArgs args;
	int res;
	int i;

	// addresses must be on 16-bit boundaries
	if (startAddr&1 || len&1)
		return -1;

	for (i=0; i<m_retries; i++)
		{
		args.Reset();
		args.Push((long)startAddr);
		args.Push((long)len);
		if ((res=Call(DFX_UNLOCK, &args))>=0)
			break;
		}

	if (res<0)
		return res;

	args.Pop((long *)&res);
	return res;
	}

int CFlashXferHost::Print(char *string)
	{
	int res;
	CRpcArgs args;

	args.Push(string, strlen(string)+1);
	res = Call(DFX_PRINT, &args);

	return res;
	}

int CFlashXferHost::Version(char *boot, char *hardware)
	{
	CRpcArgs args;
	unsigned short len;
	int res;
	
	res = Call(DFX_VERSION, &args);

	if (res<0)
		return res;

	len = 16;
	args.Pop(boot, &len);
	len = 16;
	args.Pop(hardware, &len);

	return res;
	}

int CFlashXferHost::GetSectorSize(unsigned long *size)
	{
	CRpcArgs args;
	int res;

	if ((res=Call(DFX_VERSION, &args))<0)
		return res;
	args.Pop((long *)size);
	return 0;
	}

int CFlashXferHost::Execute(unsigned long addr)
	{
	int res;
	CRpcArgs args;

	args.Reset();
	args.Push((long)addr);
	res = Call(DFX_EXECUTE, &args);

	return res;
	}

int CFlashXferHost::SetLed(char green, char red)
	{
	CRpcArgs args;

	args.Push((char)green);
	args.Push((char)red);
	return Call(DFX_SETLED, &args);
	}

int CFlashXferHost::SetMode(EXferMode mode)
	{
	int i, res;
	CRpcArgs args;

	for(i=0; i<m_retries; i++)
		{
		args.Reset();
		if (mode==EXM_NORMAL)
			args.Push((long)DFX_NORMAL_MODE);
		else if (mode==EXM_PRIVILEGED)
			args.Push((long)DFX_PRIVILEGED_MODE);
		else
			return -1;
		if ((res=Call(DFX_SETMODE, &args))>=0)
			break;
		}
	if (res<0)
		return res;

	args.Pop((long *)&res);
	return res;
	}
