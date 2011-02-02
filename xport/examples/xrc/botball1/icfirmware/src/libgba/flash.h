#ifndef _FLASH_H
#define _FLASH_H

#define DF_FLASH_READ_ID      			0x90
#define DF_FLASH_READ_QUERY  			0x98  
#define DF_FLASH_READ_STATUS  			0x70
#define DF_FLASH_CLEAR_STATUS			0x50
#define DF_FLASH_STATUS_READY 			0x80
#define DF_FLASH_PROGRAM				0x10
#define DF_FLASH_BLOCK_ERASE  			0x20
#define DF_FLASH_WRITE_BUFFER 			0xE8
#define DF_FLASH_SET_LOCK  				0x60
#define DF_FLASH_SET_LOCK_CONFIRM 		0x01
#define DF_FLASH_CLEAR_LOCK  			0x60
#define DF_FLASH_CLEAR_LOCK_CONFIRM	0xD0
#define DF_FLASH_CONFIRM     			0xD0
#define DF_FLASH_RESET   				0xFF
                                                     
// Status that we read back:                         
#define FLASH_ERROR_MASK				0x7E
#define FLASH_ERROR_PROGRAM				0x10
#define FLASH_ERROR_ERASE				0x20

#define DF_FLASH_BLOCK_SIZE				0x10000 // words
#define DF_TIMEOUT						1000000

class CInterruptCont;

class CFlash
	{
public:
	// Constructor requires pointer to interrupt controller so we can disable interrupts during
	// flash operations.
	CFlash(CInterruptCont *pIntCont=0);
	~CFlash();
	void SetInterruptCont(CInterruptCont *pIntCont); 
	
	// For all methods: If successful, return 0 or greater.  If error, return negative value.  
	// For all methods: 'len' is specified in 16-bit words -- the width of the flash device.

	// 'Program' requires a pointer into RAM (not flash) for the source data.
	int Program(unsigned long addr, unsigned short *data, unsigned long len); 

	// 'Erase', 'Unlock', and 'Lock' will operate on the block associated with the address.  
	// A block is 128K bytes or 64K words for the Intel Strataflash.  That is, the lower 16 bits
	// of 'addr' are tossed.
	int Erase(unsigned long addr, unsigned long len);
	int Unlock(unsigned long addr, unsigned long len);
	int Lock(unsigned long addr, unsigned long len);

private:
	static int _Program(unsigned long addr, unsigned short *data, unsigned long len);
	static int _Erase(unsigned long addr, unsigned long len);
	static int _Unlock(unsigned long addr, unsigned long len);
	static int _Lock(unsigned long addr, unsigned long len);
	static int _Dummy();

	void CopyFunc(void *func);

	char *m_funcMemory;
	char *m_func;
	unsigned int m_size;
	CInterruptCont *m_pIntCont;

	static unsigned int m_blockSize;
	};

#endif
