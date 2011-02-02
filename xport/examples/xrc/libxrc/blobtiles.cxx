#include <gba.h>
#include "palette.h"
#include "display.h"
#include "ctype.h"
#include "blobtiles.h"
#include "blob_sprites.h"

CBlobTiles::CBlobTiles(CPalette::Type pType, unsigned short charBaseBlock) :
  m_palette(pType, blob_spritesPalette, 0),
  m_tiles(charBaseBlock, NUM_TILES, blob_spritesData)
{
}

CBlobTiles::~CBlobTiles() 
{
}

int 
CBlobTiles::GetDigitTile(int digit) const
{
  if(digit<0 || digit>9) {
    return(BLANK_TILE);
  }
  else {
    return(ZERO_TILE + digit);
  }
}

int 
CBlobTiles::GetLetterTile(char c) const
{
  c = toupper(c);
  if(c<'A' || c>'Z') {
    return(BLANK_TILE);
  }
  else {
    return(A_TILE + c-'A');
  }
}

// This copies the tile set and palette into the GBA Display RAM.
void 
CBlobTiles::WriteToGBA() const
{
  m_palette.WriteToGBA();
  m_tiles.WriteToGBA();
}

