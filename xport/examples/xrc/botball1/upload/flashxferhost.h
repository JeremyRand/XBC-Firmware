#ifndef _FLASHXFERHOST_H
#define _FLASHXFERHOST_H

#include "../../librpc/rpc.h"
#include "../bootloader/flashxfertab.h"
#include <sstream>

#define DFXH_PACKET_SIZE_TRANSMIT	0x800
#define DFXH_PACKET_SIZE_RECEIVE	0x200
#define DFXH_BITSTREAM_SIZE			0x20000
#define DFXH_BITSTREAM_SIZE_STUFF	0x380
#define DFXH_DEFAULT_RETRIES		3

class CFlashXferHost : public CRpc
	{
public:
	CFlashXferHost(IComm *pComm, int retries=DFXH_DEFAULT_RETRIES);
	virtual ~CFlashXferHost();

	int Write(unsigned long addr, char *buf, unsigned long len, bool erase=true);
	int Write(unsigned long addr, std::istream &is, bool erase=true);
	int Write(unsigned long addr, char *filename, bool erase=true);
	int WriteBitstream(char *filename);
	int Read(unsigned long addr, char *buf, unsigned long len);
	int Erase(unsigned long startAddr, unsigned long len);
	int Lock(unsigned long startAddr, unsigned long len);
	int Unlock(unsigned long startAddr, unsigned long len);
	int GetSectorSize(unsigned long *size);
	int Execute(unsigned long addr);
	int Print(char *string);
	int SetLed(char green, char red);
	int Version(char *boot, char *hardware);
	int SetMode(EXferMode mode);

private:
	int WriteBuffer(unsigned long addr, char *data, unsigned long len, bool erase=true);

	int m_retries;
	};


#endif