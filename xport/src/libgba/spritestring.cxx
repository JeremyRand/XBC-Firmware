#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "spritestring.h"
#include "textdisp.h"

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

CSpriteString::CSpriteString(CTileSet &tileSet, int length, 
			     int aIndex, int blankIndex, bool initShow,
			     unsigned short initX, unsigned short initY) :
  CSprite(tileSet, blankIndex, initShow, initX, initY),
  m_len(length), 
  m_aIndex(aIndex), m_blankIndex(blankIndex), m_zeroIndex(-1)
{
  //m_chars = new CSprite *[m_len-1];
  m_chars = (CSprite **)malloc((m_len-1)*sizeof(CSprite*));
  DBG(printf("Create spritestring, m_chars=0x%x\n", m_chars));
  for(int i=1;i<m_len;i++) {
    m_chars[i-1] = new CSprite(tileSet, blankIndex, initShow, 
			       initX+8*i, initY);
    DBG(printf("  sprite[%d] = 0x%x\n",i, m_chars[i-1]));
  }
  DBG(printf("done\n"));
}
CSpriteString::~CSpriteString() 
{
  for(int i=1;i<m_len;i++) {
    delete m_chars[i-1];
  }
  delete [] m_chars;
}

int 
CSpriteString::GetLetterTile(char c)
{
  c = toupper(c);
  if(m_zeroIndex>=0 && c>='0' && c<='9') {
    // This is a digit and we know how to deal with digits
    return(m_zeroIndex + c-'0');
  }
  else if(c<'A' || c>'Z') {
    return(m_blankIndex);
  }
  else {
    return(m_aIndex + c-'A');
  }
}

void 
CSpriteString::SetText(const char *text)
{
  unsigned int i, tlen = strlen(text);
  DBG(printf("CSpriteString::SetText(%s), len %d, m_len %d, m_chars=0x%x\n", 
	     text, tlen, m_len, m_chars));
  for(i=0; i<m_len; i++) {
    int tile;
    if(i<tlen) {
      tile = GetLetterTile(text[i]);
    }
    else {
      tile = m_blankIndex;
    }

    if(i==0) {
      // Special case leftmost arg because of inheritance from CSprite
      DBG(printf("Char 0: tile %d\n", tile));
      SetTileIndex(tile);
    }
    else {
      DBG(printf("Char %d: tile %d, sprite 0x%x\n",i,tile, m_chars[i-1]));
      m_chars[i-1]->SetTileIndex(tile);
    }
  }
  DBG(printf("CSpriteString::SetText, DONE\n"));
}

void 
CSpriteString::RegisterSpriteSet(CSpriteSet &spriteSet, int spriteIndex) 
{
  CSprite::RegisterSpriteSet(spriteSet, spriteIndex);
  for(int i=0;i<m_len-1;i++) {
    int cIndex = spriteSet.RegisterSprite(*(m_chars[i]), false);
  }
}

// Override SetPos to also move the other digits
void 
CSpriteString::SetPos(unsigned short x, unsigned short y) 
{
  CSprite::SetPos(x, y);

  for(int i=1;i<m_len;i++) {
    m_chars[i-1]->SetPos(x + 8*i, y);
  }
}

// Override SetVisible to also hide/show the other digits
void 
CSpriteString::SetVisible(bool show) 
{
  CSprite::SetVisible(show);
  for(int i=1;i<m_len;i++) {
    m_chars[i-1]->SetVisible(show);
  }
}
