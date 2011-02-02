#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include "spritenum.h"
#include "textdisp.h"

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

CSpriteNum::CSpriteNum(CTileSet &tileSet, int nDigits, 
		       int zeroIndex, int blankIndex, bool initShow,
		       unsigned short initX, unsigned short initY) :
  CSprite(tileSet, blankIndex, initShow, initX, initY),
  m_digits(0), m_nDigits(nDigits), 
  m_zeroIndex(zeroIndex), m_blankIndex(blankIndex)
{
  DBG(printf("Create spritenum\n"));
  if(m_nDigits > 1) {
    m_digits = new CSprite [m_nDigits-1](tileSet, blankIndex, initShow, 
			   initX, initY);
  }
  for(int i=1;i<m_nDigits;i++) {
    m_digits[i-1].SetPos(initX+8*i, initY);
  }
  DBG(printf("done\n"));
}
CSpriteNum::~CSpriteNum() 
{
  if(m_nDigits > 1) {
    delete [] m_digits;
  }
}

void 
CSpriteNum::SetDecimalValue(int value)
{
  char buffer[m_nDigits+4];
  sprintf(buffer, "%*d", m_nDigits, value);
  if(strlen(buffer)!=m_nDigits) {
    printf("Expected %d digits, got %d: value %d, buffer '%s'\n", 
	   m_nDigits, strlen(buffer), value, buffer);
  }

  for(int i=0; i<m_nDigits; i++) {
    int digit = buffer[i];
    int tile = m_blankIndex;
    
    if(digit >= '0' && digit <='9') {
      tile = m_zeroIndex + (digit - '0');
    }
    if(i==0) {
      // Special case leftmost arg because of inheritance from CSprite
      SetTileIndex(tile);
    }
    else {
      m_digits[i-1].SetTileIndex(tile);
    }
  }
}
void 
CSpriteNum::RegisterSpriteSet(CSpriteSet &spriteSet, int spriteIndex) 
{
  CSprite::RegisterSpriteSet(spriteSet, spriteIndex);
  for(int i=0;i<m_nDigits-1;i++) {
    int cIndex = spriteSet.RegisterSprite(m_digits[i], false);
  }
}

// Override SetPos to also move the other digits
void 
CSpriteNum::SetPos(unsigned short x, unsigned short y) 
{
  CSprite::SetPos(x, y);

  for(int i=1;i<m_nDigits;i++) {
    m_digits[i-1].SetPos(x + 8*i, y);
  }
}

// Override SetVisible to also hide/show the other digits
void 
CSpriteNum::SetVisible(bool show) 
{
  CSprite::SetVisible(show);
  for(int i=1;i<m_nDigits;i++) {
    m_digits[i-1].SetVisible(show);
  }
}
