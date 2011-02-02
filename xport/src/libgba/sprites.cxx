#include <stdlib.h>
#include "intcont.h"
#include "sprites.h"
#include "display.h"
#include "trigtab.h"
#include "regbits.h"
#include "textdisp.h"

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

#define VBLANK_IRQ 0

CSprite::~CSprite() 
{
}

void 
CSprite::UpdateSpriteSet() 
{
  if(m_spriteSet!=NULL) {
    m_spriteSet->UpdateShadowMem(m_spriteIndex);
  }
}

bool
CSprite::WriteOAMAttribs(OAMAttribs *attribs) const
{
  /////////////////////////////////////////
  // Setup a0:           Width    Shift
  // 	yPos             8        0
  // 	rotScaleFlag	 1	  8 
  // 	dblOrDisableFlag 1	  9
  // 	mode		 2	  10
  // 	mosaicFlag	 1	  12
  // 	colorSize	 1	  13
  // 	shape		 2	  14

  // Set y position and clear other bits to zero
  attribs->a0 = m_yPos;
  if(m_show) {
    // When showing, use rotation if requested. If rotating set double
    // flag, if not rotating set disable to false.  Conveniently
    // enough we can just use m_useRot for both.
    attribs->a0 |= m_useRot << 8;
    attribs->a0 |= m_useRot << 9;
  }
  else {
    // Not showing, leave rotScaleFlag as 0 and set disable to 1
    attribs->a0 |= 1 << 9;
  }    
  // Leave mode, mosaicFlag, and shape as 0.  Set colorSize to 1
  attribs->a0 |= 1<< 13;

  /////////////////////////////////////////
  // Setup a1:             Width    Shift
  // 	xPos               9        0
  // 	rotScaleParamSel   5	    9 
  // 	size		   2	    14

  // Set x position and clear other bits to zero
  attribs->a1 = m_xPos ;
  if(m_useRot) {
    // Using rotation, this is the index of the rotation/scaling
    // parameter.  For now to simplify life we're not using scaling
    // and are setting the rotation value of the rot/scale sets to be
    // equal to their index so we can just write the value directly
    // here
    attribs->a1 |= m_rot << 9; 
  }
  else {
    // Not using rotation, this value is VH000 where H is horizontal flip
    // and V is vertical flip
    attribs->a1 |= (m_yFlip << 4 | m_xFlip << 3)<<9;
  }

  // For now, only support 8x8 pixel sprites by leaving size as 0
  // TODO: support > 8x8 pixel  sprites in the future

  /////////////////////////////////////////
  // Setup a2:             Width    Shift
  // 	tileNum		   10	    0
  // 	priority	   2	    10
  // 	palette		   4	    12

  // Tile num starts at 0 for CBB 4 and at 512 for CBB 5
  int cbb = m_tileSet.GetCharBaseBlock();
  if(cbb < 4) {
    // Bad tile, this isn't going to work!  What should we do????
    return(false);
  }
  // Set tileNum and clear other bits to zero.  Note that m_tileIndex
  // is multiplied by 2 for big palette size TODO: Make work for 16
  // color palettes
  attribs->a2 =  (cbb - 4)*512 + m_tileIndex*2;
  // Leave priority 0 for highest priority, TODO: make selectable?
  // Leave palette 0 since not used for 256 value palette

  return(true);
}
  

// bool
// CSprite::WriteOAMAttribs(volatile OAMAttribs &attribs) const
// {
//   attribs.a0.yPos = m_yPos;
//   if(m_show) {
//     // When showing, use rotation if requested. If rotating set double
//     // flag, if not rotating set disable to false.  Conveniently
//     // enough we can just use m_useRot for both.
//     attribs.a0.rotScaleFlag = m_useRot;
//     attribs.a0.dblOrDisableFlag = m_useRot;
//   }
//   else {
//     // Not showing, set rotScaleFlag to 0 and set disable to 1
//     attribs.a0.rotScaleFlag = 0;
//     attribs.a0.dblOrDisableFlag = 1;
//   }    
//   attribs.a0.mode = 0;
//   attribs.a0.mosaicFlag = 0;
//   attribs.a0.colorSize = 1;
//   attribs.a0.shape = 0;

//   attribs.a1.xPos = m_xPos ;
//   if(m_useRot) {
//     // Using rotation, this is the index of the rotation/scaling
//     // parameter.  For now to simplify life we're not using scaling
//     // and are setting the rotation value of the rot/scale sets to be
//     // equal to their index so we can just write the value directly
//     // here
//     attribs.a1.rotScaleParamSel = m_rot; 
//   }
//   else {
//     // Not using rotation, this value is VH000 where H is horizontal flip
//     // and V is vertical flip
//     attribs.a1.rotScaleParamSel = m_yFlip << 4 | m_xFlip << 3;
//   }
   
//   // For now, only support 8x8 pixel sprites.	
//   // TODO: support > 8x8 pixel  sprites in the future
//   attribs.a1.size = 0;

//   // Tile num starts at 0 for CBB 4 and at 512 for CBB 5
//   int cbb = m_tileSet.GetCharBaseBlock();
//   if(cbb < 4) {
//     // Bad tile, this isn't going to work!  What should we do????
//     return(false);
//   }
//   // m_tileIndex is multiplied by 2 for big palette size 
//   // TODO: Make work for 16 color palettes
//   attribs.a2.tileNum =  (cbb - 4)*512 + m_tileIndex*2;
//   attribs.a2.priority = 0;	// Highest priority, TODO: make selectable?
//   attribs.a2.palette = 0;	// Not used for 256 value palette

//   return(true);
// }

CSpriteSet::CSpriteSet(CInterruptCont *pIntCont):
  m_snum(0), m_intCont(pIntCont)
{
  // Set all sprites entries initially to NULL and initially disable
  // all sprites in shadow memory
  for(int i=0; i<MAX_SPRITE_NUM; i++) {
    m_sprites[i] = NULL;
    m_ownsSprites[i] = false;
    // Set dblOrDisableFlag
    m_shadowMem[i].a0 = 1 << 9;
  }

  // TODO: Enable vblank interrupt
}
CSpriteSet::~CSpriteSet()
{
  // TODO: Disable vblank interrupt
  for(int i=0; i<MAX_SPRITE_NUM; i++) {
    if(m_sprites[i] != NULL && m_ownsSprites[i]) {
      delete m_sprites[i];
    }
  }
}

short 
CSpriteSet::RegisterSprite(CSprite &sprite, bool giveOwnership)
{
  if(m_snum >= MAX_SPRITE_NUM) {
    printf("* MAX SPRITES EXCEEDED*\n");
    printf("  ignoring reg of sprite 0x%x\n", &sprite);
    return(-1);
  }
  // TODO: Search for first free after unregister implemented
  int index = m_snum++;
  m_sprites[index] = &sprite;
  m_ownsSprites[index] = giveOwnership;

  sprite.RegisterSpriteSet(*this, index);
  return(index);
}

void 
CSpriteSet::SetupDisplay()
{
  WriteAllToGBA();
  // Enable sprite display
  BFSET(GBA_REG_DISPCNT, drawSprites, 1);

  if (m_intCont) {
    // register interrupt
    m_intCont->Register(this, VBLANK_IRQ);
    
    // enable vblank interrupt
    m_intCont->Unmask(VBLANK_IRQ);
    BFSET(GBA_REG_DISPSTAT, enableVBInt, 1);
  }
}

void 
CSpriteSet::ReleaseDisplay()
{
  if(m_intCont) {
    // disable vblank interrupt
    BFSET(GBA_REG_DISPSTAT, enableVBInt, 0);
    m_intCont->Mask(VBLANK_IRQ);
  }

  // Disable sprite display
  BFSET(GBA_REG_DISPCNT, drawSprites, 0);
}

// Iterate over all registered sprites and 
void 
CSpriteSet::SetupShadowMem()
{
  // NOTE: This locking strategy is only useful for moderating access
  // between a single execution context and an interrupt context.
  // This only works while there is no true multitasking in the
  // system, and if this is only called from the execution context.

  // Grab the lock
  m_lockShadowMem=true;

  DBG(printf("Entering SetupShadowMem\n"));
  for(int i=0; i<m_snum; i++) {
    if(m_sprites[i] != NULL) {
      DBG(printf("  sprite[%d]: 0x%x->WriteOAMAttribs(0x%x)\n", 
		 i, m_sprites[i], &(m_shadowMem[i])));
      m_sprites[i]->WriteOAMAttribs(&(m_shadowMem[i]));
    }
    else {
      // Set dblOrDisableFlag
      m_shadowMem[i].a0 = 1 << 9;
    }
  }

  // Setup OAM rotation/scaling parameters.  For now to simplify life
  // we're not using scaling and are setting the rotation value of the
  // rot/scale sets to be equal to their index so we can just write
  // the rotaion value directly in the sprite rotation index selection
  // parameter

  // From GBATEK:
  // ** Location of Rotation/Scaling Parameters in OAM
  // Four 16bit parameters (PA,PB,PC,PD) are required to define a
  // complete group of Rotation/Scaling data. These are spread across
  // OAM as such:
  //   1st Group - PA=0700:0006, PB=0700:000E, PC=0700:0016, PD=0700:001E
  //   2nd Group - PA=0700:0026, PB=0700:002E, PC=0700:0036, PD=0700:003E
  //   etc.
  //
  // By using all blank space (128 x 16bit), up to 32 of these groups
  // (4 x 16bit each) can be defined in OAM.

  // ** Rotation/Scaling Parameters
  //   The following parameters are required for Rotation/Scaling
  //   Rotation Center X and Y Coordinates (x0,y0)
  //   Rotation Angle                      (alpha)
  //   Magnification X and Y Values        (xMag,yMag)
  //  
  // The display is rotated by 'alpha' degrees around the center.
  // The displayed picture is magnified by 'xMag' along x-Axis (Y=y0)
  // and 'yMag' along y-Axis (X=x0).
  //  
  // Calculating Rotation/Scaling Parameters A-D
  //   A = Cos (alpha) / xMag    ;distance moved in direction x, same line
  //   B = Sin (alpha) / xMag    ;distance moved in direction x, next line
  //   C = Sin (alpha) / yMag    ;distance moved in direction y, same line
  //   D = Cos (alpha) / yMag    ;distance moved in direction y, next line
  //
  // Calculating the position of a rotated/scaled dot
  // Using the following expressions,
  //   x0,y0    Rotation Center
  //   x1,y1    Old Position of a pixel (before rotation/scaling)
  //   x2,y2    New position of above pixel (after rotation scaling)
  //   A,B,C,D  BG2PA-BG2PD Parameters (as calculated above)
  //  
  // the following formula can be used to calculate x2,y2:
  //   x2 = A(x1-x0) + B(y1-y0) + x0
  //   y2 = C(x1-x0) + D(y1-y0) + y0
  
  // ** Bit packing
  //   Bit   Expl.
  //   0-7   Fractional portion (8 bits)
  //   8-26  Integer portion    (19 bits)
  //   27    Sign               (1 bit)
  //   28-31 Not used
  //
  // Because values are shifted left by eight, fractional portions may
  // be specified in steps of 1/256 pixels (this would be relevant
  // only if the screen is actually rotated or scaled). Normal signed
  // 32bit values may be written to above registers (the most
  // significant bits will be ignored and the value will be cut-down
  // to 28bits, but this is no actual problem because signed values
  // have set all MSBs to the same value).

  // There are 32 OAM rot parameters, and 512 entries in the integer
  // sin and cos tables, so shift over by 4 to get the index into the
  // tables to use for a given OAM entry.  For now we're using xMag =
  // yMag = 1 since we're not currently supporting scaling.  Therefore
  // A = D = costab[index] and B = C = sintab[index].    
  for(int i=0; i<32; i++) {
    int tab_index = i << 4;
    // PA/PD
    m_shadowMem[i*4 + 0].rotScaleParam = 
      m_shadowMem[i*4 + 3].rotScaleParam = (short)(costab[tab_index]);
    // PB/PC
    m_shadowMem[i*4 + 1].rotScaleParam = 
      m_shadowMem[i*4 + 2].rotScaleParam = (short)(sintab[tab_index]);
  }
  
  // Mark changed
  m_GBAUpdateNeeded = true;

  // Update min/max changed indices
  m_minChanged=0;
  m_maxChanged=m_snum-1;

  DBG(printf("Exiting SetupShadowMem\n"));

  // Unlock
  m_lockShadowMem=false;
}
void 
CSpriteSet::UpdateShadowMem(int i)
{
  // NOTE: This locking strategy is only useful for moderating access
  // between a single execution context and an interrupt context.
  // This only works while there is no true multitasking in the
  // system, and if this is only called from the execution context.

  // Grab the lock
  m_lockShadowMem=true;

  if(m_sprites[i] != NULL) {
    m_sprites[i]->WriteOAMAttribs(&(m_shadowMem[i]));
  }
  else {
    // Set dblOrDisableFlag
    m_shadowMem[i].a0 = 1 << 9;
  }
  
  // Mark changed
  m_GBAUpdateNeeded = true;

  // Update min/max changed indices
  if(i < m_minChanged) {
    m_minChanged = i;
  }
  if(i>m_maxChanged) {
    m_maxChanged = i;
  }

  // Unlock
  m_lockShadowMem=false;
}  

void 
CSpriteSet::WriteAllToGBA()
{
  WriteEntriesToGBA(0,MAX_SPRITE_NUM);
}
void 
CSpriteSet::WriteUpdatesToGBA()
{
  if(m_GBAUpdateNeeded) {
    WriteEntriesToGBA(m_minChanged,m_maxChanged - m_minChanged + 1);
  }
}

void
CSpriteSet::WriteEntriesToGBA(int startIndex, int num) 
{
  // Wait for vblank
  while(BFGET(GBA_REG_DISPSTAT, isVBlank));
  while(!(BFGET(GBA_REG_DISPSTAT, isVBlank)));

  _WriteEntriesToGBA(startIndex, num);
}

////////////////////////////////////////////////////////////////////////////
// These are the internal versions which assume that we're already in vblank
// and are suitable for being called on interrupt
bool 
CSpriteSet::_WriteUpdatesToGBA()
{
  if(m_GBAUpdateNeeded) {
    return(_WriteEntriesToGBA(m_minChanged,m_maxChanged - m_minChanged + 1));
  }
  else {
    return(true);
  }
}
bool
CSpriteSet::_WriteEntriesToGBA(int startIndex, int num) 
{
  // NOTE: This locking strategy is only useful for moderating access
  // between a single execution context and an interrupt context.
  // This only works while there is no true multitasking in the
  // system.  If this called from the interrupt context there is no
  // point in looping until the lock is unlocked, all we can do is
  // return false, and try again on next frame.

  // WARNING: Do not change this to memcpy, or any other method that
  // might do byte writes.  Experimentally, doing bytewise writes to
  // OAM memory results in every other byte being lost...

  // If already locked, bail out
  if(m_lockShadowMem) {
    return(false);
  }

  // Grab the lock
  m_lockShadowMem=true;

  // Copy from shadow memory to OAM memory starting at entry
  // startIndex, and copying num attribs structures
  unsigned short *srcAddr = (unsigned short *)(&(m_shadowMem[startIndex]));
  unsigned short *destAddr = GBA_BASE_OAM + (startIndex*sizeof(OAMAttribs))/2;
  int numWords = (num*sizeof(OAMAttribs))/2;

  for (int i= 0; i<numWords; i++, srcAddr++, destAddr++) {
    *destAddr = *srcAddr;
  } 

  // Clear update needed, reset change limits, and release lock
  m_GBAUpdateNeeded =false;
  m_minChanged = MAX_SPRITE_NUM;
  m_maxChanged = 0;
  m_lockShadowMem=false;
  return(true);
}


void 
CSpriteSet::Interrupt(unsigned char vector)
{
  _WriteUpdatesToGBA();
}
