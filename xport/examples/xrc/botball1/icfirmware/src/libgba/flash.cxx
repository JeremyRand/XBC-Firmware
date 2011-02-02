#include "flash.h"
#include "intcont.h"
#include "textdisp.h"
#include <string.h>

#define DF_SUBTRACT(a, b)	((int)(a)-(int)(b))
#define DF_MAX(a, b)		a>b ? a : b	

unsigned int CFlash::m_blockSize;

CFlash::CFlash(CInterruptCont *pIntCont)
	{
	m_blockSize = DF_FLASH_BLOCK_SIZE;

	m_size = DF_SUBTRACT(_Erase, _Program);
	m_size = DF_MAX(m_size, DF_SUBTRACT(_Unlock, _Erase)); 
	m_size = DF_MAX(m_size, DF_SUBTRACT(_Lock, _Unlock)); 
	m_size = DF_MAX(m_size, DF_SUBTRACT(_Dummy, _Lock)); 
	m_size += 2;
	if (m_size > 0x400)
		m_size = 0x400;
		
	m_funcMemory = new char[m_size];

	SetInterruptCont(pIntCont);
	}

CFlash::~CFlash()
	{
	delete [] m_funcMemory; 
	}

void CFlash::SetInterruptCont(CInterruptCont *pIntCont)
	{
	m_pIntCont = pIntCont;
	}

int CFlash::Program(unsigned long addr, unsigned short *data, unsigned long len)
	{
	int result;

	if (m_pIntCont)
		m_pIntCont->MaskAll();
	CopyFunc((void *)_Program);
	result = (*((int (*)(unsigned long, unsigned short *, unsigned long))m_func))(addr, data, len);
	if (m_pIntCont)
		m_pIntCont->UnmaskAll();
	return result;
	}

int CFlash::Erase(unsigned long addr, unsigned long len)
	{
	int result;

	if (m_pIntCont)
		m_pIntCont->MaskAll();
	CopyFunc((void *)_Erase);
	result = (*((int (*)(unsigned long, unsigned long))m_func))(addr, len);
	if (m_pIntCont)
		m_pIntCont->UnmaskAll();
	return result;
	}

int CFlash::Unlock(unsigned long addr, unsigned long len)
	{
	int result;

	if (m_pIntCont)
		m_pIntCont->MaskAll();
	CopyFunc((void *)_Unlock);
	result = (*((int (*)(unsigned long, unsigned long))m_func))(addr, len);
	if (m_pIntCont)
		m_pIntCont->UnmaskAll();
	return result;
	}

int CFlash::Lock(unsigned long addr, unsigned long len)
	{
	int result;

	if (m_pIntCont)
		m_pIntCont->MaskAll();
	CopyFunc((void *)_Lock);
	result = (*((int (*)(unsigned long, unsigned long))m_func))(addr, len);
	if (m_pIntCont)
		m_pIntCont->UnmaskAll();
	return result;
	}

int CFlash::_Program(unsigned long addr, unsigned short *data, unsigned long len)
	{
	volatile unsigned short *programAddr;
    unsigned short stat = 0;
    int timeout;
    int i, result = 0;

	programAddr = (unsigned short *)addr;
    // Clear any error conditions
    *programAddr = DF_FLASH_CLEAR_STATUS;

    // Write any big chunks first
    while (len>=16) 
		{
        len -= 16;

        *programAddr = DF_FLASH_WRITE_BUFFER;
        timeout = DF_TIMEOUT;
        while(((stat=*programAddr)&DF_FLASH_STATUS_READY) != DF_FLASH_STATUS_READY) 
			{
            if (--timeout == 0)
				{
				result = -1;
                goto bad;
				}
            *programAddr = DF_FLASH_WRITE_BUFFER;
			}
        *programAddr = 15;  // Count is 0..N-1
        for (i = 0;  i < 16;  i++)
            *(programAddr+i) = *(data+i);
        *programAddr = DF_FLASH_CONFIRM;
    
        *programAddr = DF_FLASH_READ_STATUS;
        timeout = DF_TIMEOUT;
        while(((stat=*programAddr)&DF_FLASH_STATUS_READY) != DF_FLASH_STATUS_READY) 
			{
            if (--timeout == 0) 
				{
				result = -2;
                goto bad;
				}
			}
        // Jump out if there was an error
        if (stat & FLASH_ERROR_MASK)
			{
			result = -3;
            goto bad;
			}

        // And verify the data - also increments the pointers.
        *programAddr = DF_FLASH_RESET;            
        for (i = 0;  i < 16;  i++) 
			{
            if ( *programAddr++ != *data++ )
				{
                result = (int)programAddr;
                goto bad;
				}
			}
        }

    while (len>0) 
		{
        *programAddr = DF_FLASH_PROGRAM;
        *programAddr = *data;
        timeout = DF_TIMEOUT;
        while(((stat=*programAddr)&DF_FLASH_STATUS_READY) != DF_FLASH_STATUS_READY) 
			{
            if (--timeout == 0)
				{
				result = -5;
                goto bad;
				}
            }
        if (stat & FLASH_ERROR_MASK)
			{
			result = -6;
            break;
			}
       
        *programAddr = DF_FLASH_RESET;            
        if (*programAddr++ != *data++) 
			{
            result = -7;
            break;
			}
        len--;
	    }

    // Restore ROM to "normal" mode
 bad:
    *programAddr = DF_FLASH_RESET;            

    return result;
	}

int CFlash::_Erase(unsigned long addr, unsigned long len)
	{
    volatile unsigned short *programAddr;
    unsigned short stat = 0;
    int timeout;
	long eraseLen = len;

    // Get base address and map addresses to virtual addresses
    programAddr = (volatile unsigned short *)addr;

    // Clear any error conditions
    *programAddr = DF_FLASH_CLEAR_STATUS;

    // Erase block
    while (eraseLen > 0) 
		{
        *programAddr = DF_FLASH_BLOCK_ERASE;
        *programAddr = DF_FLASH_CONFIRM;
        timeout = DF_TIMEOUT;
        while(((stat=*programAddr)&DF_FLASH_STATUS_READY) != DF_FLASH_STATUS_READY) 
			{
            if (--timeout == 0)
				break;
            }

        eraseLen -= m_blockSize;
        programAddr += m_blockSize;
		}

    // Restore programAddr to "normal" mode
    *programAddr = DF_FLASH_RESET;

	if (stat!=DF_FLASH_STATUS_READY)
		return -1;
	return 0;
	}

int CFlash::_Unlock(unsigned long addr, unsigned long len)
	{
    volatile unsigned short *programAddr;
    unsigned short stat = 0;
    int timeout;
	long unlockLen = len;

    // Get base address and map addresses to virtual addresses
    programAddr = (volatile unsigned short *)addr;

    // Clear any error conditions
    *programAddr = DF_FLASH_CLEAR_STATUS;

    // Erase block
    while (unlockLen > 0) 
		{
        *programAddr = DF_FLASH_CLEAR_LOCK;
        *programAddr = DF_FLASH_CLEAR_LOCK_CONFIRM;
        timeout = DF_TIMEOUT;
        while(((stat=*programAddr)&DF_FLASH_STATUS_READY) != DF_FLASH_STATUS_READY) 
			{
            if (--timeout == 0)
				break;
            }

        unlockLen -= m_blockSize;
        programAddr += m_blockSize;
		}

    // Restore programAddr to "normal" mode
    *programAddr = DF_FLASH_RESET;

	if (stat!=DF_FLASH_STATUS_READY)
		return -1;
	return 0;
	}

int CFlash::_Lock(unsigned long addr, unsigned long len)
	{
    volatile unsigned short *programAddr;
    unsigned short stat = 0;
    int timeout;
	long lockLen = len;

    // Get base address and map addresses to virtual addresses
    programAddr = (volatile unsigned short *)addr;

    // Clear any error conditions
    *programAddr = DF_FLASH_CLEAR_STATUS;

    // Erase block
    while (lockLen > 0) 
		{
        *programAddr = DF_FLASH_SET_LOCK;
        *programAddr = DF_FLASH_SET_LOCK_CONFIRM;
        timeout = DF_TIMEOUT;
        while(((stat=*programAddr)&DF_FLASH_STATUS_READY) != DF_FLASH_STATUS_READY) 
			{
            if (--timeout == 0)
				break;
            }

        lockLen -= m_blockSize;
        programAddr += m_blockSize;
		}

    // Restore programAddr to "normal" mode
    *programAddr = DF_FLASH_RESET;

	if (stat!=DF_FLASH_STATUS_READY)
		return -1;
	return 0;
	}

int CFlash::_Dummy()
	{
	return 0;
	}

void CFlash::CopyFunc(void *func)
	{
	// THUMB code needs to be treated differently... 
	if (((long)func)&0x01)
		{
		memcpy(m_funcMemory, (char *)(func)-1, m_size);
		m_func = m_funcMemory+1;
		}
	else
		{
		memcpy(m_funcMemory, (char *)func, m_size);
		m_func = m_funcMemory;
		}
	}
