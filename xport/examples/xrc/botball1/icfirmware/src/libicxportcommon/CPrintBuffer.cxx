#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "CPrintBuffer.h"
#include "textdisp.h"

//Number of lines to scroll backwards to start
#define SCROLL_BACK_AMOUNT 15
#define PRINTBUF_LINES 32

#define TOP_MARGIN 3

CPrintBuffer *g_printBuffer = new CPrintBuffer();

CPrintBuffer::CPrintBuffer() : 
  m_currentBufferIndex(BUF_IC),  m_onScreen(false)
{
  for(int i=0; i<NUM_BUFFERS; i++) {
      m_buffers[i] = new CTextBuf(SCROLL_BACK_AMOUNT + TD_SCREEN_HEIGHT, 
				  TD_SCREEN_WIDTH, 0, 2, TOP_MARGIN, 3);
  }
}

CPrintBuffer::~CPrintBuffer()
{
}

void CPrintBuffer::FlipPrintBuffers(int dir)
{
  unsigned short nIndex = (m_currentBufferIndex+dir+NUM_BUFFERS)%NUM_BUFFERS;
  ShowPrintBuffer(nIndex);
}

void CPrintBuffer::ShowPrintBuffer(unsigned short bufnum)
{
  unsigned short cIndex = m_currentBufferIndex;

  // Either invalid or already showing, return
  if(bufnum>NUM_BUFFERS || cIndex == bufnum) return;

  // Copy from screen buffer to the currenly active print buffer
  if(m_onScreen) {
    ptd->ScreenToTextBuf(*(m_buffers[cIndex]), false);
  }

  // Copy from the requested print buffer to the screen buffer
  ptd->TextBufToScreen(*(m_buffers[bufnum]));
  m_onScreen = true;
  m_currentBufferIndex = bufnum;
}

void CPrintBuffer::Clear()
{
  for(int i=0; i<NUM_BUFFERS; i++) {
    m_buffers[i]->Clear();
  }
  if(m_onScreen) {
    ptd->Clear();
  }
}

void CPrintBuffer::CurrPrintBufToScreen()
{
  ptd->ScreenToTextBuf(*(m_buffers[BUF_SYSTEM]), false);
  // Copy from the next active print buffer to the screen buffer
  ptd->TextBufToScreen(*(m_buffers[m_currentBufferIndex]), false);
  m_onScreen=true;
}

void CPrintBuffer::ScreenToCurrPrintBuf()
{
  // Copy from the next active print buffer to the screen buffer
  ptd->ScreenToTextBuf(*(m_buffers[m_currentBufferIndex]), false);
  m_onScreen=false;
  ptd->TextBufToScreen(*(m_buffers[BUF_SYSTEM]), false);
}

void CPrintBuffer::Print(char *message, bool isICMessage)
{
  unsigned short mIndex = isICMessage?1:0;

  if(m_onScreen && m_currentBufferIndex == mIndex) {
    // Currently showing the one requested, print directly to screen
    ptd->Print(message);
  }
  else {
    // Not currentluy showing the one requested, print to the
    // requested buffer
    m_buffers[mIndex]->Print(message);
  }
}

void CPrintBuffer::PrintfIC(char *format, ...)
{
	char buf[128];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	Print(buf, true);
}

void CPrintBuffer::Printf(char *format, ...)
{
	char buf[128];
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);
	Print(buf);
}

void CPrintBuffer::Clear(unsigned short bufnum)
{
  if(m_onScreen && m_currentBufferIndex == bufnum) {
    ptd->Clear();
  }
  else if(bufnum<NUM_BUFFERS) {
    m_buffers[bufnum]->Clear();
  }
}

void CPrintBuffer::SetPosition(unsigned short bufnum, 
			       unsigned char x, unsigned char y)
{
  if(m_onScreen && m_currentBufferIndex == bufnum) {
    ptd->SetPosition(x, y);
  }
  else if(bufnum<NUM_BUFFERS) {
    m_buffers[bufnum]->xPos = x%TD_SCREEN_WIDTH;
    m_buffers[bufnum]->yPos = (y + TOP_MARGIN + m_buffers[bufnum]->scrollPos)%m_buffers[bufnum]->lineNum;
  }
}
void CPrintBuffer::GetPosition(unsigned short bufnum, 
			       unsigned char *x, unsigned char *y)
{
  if(m_onScreen && m_currentBufferIndex == bufnum) {
    ptd->GetPosition(x, y);
  }
  else if(bufnum<NUM_BUFFERS) {
    if(x) *x = m_buffers[bufnum]->xPos;
    if(y) *y = m_buffers[bufnum]->yPos - TOP_MARGIN - m_buffers[bufnum]->scrollPos;
  }
}

void CPrintBuffer::SetBufferPosition(unsigned short bufnum,
                                unsigned char x, unsigned char y)
{
  if(m_onScreen && m_currentBufferIndex == bufnum) {
    ptd->SetBufferPosition(x, y);
  }
  else if(bufnum<NUM_BUFFERS) {
    m_buffers[bufnum]->xPos = x%TD_SCREEN_WIDTH;
    m_buffers[bufnum]->yPos = (y + TOP_MARGIN)%m_buffers[bufnum]->lineNum;
  }
}
 void CPrintBuffer::GetBufferPosition(unsigned short bufnum,
                               unsigned char *x, unsigned char *y)
{
  if(m_onScreen && m_currentBufferIndex == bufnum) {
    ptd->GetBufferPosition(x, y);
  }
  else if(bufnum<NUM_BUFFERS) {
    if(x) *x = m_buffers[bufnum]->xPos;
    if(y) *y = m_buffers[bufnum]->yPos - TOP_MARGIN;
  }
}

void CPrintBuffer::Scroll(short val)
{
        if(m_onScreen)
                ptd->Scroll(val);
}

