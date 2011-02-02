#include <string.h>
#include "flashxfer.h"
#include <textdisp.h>
#include <xport.h>

extern CTextDisp td;

//#define DEBUG

char *CFlashXfer::m_buf;
unsigned int CFlashXfer::m_bufSize;
CFlash CFlashXfer::m_flash;
EXferMode CFlashXfer::m_mode = EXM_NONE;

CFlashXfer::CFlashXfer(IComm *pComm, CInterruptCont *pIntCont, unsigned int bufSize) : 
	CRpc(pComm)
	{
	m_buf = new char[bufSize];
	m_bufSize = bufSize;
	m_flash.SetInterruptCont(pIntCont);

	Register(DFX_FILLBUFFER, FillBuffer);
	Register(DFX_WRITEBUFFER, WriteBuffer);
	Register(DFX_ERASE, Erase);
	Register(DFX_LOCK, Lock);
	Register(DFX_UNLOCK, Unlock);
	Register(DFX_READ, Read);
	Register(DFX_GETBUFFERSIZE, GetBufferSize);
	Register(DFX_GETSECTORSIZE, GetSectorSize);
	Register(DFX_BUFFERCHECKSUM, BufferChecksum);
	Register(DFX_EXECUTE, Execute);
	Register(DFX_VERSION, Version);
	Register(DFX_PRINT, Print);
	Register(DFX_SETMODE, SetMode);
	Register(DFX_SETLED, SetLed);
	}

CFlashXfer::~CFlashXfer()
	{
	delete [] m_buf;
	}

EXferMode CFlashXfer::GetMode()
	{
	return m_mode;
	}

void CFlashXfer::FillBuffer(CRpcArgs *args)
	{
	int res;
	unsigned long addr;
	unsigned short len;

	res = args->Pop((long *)&addr);	
	if (res>=0 && addr<m_bufSize)
		{
		len = m_bufSize-addr>0xffff ? 0xffff : m_bufSize-addr;
		res = args->Pop(m_buf+addr, &len);
#ifdef DEBUG
		printf("Fillbuffer %d %d\n", addr, len);
#endif
		}
	
	args->Reset();
	if (res<0) // error
		args->Push((long)-1);
	else // return length
		args->Push((long)len);
	}

void CFlashXfer::WriteBuffer(CRpcArgs *args)
	{
	unsigned long addr, len;
	int res;

	args->Pop((long *)&addr);
	args->Pop((long *)&len);

	args->Reset();
	addr += DFX_MIN_FLASH_ADDR;
	if (len<=m_bufSize &&
		(m_mode==EXM_PRIVILEGED || (DFX_VALID_ADDR(addr) && DFX_VALID_ADDR(addr+len))))
		{
		
#ifdef DEBUG
		printf("Write %x %d\n", addr, len);
#endif
		if ((res=m_flash.Program(addr, (unsigned short *)m_buf, len>>1))<0)
			args->Push((long)res);
		else
			args->Push((long)len);
		}
	else
		args->Push((long)-1);
	}

void CFlashXfer::Erase(CRpcArgs *args)
	{
	unsigned long startAddr, len;
	int res;

	args->Pop((long *)&startAddr);
	args->Pop((long *)&len);
	startAddr += DFX_MIN_FLASH_ADDR;

	args->Reset();
	if (m_mode==EXM_PRIVILEGED || (DFX_VALID_ADDR(startAddr) && DFX_VALID_ADDR(startAddr+len)))
		{
#ifdef DEBUG
		printf("Erase %x %x\n", startAddr, len);
#endif
		if ((res=m_flash.Erase(startAddr, len>>1))<0)
			args->Push((long)res);
		else
			args->Push((long)0);
		}
	else
		args->Push((long)-1);
	}

void CFlashXfer::Lock(CRpcArgs *args)
	{
	unsigned long startAddr, len;
	int res;

	args->Pop((long *)&startAddr);
	args->Pop((long *)&len);
	startAddr += DFX_MIN_FLASH_ADDR;

	args->Reset();
	if (m_mode==EXM_PRIVILEGED || (DFX_VALID_ADDR(startAddr) && DFX_VALID_ADDR(startAddr+len)))
		{
#ifdef DEBUG
		printf("Lock %x %x\n", startAddr, len);
#endif
		if ((res=m_flash.Lock(startAddr, len>>1))<0)
			args->Push((long)res);
		else
			args->Push((long)0);
		}
	else
		args->Push((long)-1);
	}

void CFlashXfer::Unlock(CRpcArgs *args)
	{
	unsigned long startAddr, len;
	int res;

	args->Pop((long *)&startAddr);
	args->Pop((long *)&len);
	startAddr += DFX_MIN_FLASH_ADDR;

	args->Reset();
	if (m_mode==EXM_PRIVILEGED || (DFX_VALID_ADDR(startAddr) && DFX_VALID_ADDR(startAddr+len)))
		{
#ifdef DEBUG
		printf("Unlock %x %x\n", startAddr, len);
#endif
		if ((res=m_flash.Unlock(startAddr, len>>1))<0)
			args->Push((long)res);
		else
			args->Push((long)0);
		}
	else
		args->Push((long)-1);
	}

void CFlashXfer::Read(CRpcArgs *args)
	{
	unsigned long addr;
	unsigned short len;

	args->Pop((long *)&addr);
	args->Pop((short *)&len);

#ifdef DEBUG
	printf("Read %x %x\n", addr, len);
#endif
	args->Reset();
	args->Push((char *)(addr + DFX_MIN_FLASH_ADDR), len);
	}

void CFlashXfer::GetBufferSize(CRpcArgs *args)
	{
#ifdef DEBUG
	printf("GetBufferSize\n");
#endif
	args->Push((long)m_bufSize);
	}

void CFlashXfer::GetSectorSize(CRpcArgs *args)
	{
#ifdef DEBUG
	printf("GetBufferSize\n");
#endif
	args->Push((long)DF_FLASH_BLOCK_SIZE*2);
	}

void CFlashXfer::SetMode(CRpcArgs *args)
	{
	unsigned long magicNumber;

	args->Pop((long *)&magicNumber);
	args->Reset();
#ifdef DEBUG
	printf("SetMode %x\n", magicNumber);
#endif
	if (magicNumber==DFX_NORMAL_MODE)
		{
		m_mode = EXM_NORMAL;	
		args->Push((long)0);
		}
	else if (magicNumber==DFX_PRIVILEGED_MODE)
		{
		m_mode = EXM_PRIVILEGED;
		args->Push((long)0);
		}
	else
		args->Push((long)-1);
	}

void CFlashXfer::BufferChecksum(CRpcArgs *args)
	{
	unsigned long i, len;
	unsigned short checksum;

	args->Pop((long *)&len);

	args->Reset();
	if (len<=m_bufSize)
		{
		for (i=0, checksum=0; i<len; i++)
			checksum += (unsigned char)m_buf[i];
		args->Push((long)0); // success
		args->Push((short)checksum);
#ifdef DEBUG
	printf("BufferChecksum %x %x\n", len, checksum);
#endif
		}
	else
		args->Push((long)-1);
	}

void CFlashXfer::Execute(CRpcArgs *args) 
	{
	void (*func)();
	args->Pop((long *)&func);
	//#ifdef DEBUG
	printf("Executing %x\n", func);
	//#endif
	
	(*func)();
	}


void CFlashXfer::Version(CRpcArgs *args) 
	{
#ifdef DEBUG
	printf("Version\n");
#endif
	args->Reset();
	args->Push(DFX_XFER_VERSION, sizeof(DFX_XFER_VERSION));
	args->Push(DFX_HARDWARE_VERSION, sizeof(DFX_HARDWARE_VERSION));
	}

void CFlashXfer::SetLed(CRpcArgs *args) 
	{
	unsigned char green, red;
	unsigned short val = 0;

	args->Pop((char *)&green);
	args->Pop((char *)&red);

	if (green)
		val = XP_LED_GREEN;
	if (red)
		val |= XP_LED_RED;

	XP_REG_LED = val;

	args->Reset(); // void
	}

void CFlashXfer::Print(CRpcArgs *args) 
	{
	char buf[0x80];
	unsigned short len = 0x80;

	args->Pop(buf, &len);
	printf(buf);

	args->Reset(); // void
	}

