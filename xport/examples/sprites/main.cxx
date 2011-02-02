#include <string.h>
#include "intcont.h"
#include "dispcontext.h"
#include "textdisp.h"
#include "textcontext.h"
#include "btnstate.h"
#include "palette.h"
#include "display.h"
#include "sprites.h"
#include "spritebox.h"
#include "spritenum.h"
#include <gba.h>

#include "test_sprites.c"
  
class CTestContext : public IDispContext
{
public:
  enum {
    ZERO_TILE = 10, 
    BLANK_TILE = 20,
    TILE_NUM = 21
  };
    
  CTestContext(CInterruptCont *pIntCont);
  virtual ~CTestContext() {}

  virtual bool Setup();	    // Newly created context, now top of stack
  virtual bool Blur();	  // Newer context on top of stack, lose focus
  virtual bool Focus();	  // Newer context(s) done, top of stack again
  virtual bool Shutdown();     // Context done, being popped off stack

  virtual bool Process();	// Do a quantum of work and return

protected:
  CInterruptCont *m_intCont;
  CPalette m_palette;
  CTileSet m_tiles;
  CSpriteBox *m_sboxes[4];
  CSpriteSet m_spriteSet;
  CSpriteNum m_xPos;
  CSpriteNum m_yPos;
};

CTestContext::CTestContext(CInterruptCont *pIntCont) :
  m_intCont(pIntCont),
  m_palette(CPalette::OBJ, test_spritesPalette, 0), 
  m_tiles(5, TILE_NUM, test_spritesData), 
  m_spriteSet(m_intCont), 
  m_xPos(m_tiles, 3, ZERO_TILE, BLANK_TILE, true, 
	 0, 0), 
  m_yPos(m_tiles, 3, ZERO_TILE, BLANK_TILE, true, 
	 0, 8)
{
  for(int i=0; i<4; i++) {
    m_sboxes[i] = new CSpriteBox(m_tiles, 2*i, (2*i)+1, true, 
				 80 + 20*i, 80);
    if(i>1) {
      m_sboxes[i]->SetBBox(75 + 20*i, 75, 
			   85 + 20*i, 85);
    }

    //m_sboxes[i]->SetRot(4*i);
    m_spriteSet.RegisterSprite(*(m_sboxes[i]), true);
  }
  m_spriteSet.RegisterSprite(m_xPos, false);
  m_spriteSet.RegisterSprite(m_yPos, false);
  
  m_spriteSet.SetupShadowMem();
}
bool 
CTestContext::Setup()
{
  return(Focus());
}

bool 
CTestContext::Blur()
{
  m_spriteSet.ReleaseDisplay();
  return(true);
}

#define DV_BUILD_COLOR(r, g, b)	(r&0x1f) | ((g&0x1f)<<5) | ((b&0x1f)<<10)

bool 
CTestContext::Focus()
{
  // set high-res graphics mode
  GBA_REG_DISPCNT = GBA_BG_MODE_3 | GBA_BG2_ENABLE<<8;

  // Set scaling at 1:1
  GBA_REG_BG2PA = 0x100;
  GBA_REG_BG2PD = 0x100;

  for (int i=0; i<GBA_DISP_NROWS; i++) {
    for (int j=0; j<GBA_DISP_NCOLS; j++) {
//       int n = (31-(i+j))%31;
//       GBA_BASE_VRAM[i*GBA_DISP_NCOLS + j] = DV_BUILD_COLOR(n,n,n);
      GBA_BASE_VRAM[i*GBA_DISP_NCOLS + j] = DV_BUILD_COLOR(0,0,0);
    }
  }

  // Setup sprite palette and tiles
  CDisplay::SetCurrPalette(&m_palette);
  m_tiles.WriteToGBA();

  // Setup the registers, interrupt, and OAM memory to display sprites
  m_spriteSet.SetupDisplay();

  return(true);
}

bool 
CTestContext::Shutdown()
{
  return(true);
}

bool 
CTestContext::Process()
{
  static CBtnState btnState;
  unsigned short x,y;
  bool changed = false;
  static int sel=0;

  m_sboxes[sel]->GetPos(x, y);
  
  btnState.PollKeys();

  if(btnState.CurrState() != btnState.PrevState()) {
    printf("Start %d: %d, %d\n", sel, x, y);
  }

  if(btnState.KeyHit(CBtnState::DOWN_BUTTON)) {
    y++;  changed=true;
    printf("  Down %d: %d, %d\n", sel, x, y);
  }
  if(btnState.KeyHit(CBtnState::UP_BUTTON)) {
    y--;  changed=true;
    printf("  Up %d: %d, %d\n", sel, x, y);
  }
  if(btnState.KeyHit(CBtnState::LEFT_BUTTON)) {
    x--;  changed=true;
    printf("  Left %d: %d, %d\n", sel, x, y);
  }
  if(btnState.KeyHit(CBtnState::RIGHT_BUTTON)) {
    x++;  changed=true;
    printf("  Right %d: %d, %d\n", sel, x, y);
  }
  if(btnState.KeyHit(CBtnState::A_BUTTON)) {
    // Set outgoing sprite box to have normal colored cross
    m_sboxes[sel]->SetTileIndex(2*sel);
    sel = (sel + 1)%4;
    // Set incoming sprite box to have white cross
    m_sboxes[sel]->SetTileIndex(8);
    printf("  Selected %d\n", sel);
  }
//   if(btnState.KeyHit(CBtnState::B_BUTTON)) m_test.GoToParentTest();
  if(btnState.KeyHit(CBtnState::SELECT_BUTTON)) {
    // Push textdisp on top of the stack
    CTextContext::Push();
    return(true);
  }

  if(changed) {
    printf("  Update %d: %d, %d\n", sel, x, y);
    m_sboxes[sel]->SetPos(x,y);
    m_xPos.SetDecimalValue(x);
    m_yPos.SetDecimalValue(y);

    if(sel>1) {
      m_sboxes[sel]->SetBBox(x-5, y-5, 
			     x+5, y+5);
    }
    short left, top, right, bottom;
    m_sboxes[sel]->GetBBox(left, top, right, bottom);
    //printf("   BBox = (%d, %d, %d, %d)\n", left, top, right, bottom);
  }
  return(true);
}

CTextDisp td(TDM_LCD_AND_CPORT);

int main()
{
  int i=0;
  CBtnState btnState;

  // Create a test context and push it onto the stack above this menu
  CInterruptCont intCont;
  CTestContext *tContext = new CTestContext(&intCont);
  DispContextStackSingleton.PushContext(tContext);

  printf("Sprite Test\n");
  while(1) {
    DispContextStackSingleton.Process();
  }
}
