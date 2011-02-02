//  BEGIN LICENSE BLOCK
// 
//  Version: MPL 1.1
// 
//  The contents of this file are subject to the Mozilla Public License Version
//  1.1 (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
// 
//  Software distributed under the License is distributed on an "AS IS" basis,
//  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
//  for the specific language governing rights and limitations under the
//  License.
// 
//  The Original Code is The Xport Software Distribution.
// 
//  The Initial Developer of the Original Code is Charmed Labs LLC.
//  Portions created by the Initial Developer are Copyright (C) 2003
//  the Initial Developer. All Rights Reserved.
// 
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
// 
//  END LICENSE BLOCK 

#ifndef _TEXTDISP_H
#define _TEXTDISP_H

#include "cport.h"

#define TD_SCREEN_WIDTH		30
#define TD_SCREEN_VWIDTH	32
#define TD_SCREEN_HEIGHT	20
#define TD_SCREEN_VHEIGHT	64

#define	TDM_LCD				0
#define	TDM_CPORT			1
#define TDM_LCD_AND_CPORT	2

#define printf CTextDisp::SafePrintf

extern "C" 
	{
	void TDInit(unsigned char mode);
	void TDPrintf(const char *format, ...);
	void TDPrint(const char *buf);
	}

enum DispProp
	{
	DP_REVERSE,
	DP_LMARGIN,
	DP_RMARGIN,
	DP_BMARGIN
//	DP_COLOR, 
//	DP_ORIENTATION
//	DP_ITALIC,
//	DP_BOLD
	};

// If height < TD_SCREEN_HEIGHT the scrolling may not work right...

class CTextBuf {
public:
  // Use this constructor if you want to write into a pre-existing
  // textMap, such as directly to screen memory.  Be sure that the
  // allocated space of textMap is at least
  // lnum*lwidth*sizeof(unsigned short)
  CTextBuf(unsigned short *tMap, int lnum, int lwidth, 
	   int lMargin=0, int rMargin=0, 
	   int tMargin=0, int bMargin=0);
  // Use this constructor if you want this text buffer to allocate and
  // own the storage for textMap
  CTextBuf(int lnum=TD_SCREEN_VHEIGHT, int lwidth=TD_SCREEN_VWIDTH, 
	   int lMargin=0, int rMargin=0, 
	   int tMargin=0, int bMargin=0);

  ~CTextBuf();
	
  short Clear();
  short crlf();
  short ScrollToTextEnd();
  short Scroll(short val);

  // Put text into the buffer
  short Print(const char *string);
  short Printf(const char *format, ...);

  // Get properties of the buffer
  unsigned short TopLine() const {
    if(!wrapped || !enableWrap) 
      return(0);
    else 
      return((yPos + bottomMargin + 1)%lineNum);
  }
  unsigned short BottomLine() const {
    return((yPos + bottomMargin)%lineNum);
  }
  unsigned short ValidLines() const {
    if(wrapped || !enableWrap) 
      return(lineNum);
    else 
      return(yPos + bottomMargin);
  }
  
  unsigned short ScrollPos() const {
    return(scrollPos);
  }
  unsigned short EndScrollPos() const {
    if(ValidLines()>=TD_SCREEN_HEIGHT) {
      return((yPos + bottomMargin + 1 + lineNum - TD_SCREEN_HEIGHT)%lineNum);
    }
    else {
      return(0);
    }
  }
  unsigned short Wrap(short val) const {
    if(val<0) {
      val += lineNum;
    }
    return(val%lineNum);
  }

  void SetWrapEnable(bool enable) {
    enableWrap = enable;
  }

  // This copies the contents, margins, positions, and scroll from buf
  // to *this while preserving the dimensions and moving topLine to 0
  // If optimizeCopy is true it only copies the ValidLines lines.  If
  // false, it copies as many lines as it can.  Set to true for
  // optimization in the simple scrolling-text case.  Set to false for
  // completeness in the presence of random-access screen accesses.
  short CopyFromTextBuf(const CTextBuf &buf, bool optimizeCopy=true);
   
public:
  // Number of lines in buffer
  unsigned short lineNum;
  // Number of characters per line
  unsigned short lineWidth;
  // Properties
  //   Margins
  unsigned short leftMargin;
  unsigned short rightMargin;
  unsigned short topMargin;
  unsigned short bottomMargin;
  //   Display options
  bool reverse;
  bool enableWrap;

  // Current state
  unsigned short scrollPos;
  bool wrapped;
  unsigned short xPos;
  unsigned short yPos;

  // Buffer to write to
  unsigned short *textMap;

  // This is true if the text buffer should free textMap when destructed
  bool ownsStorage;
};

class CTextContext;

class CTextDisp 
	{
	  // CTextContext needs access to GetScreenTextBuf, so it is a friend
	  friend class CTextContext;

	public:
	CTextDisp(unsigned char mode=TDM_LCD);
	~CTextDisp();
	short Print(const char *string);
	short Printf(const char *format, ...);
	short GetKey(unsigned short *key);
	short Clear();
	short SetProperty(DispProp property, short val);
	short SetPosition(unsigned char x, unsigned char y);
	short GetPosition(unsigned char *x, unsigned char *y);
	short SetBufferPosition(unsigned char x, unsigned char y);
        short GetBufferPosition(unsigned char *x, unsigned char *y);
	short Scroll(short val);
	  short ScrollToTextEnd();
	  

	  // Context switching support 
	  void SetupDisplay();
	  //   Backup video memory to an in-memory buffer
	  void BackupDisplay();
	  //   Restore in-memory buffer onto real display
	  void RestoreDisplay();

	  // Copy contents and margins from buf and display on the screen
	  void TextBufToScreen(const CTextBuf &buf, bool optimizeCopy=true);
	  // Copy contents and margins from screen into buf
	  void ScreenToTextBuf(CTextBuf &buf, bool optimizeCopy=true);

	  // These static methods are used by the printf alias, and are
	  // safe to call regardless of whether ptd is set yet or not.
	  static short SafePrintf(const char *format, ...);
	  static short SafePrint(const char *string);
	protected:
	  // This allows friends to directly access the screen
	  // CTextBuf.  This is currently used by CTextContext to
	  // limit manual scrolling to stop at TopLine and BottomLine.
	  const CTextBuf *GetScreenTextBuf() const {
	    return(textBuf);
	  }

	private:
	  short Init();
	  // If we're writing to the screen set the scroll register
	  // based on textBuf.  This is an internal function and
	  // assumes that it will only be called if screen display is
	  // enabled and textBuf is non-NULL
	  void SetScrollReg();

	  // textBackupMap points to an in-memory buffer to use
	  // as backup when the display is showing a different context
	  unsigned short *textBackupMap;
	  // textBuf maintains the display state and properties.  Its
	  // textMap points at the current place to render text, which
	  // can either point to display RAM or textBackupMap
	  // depending on whether the display is showing this context
	  // or a different one
	  CTextBuf *textBuf;
	  short lastScrollPos;

	  unsigned char mode;
	  CCport cport;
	};

extern CTextDisp * ptd;

#endif
