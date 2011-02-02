#ifndef SPRITEBOX_H
#define SPRITEBOX_H

#include "sprites.h"

#define TILE_CENTER 3

// CSpriteBox acts like a single sprite with respect to setting the
// position, but it additionally aggregates four more sprites to be
// corners.    You can set the position of the corners in two ways:
// * SetSize(xSize,ySize) assumes cross is centered and sets left and 
//   right automatically from the center position
// * SetBBox(left, top, right, bottom) sets corners independent of center.

class CSpriteBox : public CSprite {
public:
  // Corners go:
  //   0  1
  //   2  3
  // x negative for 0, 2, positive for 1, 3
  // y negative for 0, 1, positive for 2, 3
  // xFlip true for 1, 3
  // yFlip true for 2, 3
  CSpriteBox(CTileSet &tileSet, int crossIndex, int cornerIndex, 
	     bool initShow=true, 
	     unsigned short centerX=0, unsigned short centerY=0,
	     unsigned short xSize=10, unsigned short ySize=10);
  ~CSpriteBox();

  // Returns true if the corner fits on screen, false otherwise, and 
  // puts the location of the corners into cornerX and cornerY
  bool CalculateCornerPos(int i, 
			  short &cornerX, 
			  short &cornerY) const;

  // Set x and y size of the box, assumes cross is centered
  void SetSize(unsigned short xSize, unsigned short ySize);
  // Sets corners independent of center
  void SetBBox(short left,  short top, 
	       short right, short bottom);

  bool KeepingCentered() const {
    return(m_keepCentered);
  }

  void GetBBox(short &left,  short &top, 
	       short &right, short &bottom) const;

  // Return a pointer to the given corner sprite.  cornerNum is in the
  // range 0 to 3.  See comment above for corner numbering order.
  CSprite *GetCornerSprite(int cornerNum) {
    if(cornerNum < 0 || cornerNum >= 4) {
      return(NULL);
    }
    else {
      return(m_corners[cornerNum]);
    }
  }

  // Override SetPos to apply to the center of the cross instead of
  // the top left corner and also move the corners
  virtual void SetPos(unsigned short centerX, unsigned short centerY);

  // Override GetPos to return the center of the cross instead of
  // the top left corner
  virtual void GetPos(unsigned short &x, unsigned short &y) const;

  // Override SetVisible to also hide/show the corners
  virtual void SetVisible(bool show);

  // Override RegisterSpriteSet to also register the corners
  virtual void RegisterSpriteSet(CSpriteSet &spriteSet, int spriteIndex);

protected:
  CSprite *m_corners[4];
  unsigned short m_xSize, m_ySize;
  short m_xLeft, m_yTop;
  // If keepCentered is true if using SetSize, false if using SetBBox
  bool m_keepCentered;
};

#endif
