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

#include <stdarg.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>
#include "gba.h"
#include "textdisp.h"
#include "palette.h"
#include "display.h"
#include "dispcontext.h"
#include "textcontext.h"
#include "regbits.h"

extern const unsigned char fontData[];

CTextDisp *ptd = 0;

void TDInit(unsigned char mode)
{
    CTextDisp td(mode);
    ptd = (CTextDisp *)malloc(sizeof(CTextDisp));
    *ptd = td;
}

void TDPrintf(const char *format, ...)
{
    char buf[128];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    CTextDisp::SafePrint(buf);
}

void TDPrint(const char *buf)
{
    CTextDisp::SafePrint(buf);
}

/////////////////////////////////////////////////////////////////////
// Static methods to support making printf alias safe regardless of
// whether or not ptd is set yet
short 
CTextDisp::SafePrintf(const char *format, ...)
{
    if(ptd==0) {
        return(-1);
    }
    char buf[128];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    return(ptd->Print(buf));
}
short 
CTextDisp::SafePrint(const char *string)
{
    if(ptd==0) {
        return(-1);
    }
    return(ptd->Print(string));
}
/////////////////////////////////////////////////////////////////////
// Text Buffer support

// Use this constructor if you want to write into a pre-existing
// textMap, such as directly to screen memory.  Be sure that the
// allocated space of textMap is at least
// lnum*lwidth*sizeof(unsigned short)
CTextBuf::CTextBuf(unsigned short *tMap, int lnum, int lwidth, 
		   int lMargin, int rMargin, 
		   int tMargin, int bMargin) :
    lineNum(lnum), lineWidth(lwidth), 
    leftMargin(lMargin), rightMargin(rMargin), 
    topMargin(tMargin), bottomMargin(bMargin), 
    reverse(false), enableWrap(true),
    scrollPos(0), wrapped(false),
    xPos(leftMargin), yPos(topMargin), 
    textMap(tMap), ownsStorage(false) 
{
    Clear();
}

// Use this constructor if you want this text buffer to allocate and
// own the storage for textMap
CTextBuf::CTextBuf(int lnum, int lwidth,
		   int lMargin, int rMargin, 
		   int tMargin, int bMargin):
    lineNum(lnum), lineWidth(lwidth), 
    leftMargin(lMargin), rightMargin(rMargin), 
    topMargin(tMargin), bottomMargin(bMargin), 
    reverse(false), enableWrap(true),
    scrollPos(0), wrapped(false),
    xPos(leftMargin), yPos(topMargin), 
    textMap(NULL), ownsStorage(true) 
{
    textMap = (unsigned short*)malloc(lnum*lwidth*sizeof(unsigned short));
    Clear();
}

CTextBuf::~CTextBuf()
{
    if(ownsStorage) {
        free(textMap);
    }
}

short 
CTextBuf::Clear()
{
    unsigned char x, y;

    xPos = leftMargin;
    yPos = topMargin;
    reverse = false;
    scrollPos = 0;
    wrapped=false;

    unsigned short spaceIndex = CDispFont::AsciiToTileIndex(' ');

    for(y=0; y<lineNum; y++)
        for(x=0; x<lineWidth; x++)
            textMap[y*lineWidth + x] = spaceIndex;
}

short 
CTextBuf::crlf()
{
    unsigned char x;
    unsigned short temp16, line;
    unsigned short syPos = yPos;
    // If wrapping is enabled, have a endBlank of 1 at the end, otherwise don't
    unsigned short endBlank = (enableWrap)?1:0;

    xPos = leftMargin;
    yPos = Wrap(yPos+1);
    if(yPos < syPos || yPos + bottomMargin + endBlank>= lineNum) {
        wrapped=true;
    }
    //if(ValidLines()>=TD_SCREEN_HEIGHT) {
    //    ScrollToTextEnd();
    //}

    if(!enableWrap) {
        // If not wrapping, do not clear lines
        return 0;
    }
    // clear line(s) TODO: Can we just clear the last one and assume the
    // rest are ok from earlier crlf calls?
    unsigned short spaceIndex = CDispFont::AsciiToTileIndex(' ');
    for(line=0; line<1+bottomMargin; line++) {
        temp16 = Wrap(yPos+line)*lineWidth; 
        for(x=0; x<lineWidth; ++x)
            textMap[temp16 + x] = spaceIndex;
    }
    return 0;
}

short 
CTextBuf::Print(const char *string)
{
    unsigned short i, temp16;

    temp16 = (yPos%lineNum)*lineWidth;
    for (i=0; string[i]; i++) {
        if (xPos>=lineWidth-rightMargin || string[i]==10 || string[i]==13) {
            crlf();
            temp16 = (yPos%lineNum)*lineWidth;
        }
    
        if (string[i]==8 && xPos) {
            xPos--;
        } else if(string[i]==10 || string[i]==13) {
            // Non-printing control characters, skip them
        } else if(!enableWrap && wrapped) {
            // Not wrapping and already full, ignore the rest of the text
        } else {
            textMap[temp16 + xPos] = CDispFont::AsciiToTileIndex(string[i], 
                                                                 reverse);
            xPos++;
        }
    }
        if(yPos-topMargin < scrollPos && yPos-topMargin >= 0)
 	                 scrollPos = yPos-topMargin;
 	         if(yPos > scrollPos+TD_SCREEN_HEIGHT-bottomMargin - 1)
 	                 scrollPos += yPos-(scrollPos+TD_SCREEN_HEIGHT-bottomMargin-1);
        //while((yPos - topMargin) < scrollPos)
        //      Scroll(-1);
        //while(yPos > scrollPos+TD_SCREEN_HEIGHT-bottomMargin - 1)
        //      Scroll(1);
}

short 
CTextBuf::ScrollToTextEnd()
{
    scrollPos = EndScrollPos();
}

short 
CTextBuf::Scroll(short val)
{
    scrollPos = Wrap(scrollPos + val);
}

// This copies the contents, margins, and x,y positions buf to *this
// while preserving the dimensions and moving topLine to 0.  Scroll is
// set to point to the end of text.
short 
CTextBuf::CopyFromTextBuf(const CTextBuf &buf, bool optimizeCopy)
{
    unsigned short i, bufLine, cnum, nlines, sline;

    // cnum is the number of characters per line to copy, which is the smaller 
    // of the lineWidth's of the two buffers
    cnum = buf.lineWidth;
    if(cnum > lineWidth) {
        cnum = lineWidth;
    }

    // nlines is the number of lines to copy, which is the smaller of
    // the number of valid lines in the source buffer and the total
    // number of lines in the destination buffer
    if(optimizeCopy) {
        nlines = buf.ValidLines();
    }
    else {
        nlines = buf.lineNum;
    }

    if(nlines > lineNum) {
        nlines = lineNum;
    }

    // slines is the line in buf to start copying from.  This takes into
    // account the possibility that the destination buffer is smaller
    // than the number of valid lines in the source buffer
    if(buf.enableWrap) {
        if(optimizeCopy || 
           (buf.lineNum > lineNum && buf.ValidLines()>nlines)) {
            sline = buf.Wrap(buf.BottomLine() - nlines);
        }
        else {
            sline = buf.TopLine();
        }
    }
    else {
        sline = 0;
    }

    // Clear the destination buffer
    Clear();

    // Copy the last nlines lines from the source buffer to the destination
    for(i=buf.topMargin; i<nlines; i++) {
        bufLine = buf.Wrap(sline+i);
        memcpy(textMap+i*lineWidth, 
               buf.textMap+bufLine*buf.lineWidth, 
               cnum*sizeof(unsigned short));
    }
    // Copy the margins, correcting rightMargin if necessary to keep
    // text width constant
    leftMargin = buf.leftMargin;
    rightMargin = (lineWidth - buf.lineWidth) + buf.rightMargin ;
    bottomMargin = buf.bottomMargin;
    topMargin = buf.topMargin;

    // Copy the x and y positions, accounting for relocation of the top
    // and potential size differences when copying yPos
    xPos = buf.xPos;
    if(optimizeCopy || 
       (buf.lineNum > lineNum && buf.ValidLines()>nlines)) {
        yPos = nlines - bottomMargin;
    }
    else {
        yPos = buf.yPos;
    }

    // Set scroll to point to the end of the valid text
    ScrollToTextEnd();
    //   Printf("DB: %d %d %d %d %d %d\n", yPos, lineNum, 
    // 	 TopLine(), BottomLine(), ValidLines(), scrollPos);
    //   Printf("SB: %d %d %d %d %d %d\n", buf.yPos, buf.lineNum, 
    // 	 buf.TopLine(), buf.BottomLine(), buf.ValidLines(), 
    // 	 buf.scrollPos);
    //   Printf("CP: %d %d\n", sline, nlines);
    return(0);
}

short CTextBuf::Printf(const char *format, ...)
{
    char buf[128];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    Print(buf);
}

/////////////////////////////////////////////////////////////////////
// Display context switching support
//   Setup video registers to display text

void 
CTextDisp::SetupDisplay()
{
    // Set LCD Control register to mode 0, BG2 enabled, all other layers
    // left as they were

    //Turn on Mode 0, background 1, and sprite drawing
    BFSET(GBA_REG_DISPCNT, videoMode, 0);

    //Turn on BG2 
    BFSET(GBA_REG_DISPCNT, drawBG2, 1);

    // Set BG2 control register to 2rd priority, char base block 0,
    // no mosaic, 256 color palette, screen base block 8, enable
    // wrapping, screen size 2 (4K)
    //GBA_REG_BG2CNT = 0xa880;
    //Turn on Background 0 settings 
    //0 = 256x256, since our background is 240x160, this should be fine
    BFSET(GBA_REG_BG2CNT, backgroundSize, 0x02);
	BFSET(GBA_REG_BG2CNT, priority, 0x2);
	BFSET(GBA_REG_BG2CNT, baseTileBlock, 0x0);
	BFSET(GBA_REG_BG2CNT, baseMapBlock, (unsigned char)8);
	BFSET(GBA_REG_BG2CNT, mosaic, 0x0);
	BFSET(GBA_REG_BG2CNT, palette, 0x1);
	BFSET(GBA_REG_BG2CNT, wrapFlag, 0x1); 
}

void
CTextDisp::BackupDisplay()
{
    assert(textBuf!=NULL);

    // Create textBackupMap if not already allocated
    if(textBackupMap == NULL) {
        textBackupMap = (unsigned short *) malloc(TD_SCREEN_VHEIGHT * TD_SCREEN_VWIDTH*sizeof(unsigned short));
    }
    else if(textBuf->textMap == textBackupMap) {
        // Already in backup mode, return
        return;
    }
    // Currently displaying on screen, copy to backup
    // TODO: Optimize by checking how much is valid? 
    memcpy(textBackupMap, textBuf->textMap, 
           TD_SCREEN_VHEIGHT * TD_SCREEN_VWIDTH*sizeof(unsigned short));
    // Switch textMap to point to the backup
    textBuf->textMap = textBackupMap;
}

//   Restore in-memory buffer onto real display
void 
CTextDisp::RestoreDisplay()
{
    assert(textBuf!=NULL);

    // Setup the display registers to show text
    SetupDisplay();

    if(textBackupMap == NULL || textBuf->textMap != textBackupMap) {
        return;
    }
    // Switch textMap to point to the screen 
    textBuf->textMap = GBA_BASE_VRAM + 0x2000;

    // Copy from backup memory to screen
    // TODO: Optimize by checking how much is valid? 
    memcpy(textBuf->textMap, textBackupMap, 
           TD_SCREEN_VHEIGHT * TD_SCREEN_VWIDTH*sizeof(unsigned short));

    // Restore scroll to hardware registers
    SetScrollReg();
}

// Fill current text buffer from tile data provided, set xPos, yPos,
// and scrollPos based on args passed in
void CTextDisp::TextBufToScreen(const CTextBuf &buf, bool optimizeCopy)
{
    assert(textBuf!=NULL);
    textBuf->CopyFromTextBuf(buf, optimizeCopy);
    // Force restore of scroll to hardware registers
    lastScrollPos = -1;
    SetScrollReg();
}
void CTextDisp::ScreenToTextBuf(CTextBuf &buf, bool optimizeCopy)
{
    assert(textBuf!=NULL);
    buf.CopyFromTextBuf(*textBuf, optimizeCopy);
}

short CTextDisp::Init()
{
    // Create the textBuf to point directly to screen memory
    textBuf = new CTextBuf(GBA_BASE_VRAM + 0x2000, TD_SCREEN_VHEIGHT, 
                           TD_SCREEN_VWIDTH, 0, 2, 0);
    
    // Setup default palette and display font
    SetupDisplay();
  
    // Create a text context which is initially at the top of the
    // context stack.  The stack will own this context, so we don't have
    // to worry about deleting it.
    CTextContext *tContext = new CTextContext();
    DispContextStackSingleton.PushContext(tContext);
  
    Clear();
}

CTextDisp::CTextDisp(unsigned char mode) : 
    textBackupMap(NULL), textBuf(NULL)
{
    this->mode = mode;
    if (mode==TDM_LCD || mode==TDM_LCD_AND_CPORT)
        Init();
    ptd = this;
}
CTextDisp::~CTextDisp()
{
    if(textBackupMap) free(textBackupMap);
    if(textBuf) delete textBuf;
}

short CTextDisp::Print(const char *string)
{
    unsigned short i, temp16;

    if (mode==TDM_LCD || mode==TDM_LCD_AND_CPORT)
	{
            assert(textBuf!=NULL);
            textBuf->Print(string);
            SetScrollReg();
	}

    if (mode==TDM_CPORT || mode==TDM_LCD_AND_CPORT)
	{
            for (i=0; string[i]; i++) 
		{
                    if (string[i]==10)
			{
                            cport.SendChar(10);
                            cport.SendChar(13);
			}
                    else
                        cport.SendChar(string[i]);
		}
	}
}

short CTextDisp::Printf(const char *format, ...)
{
    char buf[128];
    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);
    Print(buf);
}

short CTextDisp::ScrollToTextEnd()
{
    if(!textBuf) {
        return(-1);
    }
    short ret = textBuf->ScrollToTextEnd();
    SetScrollReg();
    return(ret);
}

short CTextDisp::Scroll(short val)
{
    if(!textBuf) {
        return(-1);
    }
    short ret = textBuf->Scroll(val);
    SetScrollReg();
    return(ret);
}

void CTextDisp::SetScrollReg()
{
    // If writing directly to the screen update the hardware scroll
    // register
    //  if(textBuf->textMap != textBackupMap && textBuf->scrollPos!=lastScrollPos) {
    if(textBuf->textMap != textBackupMap) {
        GBA_REG_BG2VOFS = textBuf->scrollPos*8;
        lastScrollPos = textBuf->scrollPos;
    }
}

short CTextDisp::GetKey(unsigned short *key)
{
    if (key)
        *key = GBA_REG_P1;
    return(0);
}

short CTextDisp::Clear()
{
    if(!textBuf) {
        return(-1);
    }
    return(textBuf->Clear());
}

short CTextDisp::SetProperty(DispProp property, short val)
{
    switch (property)
	{
	case DP_REVERSE:
            if(textBuf) textBuf->reverse = val;
            break;
	case DP_LMARGIN:
            if(textBuf) textBuf->leftMargin = val;
	case DP_RMARGIN:
            // Add 2 to right margin because the video memory layout has
            // two extra undisplayable columns to the right...
            if(textBuf) textBuf->rightMargin = val+2;
	case DP_BMARGIN:
            if(textBuf) textBuf->bottomMargin = val;
            break;
	}
    return(0);
}

short CTextDisp::SetPosition(unsigned char x, unsigned char y)
{
//	if(x < 0 || y < 0)
//            return 0;
    if(textBuf) {
        textBuf->xPos = x%textBuf->lineWidth;
        textBuf->yPos = (y + textBuf->topMargin + textBuf->ScrollPos())%textBuf->lineNum;
    }
    return(0);
}

short CTextDisp::GetPosition(unsigned char *x, unsigned char *y)
{
    if (x && textBuf)
        *x = (unsigned char)textBuf->xPos;
    if (y && textBuf)
        *y = (unsigned char)textBuf->yPos - textBuf->topMargin - textBuf->ScrollPos();
    return(0);
}

short CTextDisp::SetBufferPosition(unsigned char x, unsigned char y)
{
//	if(x < 0 || y < 0)
//            return 0;
    if(textBuf) {
        textBuf->xPos = x%textBuf->lineWidth;
        textBuf->yPos = (y + textBuf->topMargin)%textBuf->lineNum;
    }
    return(0);
}

short CTextDisp::GetBufferPosition(unsigned char *x, unsigned char *y)
{
    if (x && textBuf)
        *x = (unsigned char)textBuf->xPos;
    if (y && textBuf)
        *y = (unsigned char)textBuf->yPos - textBuf->topMargin;
    return(0);
}

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//    indent-tabs-mode: nil
//   End:
