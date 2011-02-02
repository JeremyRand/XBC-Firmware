#include <stdlib.h>
#include "spritebox.h"
#include "textdisp.h"

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

// Corners go:
//   0  1
//   2  3
// x is left for 0, 2, right for 1, 3
// y is top for 0, 1, bottom for 2, 3
// xFlip true for 1, 3
// yFlip true for 2, 3
CSpriteBox::CSpriteBox(CTileSet &tileSet, int crossIndex, int cornerIndex, 
		       bool initShow, 
		       unsigned short centerX, unsigned short centerY, 
		       unsigned short xSize, unsigned short ySize) :
  CSprite(tileSet, crossIndex, initShow, centerX, centerY),
  m_xSize(xSize), m_ySize(ySize), 
  m_keepCentered(true)
{
  DBG(printf("Create spritebox\n"));
  for(int i=0;i<4;i++) {
    short cornerX, cornerY;
    bool show = initShow;
    bool xFlip = (i%2)?true:false;
    bool yFlip = (i<2)?false:true;
    
    DBG(printf("  Create corner %d\n", i));
    if(!CalculateCornerPos(i, cornerX, cornerY)) {
      show = false;
    }
    m_corners[i] = new CSprite(tileSet, cornerIndex, show, 
			       cornerX, cornerY);
    DBG(printf("  Flip corner %d, %d\n", xFlip, yFlip));
    m_corners[i]->SetFlip(xFlip, yFlip);
  }
  DBG(printf("done\n"));
}
CSpriteBox::~CSpriteBox() 
{
  for(int i=0; i<4; i++) {
    delete m_corners[i];
  }
}
void 
CSpriteBox::RegisterSpriteSet(CSpriteSet &spriteSet, int spriteIndex) 
{
  CSprite::RegisterSpriteSet(spriteSet, spriteIndex);
  for(int i=0; i<4; i++) {
    int cIndex = spriteSet.RegisterSprite(*(m_corners[i]), false);
  }
}

// Returns true if the corner fits on screen, false otherwise, and 
// puts the location of the corners into cornerX and cornerY
bool 
CSpriteBox::CalculateCornerPos(int i, 
			       short &cornerX, 
			       short &cornerY) const
{
  bool fits = true;

  if(m_keepCentered) {
    // Using set size, use center position and ignore left and right
    cornerX = m_xPos + (m_xSize/2)*((i%2)?1:-1);
    cornerY = m_yPos + (m_ySize/2)*((i<2)?-1:1);
  }
  else {
    // Using bounding box, use left and right and ignore center
    cornerX = m_xLeft + ((i%2)?m_xSize:0);
    cornerY = m_yTop + ((i<2)?0:m_ySize);
  }

  if(cornerX<0) {
    fits = false;
    cornerX = 0;
  } else if(cornerX >=GBA_DISP_NCOLS) {
    fits = false;
    cornerX = GBA_DISP_NCOLS-1;
  }

  if(cornerY<0) {
    fits = false;
    cornerY = 0;
  } else if(cornerY >=GBA_DISP_NROWS) {
    fits = false;
    cornerY = GBA_DISP_NROWS-1;
  }
  return(fits);
}

// Set x and y size of the box, assumes cross is centered
void 
CSpriteBox::SetSize(unsigned short xSize, unsigned short ySize) 
{
  if(m_xSize == xSize && m_ySize == ySize) {
    // Not changing anything, return
    return;
  }
  // Change size params
  m_xSize = xSize; 
  m_ySize = ySize;
  // Set keep centered
  m_keepCentered = true;

  // Set position of corners
  for(int i=0;i<4;i++) {
    short cornerX, cornerY;

    if(!CalculateCornerPos(i, cornerX, cornerY)) {
      m_corners[i]->Hide();
    }
    else {
      m_corners[i]->SetPos(cornerX, cornerY);
    }
  }
}

// Sets corners independent of center.  Subtract TILE_CENTER from x
// and y coords so that what we're setting is the position of the
// drawn corner
void 
CSpriteBox::SetBBox(short left,  short top, 
		    short right, short bottom)
{
  // Change left, top, and size params
  m_xLeft = left-TILE_CENTER;
  m_yTop = top-TILE_CENTER;
  m_xSize = right-left; 
  m_ySize = bottom-top;
  // Set keep centered false
  m_keepCentered = false;

  // Set position of corners
  for(int i=0;i<4;i++) {
    short cornerX, cornerY;

    if(!CalculateCornerPos(i, cornerX, cornerY)) {
      m_corners[i]->Hide();
    }
    else {
      m_corners[i]->SetPos(cornerX, cornerY);
    }
  }
}
    
void 
CSpriteBox::GetBBox(short &left,  short &top, 
		    short &right, short &bottom) const
{
  // Get position of corners 0 and 3
  CalculateCornerPos(0, left, top);
  CalculateCornerPos(3, right, bottom);
}

// Override SetPos to also move the corners if in keep centered mode
void 
CSpriteBox::SetPos(unsigned short centerX, unsigned short centerY) 
{
  // Set position of cross Subtract TILE_CENTER from x and y coords so
  // that what we're setting is the position of the center of the
  // drawn cross
  CSprite::SetPos(centerX-TILE_CENTER, centerY-TILE_CENTER);
  // Set position of corners if keeping centered
  if(m_keepCentered) {
    for(int i=0;i<4;i++) {
      short cornerX, cornerY;
      
      if(!CalculateCornerPos(i, cornerX, cornerY)) {
	m_corners[i]->Hide();
      }
      else {
	m_corners[i]->SetPos(cornerX, cornerY);
      }
    }
  }
}

// Override GetPos to return the center of the cross instead of
// the top left corner
void 
CSpriteBox::GetPos(unsigned short &x, unsigned short &y) const
{
  x = m_xPos + TILE_CENTER;
  y = m_yPos + TILE_CENTER;
}

// Override SetVisible to also hide/show the corners
void 
CSpriteBox::SetVisible(bool show) 
{
  // Set visibility of cross
  if(show) {
    // Making cross visible, if it's not visible yet we need to actually
    // check if each corner should be visible
    if(!m_show) {
      // We were hidden, set visibility of corners based on whether
      // or not they fit
      for(int i=0;i<4;i++) {
	short cornerX, cornerY;
	  
	if(CalculateCornerPos(i, cornerX, cornerY)) {
	  // Fits, show it
	  m_corners[i]->Show();
	}
      }
    }
  } else {
    // Making cross invisible, hide all the corners regardless of position
    for(int i=0;i<4;i++) {
      m_corners[i]->Hide();
    }
  }      
  CSprite::SetVisible(show);
}
