#ifndef SPRITENUM_H
#define SPRITENUM_H

#include "sprites.h"

// CSpriteNum acts like a single sprite with respect to setting the
// position, but it additionally aggregates other sprites to put
// together a multi-digit number.  You set the tile index for 0, then
// it assumes that the tile indices for the other digits are
// consecutive.  The position is the position of the top left corner
// of the first digit.

class CSpriteNum : public CSprite {
public:
  CSpriteNum(CTileSet &tileSet, int nDigits, 
	     int zeroIndex, int blankIndex, bool initShow=true,
	     unsigned short initX=0, unsigned short initY=0);
  ~CSpriteNum();

  // Set the value displayed by the sprites 
  void SetDecimalValue(int value);

  // Override SetPos to move the other digits as well
  virtual void SetPos(unsigned short x, unsigned short y);

  // Override SetVisible to also hide/show the other digits
  virtual void SetVisible(bool show);

  // Override RegisterSpriteSet to also register the other digits
  virtual void RegisterSpriteSet(CSpriteSet &spriteSet, int spriteIndex);

protected:
  CSprite *m_digits;
  int m_nDigits;
  int m_zeroIndex;
  int m_blankIndex;
};

#endif
