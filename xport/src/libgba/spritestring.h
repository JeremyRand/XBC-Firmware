#ifndef SPRITESTRING_H
#define SPRITESTRING_H

#include "sprites.h"

// CSpriteString acts like a single sprite with respect to setting the
// position, but it additionally aggregates other sprites to put
// together a multi-digit stringber.  You set the tile index for 0, then
// it assumes that the tile indices for the other digits are
// consecutive.  The position is the position of the top left corner
// of the first digit.

class CSpriteString : public CSprite {
public:
  CSpriteString(CTileSet &tileSet, int length, 
		int aIndex, int blankIndex, bool initShow=true,
		unsigned short initX=0, unsigned short initY=0);
  ~CSpriteString();

  // Set the value displayed by the sprites 
  void SetText(const char *text);

  // Get the tile associated with the letter provided
  int GetLetterTile(char c);

  // If you want to combine letters and numbers, you need to 
  // set the zero index.  
  void SetZeroIndex(int index) {
    m_zeroIndex = index;
  }

  // Override SetPos to move the other digits as well
  virtual void SetPos(unsigned short x, unsigned short y);

  // Override SetVisible to also hide/show the other digits
  virtual void SetVisible(bool show);

  // Override RegisterSpriteSet to also register the other digits
  virtual void RegisterSpriteSet(CSpriteSet &spriteSet, int spriteIndex);

protected:
  CSprite **m_chars;
  int m_len;
  int m_aIndex;
  int m_blankIndex;
  int m_zeroIndex;
};

#endif
