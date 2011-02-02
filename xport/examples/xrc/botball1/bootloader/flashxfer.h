#ifndef _FLASHXFER_H
#define _FLASHXFER_H

#include "../../librpc/rpc.h"
#include "flash.h"
#include "flashxfertab.h"

#define DFX_MAX_FLASH_ADDR				0x83c0000
#define DFX_MIN_FLASH_ADDR				0x8020000
#define DFX_VALID_ADDR(addr)			(addr>=DFX_MIN_FLASH_ADDR && addr<DFX_MAX_FLASH_ADDR)


#define DFX_XFER_VERSION				"XFER 1.0"
// hardware version should be set externally 
#ifndef DFX_HARDWARE_VERSION		
#define DFX_HARDWARE_VERSION			"unknown"
#endif

class CInterruptCont;

class CFlashXfer : public CRpc
	{
public:
	CFlashXfer(IComm *pComm, CInterruptCont *pIntCont, unsigned int bufSize=DF_FLASH_BLOCK_SIZE*2);
	virtual ~CFlashXfer();

	static EXferMode GetMode();

	static void FillBuffer(CRpcArgs *args);
	static void WriteBuffer(CRpcArgs *args);
	static void Erase(CRpcArgs *args);
	static void Lock(CRpcArgs *args);
	static void Unlock(CRpcArgs *args);
	static void Read(CRpcArgs *args); 
	static void GetBufferSize(CRpcArgs *args); 
	static void GetSectorSize(CRpcArgs *args);
	static void BufferChecksum(CRpcArgs *args);
	static void Execute(CRpcArgs *args);
	static void Version(CRpcArgs *args); 
	static void Print(CRpcArgs *args); 
	static void SetMode(CRpcArgs *args);
	static void SetLed(CRpcArgs *args);

private:
	static char *m_buf;
	static unsigned int m_bufSize;
	static CFlash m_flash;
	static EXferMode m_mode;
	};

#endif
