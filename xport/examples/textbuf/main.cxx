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


#include <textdisp.h>
#include <display.h>
#include <dispcontext.h>
#include <textcontext.h>
#include <btnstate.h>
#include <regbits.h>

class CTestContext : public CTextContext {
public:
  CTestContext();
  virtual ~CTestContext() {}

  bool Focus();
  bool Process();	// Do a quantum of work and return

  void SetupDisplay();

public:
  CTextBuf *buf[6];
  unsigned short bsel;
  bool printing;
};

#define STATUS_BLOCK ((unsigned char)22)

CTestContext::CTestContext() :
  bsel(0), printing(true)
{
  for(int i=0; i<4; i++) {
    buf[i] = new CTextBuf(40, TD_SCREEN_WIDTH, i, i, i, i);
  }
  buf[4] = new CTextBuf(2, TD_SCREEN_VWIDTH, 0, 2);
  buf[4]->SetWrapEnable(false);
  buf[5] = new CTextBuf(CDisplay::ScreenBaseBlockAddr(STATUS_BLOCK),
			3, TD_SCREEN_VWIDTH, 0, 2);
  buf[5]->SetWrapEnable(false);
}

bool CTestContext::Focus()
{
  CTextContext::Focus();
  SetupDisplay();
  return(true);
}

void CTestContext::SetupDisplay()
{
	//Turn on Background 4 settings 
	//0 = 256x256, since our background is 240x160, this should be fine
	BFSET(GBA_REG_BG3CNT, backgroundSize, 0x00);
	//Lowest priority
	BFSET(GBA_REG_BG3CNT, priority, 0x3);

	// Setup tile block based on the font info
	CDispFont const *font=CDisplay::GetCurrDispFont();
	if(!font) {
		return;
	}
	BFSET(GBA_REG_BG3CNT, baseTileBlock, 0x0);
	BFSET(GBA_REG_BG3CNT, baseMapBlock, STATUS_BLOCK);
	//No mosaic
	BFSET(GBA_REG_BG3CNT, mosaic, 0x0);
	//256 color palette
	BFSET(GBA_REG_BG3CNT, palette, 0x1);
	BFSET(GBA_REG_DISPCNT, drawBG3, 1);
	GBA_REG_BG3VOFS = (unsigned short)-144;
}

bool 
CTestContext::Process() 
{
  static CBtnState bstate;
  static int pnum=0;
  bool show_once=false;
  bool optimizeCopy = true;
  bstate.PollKeys();

  if(bstate.KeyHit(CBtnState::A_BUTTON)) {
    show_once=true;
  } else if(bstate.KeyHit(CBtnState::B_BUTTON)) {
    printing=!printing;
    printf("Setting printing to %s\n", printing?"true":"false");
  } else if(bstate.KeyHit(CBtnState::LEFT_BUTTON)) {
    ptd->TextBufToScreen(*(buf[bsel]));
  } else if(bstate.KeyHit(CBtnState::RIGHT_BUTTON)) {
    ptd->Clear();
    printf("Starting at %d\n", pnum);
    printf("  Showing buf %d\n", bsel);
  } else if(bstate.KeyHit(CBtnState::L_BUTTON)) {
    ptd->ScreenToTextBuf(*(buf[bsel]), optimizeCopy);
    bsel = (bsel-1)&3;
    if(bsel > 1) {
      optimizeCopy=false;
    }
    else {
      optimizeCopy=true;
    }
    ptd->TextBufToScreen(*(buf[bsel]), optimizeCopy);
    printf("Now showing textbuf %d\n", (int)bsel);
  } else if(bstate.KeyHit(CBtnState::R_BUTTON)) {
    ptd->ScreenToTextBuf(*(buf[bsel]), optimizeCopy);

    bsel = (bsel+1)%5;
    if(bsel > 1 && bsel < 4) {
      optimizeCopy=false;
    }
    else {
      optimizeCopy=true;
    }

    ptd->TextBufToScreen(*(buf[bsel]), optimizeCopy);

    if(bsel==4) {
      buf[5]->CopyFromTextBuf(*buf[4]);
    }
    printf("Now showing textbuf %d\n", (int)bsel);
  } else if(bstate.KeyHit(CBtnState::START_BUTTON)) {
    printf("* Sel buf %d %s:\n", bsel, 
	   (buf[bsel]->wrapped)?"WRAP":"");
    printf("  yPos=%d, scroll=%d\n", 
	   buf[bsel]->yPos, buf[bsel]->scrollPos);
    printf("  top=%d, bottom=%d\n", 
	   buf[bsel]->TopLine(), buf[bsel]->BottomLine());
    printf("  valid=%d, size=%d\n", 
	   buf[bsel]->ValidLines(), buf[bsel]->lineNum);

    buf[4]->CopyFromTextBuf(*buf[5]);
    if(bsel==4) {
      ptd->TextBufToScreen(*(buf[4]));
    }

//     const CTextBuf *sbuf = ptd->GetScreenTextBuf();
//     printf("* Screen Buf %s:\n", 
// 	   (sbuf->wrapped)?"WRAP":"");
//     printf("  yPos=%d, scroll=%d\n", 
// 	   sbuf->yPos, sbuf->scrollPos);
//     printf("  top=%d, bottom=%d\n", 
// 	   sbuf->TopLine(), sbuf->BottomLine());
//     printf("  valid=%d, size=%d\n", 
// 	   sbuf->ValidLines(), sbuf->lineNum);
  } else if(bstate.KeyHit(CBtnState::SELECT_BUTTON)) {
    // Push text display onto top of stack 
    CTextContext::Push();
    return(true);
  }
  else {
    CTextContext::Process();
  }

  if(show_once || printing) {
    pnum++;
    for(int i=0; i<4; i++) { 
      if(i!=bsel) {
	buf[i]->Printf("B%d: Hello world! %d....|....|....|....|....|\n", 
		       i, pnum);
      }
    }
    buf[4]->Clear();
    buf[4]->Printf("Hello world! %d....|....|....|....|....|\n", 
		   pnum);
    printf("SB: Hello world! %d....|....|....|....|....|\n", pnum);
  }
  return(true);
}

CTextDisp td(TDM_LCD_AND_CPORT);

int main(void)
{
  CTestContext *tContext = new CTestContext();
  DispContextStackSingleton.PushContext(tContext);

  printf("ScreenBuf\n");
  for(int i=0; i<4; i++) { 
    tContext->buf[i]->Printf("TextBuf %d\n", i);
  }

  while(1) {
    DispContextStackSingleton.Process();
  }
}
