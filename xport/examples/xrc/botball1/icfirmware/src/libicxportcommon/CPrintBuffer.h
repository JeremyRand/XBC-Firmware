#ifndef CPRINTBUFFER_H
#define CPRINTBUFFER_H

#include "cport.h"

#define PrintToBuffer g_printBuffer->Printf
#define PrintToICBuffer g_printBuffer->PrintfIC

#define NUM_BUFFERS 3

class CTextBuf;

class CPrintBuffer
{
	CTextBuf *m_buffers[NUM_BUFFERS];
	unsigned short m_currentBufferIndex;
        bool m_onScreen;

public:
	enum
	{
		MSG_NORMAL,
		MSG_ERROR,
		MSG_CRITICAL,
		MSG_IC
	};

	enum
	{
	        BUF_FIRMWARE = 0,
		BUF_IC = 1,
		BUF_SYSTEM = 2
	};

	CPrintBuffer();
	~CPrintBuffer();

        // Print to BUF_FIRMWARE
	void Printf(char *format, ...);

        // If isICMessage is true print to BUF_IC, otherwise use BUF_FIRMWARE
	void Print(char *string, bool isICMessage = false);

        //////////////////////////////////////////////
        // Methods affecting current print buffer
       
	void FlipPrintBuffers(int dir=1);
        void CurrPrintBufToScreen();
        void ScreenToCurrPrintBuf();

        //////////////////////////////////////////////
        // Support for indexed print buffer access, see BUF_* enums above

	unsigned short GetCurrentIndex() const {
	  return(m_currentBufferIndex);
	}

	void ShowPrintBuffer(unsigned short bufnum);

	const CTextBuf *GetTextBuf(int index) const {
	  if(index<0 || index>=NUM_BUFFERS) {
	    return(0);
	  }
	  return(m_buffers[index]);
	}

        void Clear(unsigned short bufnum);
        void SetPosition(unsigned short bufnum, 
			 unsigned char x, unsigned char y);
        void GetPosition(unsigned short bufnum, 
			 unsigned char *x, unsigned char *y);
	void SetBufferPosition(unsigned short bufnum,
                         unsigned char x, unsigned char y);
        void GetBufferPosition(unsigned short bufnum,
                         unsigned char *x, unsigned char *y);
	void Scroll(short val);
        //////////////////////////////////////////////
        // Support for IC-specific print buffer access
        void ShowICPrintBuffer() {
	  ShowPrintBuffer(BUF_IC);
	}

        CTextBuf *GetICTextBuf() {
	  return(m_buffers[BUF_IC]);
	}

	void PrintfIC(char *format, ...);

        //////////////////////////////////////////////
        // Affect all print buffers

        // Clear all print buffers and screen if currently showing
	void Clear();
};

extern CPrintBuffer* g_printBuffer;

#endif
