#ifndef CBLOBTILES_H
#define CBLOBTILES_H

#include "display.h"
#include "palette.h"

// This context holds onto a reference to an associated model object,
// but does not own the storage for the model.  It is up to the user
// to make sure that the context is freed before its model, and that
// the model is eventually freed.
class CBlobTiles 
{
public:
  enum {
    IMAGE_CORNER_TILE = 2,
    BLANK_TILE = 9, 
    W_CORNER_TILE, 
    CYAN_CORNER_TILE,
    MAGENTA_CORNER_TILE,
    YELLOW_CORNER_TILE,
    ZERO_TILE=14,
    A_TILE=24,
    NUM_TILES=50 };
  
  CBlobTiles(CPalette::Type pType, unsigned short charBaseBlock);
  ~CBlobTiles();

  CTileSet &GetTileSet() { return(m_tiles); }

  int GetDigitTile(int digit) const;
  int GetLetterTile(char c) const;

  // This copies the tile set and palette into the GBA Display RAM.
  void WriteToGBA() const;

protected:
  // Sprite support
  CPalette m_palette;
  CTileSet m_tiles;
};

#endif
