#include <gba.h>
#include "string.h"
#include "vision.h"
#include "btnstate.h"

#include "modelcontext.h"
#include "textcontext.h"

#include "gba_image.h"
#include "xrc_lut.h"

#include "palette.h"
#include "display.h"
#include "sprites.h"
#include "spritebox.h"
#include "spritenum.h"
#include "spritestring.h"

#include <textdisp.h>

#include "blob_sprites.h"
#include "msimage.h"
#include "ctype.h"

#define BAR_H (GBA_NROWS - DV_GRAB_ROWS)/3 - 2
#define BAR_W DV_GRAB_COLS

#define SPRITE_DEFAULT_TILE CYAN_CORNER_TILE
#define SPRITE_HIGHLIGHT_TILE YELLOW_CORNER_TILE

#define IMAGE_CORNER_TILE 2
#define IMAGE_BOX_SIZE 10

#define LABEL_TEXT_YPOS (DV_GRAB_ROWS+1)
#define LABEL_TEXT_LEN 5
#define RGB_TEXT_YPOS (LABEL_TEXT_YPOS+8)
#define LABEL_X_START 0
#define HSV_NUM_X_START 48
#define HSV_NUM_X_OFFSET 10
#define HSV_NUM_X_SPACING ((DV_GRAB_COLS-HSV_NUM_X_START)/3)

#define MR_SPRITE_X  GBA_DISP_NCOLS-14
#define MR_SPRITE_Y  6

// #define ENABLE_AUTOTRAIN

// #define DEBUG

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

/////////////////////////////////////////////////////////////////////////
// Model Context
CModelContext::CModelContext(CVision &vision, 
			     unsigned short modelIndex,
			     CColorModelHSV &model, 
			     const CVisionDispOptions &options) :
  CVisionContext(vision, options),
  m_model(model),  m_hp(MSI_XSIZE, MSI_YSIZE, MSI_XCENTER, 
			MSI_START_H, MSI_MIN_S, MSI_MIN_V),
  m_modelIndex(modelIndex), 
  m_modSel(0), m_dispSel(0), m_modelChanged(0)
{
  DBG(printf("Creating CModelContext\n"));
  DBG(m_options.PrintSettings());

  DBG(printf(" Creating Palette\n"));
  m_spritePalette = new CPalette(CPalette::OBJ, blob_spritesPalette, 0);
  DBG(printf(" Creating Tiles\n"));
  m_tiles = new CTileSet(5, NUM_TILES, blob_spritesData);
  DBG(printf(" Creating Sprites\n"));
  m_spriteSet = new CSpriteSet(vision.GetInterruptCont());

  // Set up sprite box with white corners and no center Initially put
  // in top right corner so mod letters appear there if enabled

  // Set visible to true to reenable auto-training sprites
  m_imgSprites = new CSpriteBox(*m_tiles, BLANK_TILE, 
				IMAGE_CORNER_TILE, false, 
				DV_GRAB_COLS/2, DV_GRAB_ROWS/2, 
				IMAGE_BOX_SIZE, IMAGE_BOX_SIZE);
  m_spriteSet->RegisterSprite(*m_imgSprites, true);
  m_msiSprites = new CSpriteBox(*m_tiles, BLANK_TILE, 
				SPRITE_DEFAULT_TILE, true, 
				MR_SPRITE_X, MR_SPRITE_Y,
				10, 10);
  m_spriteSet->RegisterSprite(*m_msiSprites, true);

  // Setup label sprites
  m_modeLabelSprites = new CSpriteString(*m_tiles, 5, A_TILE, BLANK_TILE, 
					 true, 0, LABEL_TEXT_YPOS);
  m_spriteSet->RegisterSprite(*m_modeLabelSprites, true);

  // Setup HSV sprites
  for(int i=0; i<3; i++) {
    char hsv[]="HSV";
    char rgb[]="RGB";

    m_hsvLabelSprites[i] = new CSprite(*m_tiles, 
				       GetLetterTile(hsv[i]), 
				       true, 
				       HSV_NUM_X_START + 
				       HSV_NUM_X_SPACING*i, 
				       LABEL_TEXT_YPOS);
    m_spriteSet->RegisterSprite(*m_hsvLabelSprites[i], true);
    m_hsvMinSprites[i] = new CSpriteNum(*m_tiles, 3, 
					ZERO_TILE, BLANK_TILE, true,
					HSV_NUM_X_START + 
					HSV_NUM_X_SPACING*i + 
					HSV_NUM_X_OFFSET,
					LABEL_TEXT_YPOS);
    m_spriteSet->RegisterSprite(*m_hsvMinSprites[i], true);
    m_hsvMaxSprites[i] = new CSpriteNum(*m_tiles, 3, 
					ZERO_TILE, BLANK_TILE, true,
					HSV_NUM_X_START + 
					HSV_NUM_X_SPACING*i + 
					HSV_NUM_X_OFFSET,
					LABEL_TEXT_YPOS + 8);
    m_spriteSet->RegisterSprite(*m_hsvMaxSprites[i], true);

    m_rgbLabelSprites[i] = new CSprite(*m_tiles, 
				       GetLetterTile(rgb[i]), 
				       false,
				       HSV_NUM_X_START + 
				       HSV_NUM_X_SPACING*i, 
				       RGB_TEXT_YPOS);
    m_spriteSet->RegisterSprite(*m_rgbLabelSprites[i], true);
  }

  m_spriteSet->SetupShadowMem();

  // Modify options to only show the channel we're interested in
  m_options.DisableAll();
  m_options.SetEnable(modelIndex, true);

  m_modeLabelSprites->SetText("MODEL");

  // Update model sprites to match initial model
  UpdateSprites();
}

CModelContext::~CModelContext() 
{
}

int 
CModelContext::GetLetterTile(char c)
{
  c = toupper(c);
  if(c<'A' || c>'Z') {
    return(BLANK_TILE);
  }
  else {
    return(A_TILE + c-'A');
  }
}

void 
CModelContext::WriteMsiToScreen()
{
  // Copy msimage into the rightmost area of the screen
  for(int y=0; y<MSI_YSIZE; y++) {
    for(int x=0; x<MSI_XSIZE; x++) {
      GbaScreenImage.pixel(x+DV_GRAB_COLS, y) = msimage[y][x];
    }
    // Clear anything extra to the right
    for(int x=DV_GRAB_COLS + MSI_XSIZE; x<GBA_NCOLS; x++) {
      GbaScreenImage.pixel(x, y) = 0;
    }
  }
  // Clear anything extra below
  for(int y=MSI_YSIZE; y<GBA_NROWS; y++) {
    for(int x=DV_GRAB_COLS; x<GBA_NCOLS; x++) {
      GbaScreenImage.pixel(x, y) = 0;
    }
  }
}

void 
CModelContext::HistogramImageBBox()
{
  // Get bounding box of imgSprites
  short left, top, right, bottom;
  m_imgSprites->GetBBox(left, top, right, bottom);

  // Histogram pixels in box, generate model based on maxima/minima
  // Re-enable this if S+V get debugged ok
  //   CColorModelHSV nmodel(MAX_H, 0, MAX_S, MAX_S, MAX_V, MAX_V);
  CColorModelHSV nmodel(MAX_H, 0, m_hp.sMin, MAX_S, m_hp.vMin, MAX_V);
  for(int y=top; y<=bottom; y++) {
    for(int x=left; x<=right; x++) {
      // Get hsv at image coord (x, y)
      unsigned char r, g, b;
      unsigned short h, h2;
      unsigned char s, s2, v, v2;
      GbaScreenImage.get_rgb(x, y, r, g, b);
      SlowRgbToHsvOV6620_555(r, g, b,
 			     h2, s2, v2);
      RgbToHsvOV6620(r<<3, g<<3, b<<3,
		     h, s, v);
      printf("At Image (%d, %d) RGB=(%d, %d, %d)\n  HSV=(%d, %d, %d)\n", 
	     (int)x, (int)y, 
	     (int)r, (int)g, (int)b, 
	     (int)h, (int)s, (int)v);
      printf("  Alt HSV=(%d, %d, %d)\n", 
	     (int)h2, (int)s2, (int)v2);

      // Accumulate maxima/minima
      if(h<nmodel.hMin) nmodel.hMin = h;
      if(h>nmodel.hMax) nmodel.hMax = h;
// Re-enable these if S+V get debugged ok
//       if(s<nmodel.sMin) nmodel.sMin = s;
//       if(v<nmodel.vMin) nmodel.vMin = v;

      // Get the HSV plane x, y coords for this HSV value
      int mY, mlX, mrX;
      m_hp.HSVToSegment(h, s, v, mY, mlX, mrX);
      printf("   Msi coords %d: %d -> %d\n", mY, mlX, mrX);

      // Draw a line for it
      GbaScreenImage.draw_fillrect(mlX+DV_GRAB_COLS, mY, 
				   mrX+DV_GRAB_COLS, mY, 
				   Image::make_rgb(31, 31, 31));
    }
  }
  printf("New model is:\n");
  nmodel.Print();
  
  if(nmodel.hMax - nmodel.hMin > MAX_HRANGE) {
    printf("Hrange too large, aborting\n");
    return;
  }
  
  // Set m_model to new values, set m_modelChanged to
  // CVision::MODEL_PARTS to indicate that it's all going to have to
  // change now
  m_model = nmodel;
  m_modelChanged = CVision::MODEL_PARTS;

  // Update the display
  UpdateSprites();
  UpdateStatusArea();

  // Change to processed mode
  m_options.mode = RM_PROCESSED;
  SetDispOptions(m_options);
}

void 
CModelContext::UpdateShowMode() 
{
  m_dispSel++;
  printf("Setting show mode to %d\n", m_dispSel);

  SetupModeSprites();
  
  if(m_dispSel != SHOW_TEXTBUF) {
    // Drawing bars or text display
    UpdateStatusArea();
  }
  else {
    // Push text display
    m_options.PrintSettings();

    printf("Render color: %d, enable %d\n", 
	   (int)m_vision.GetRenderColor(m_modelIndex), 
	   (int)m_vision.GetRenderEnable(m_modelIndex));

    // Print current model info
    printf("\nModel %d:\n", m_modelIndex);
    m_model.Print();

    // Push text display onto top of stack 
    CTextContext::Push();
    // Make sure select mode is 0 when we come back
    m_dispSel = 0;
  }
}

void 
CModelContext::UpdateModMode(int dir) 
{
  if(m_modSel == MOD_MOVE) {
    m_modSel = MOD_RESIZE;
  }
  else {
    m_modSel = MOD_MOVE;
  }

  DBG(printf("Setting mod mode to %d\n", m_modSel));
  SetupModeSprites();
}

void 
CModelContext::SetupModeSprites() 
{
  bool showModel = ((m_dispSel == SHOW_BOTH) || 
		    (m_dispSel == SHOW_MODEL_TEXT));
  bool showPixel =  (m_dispSel == SHOW_PIXEL);
  
  // Set visibility of labels an numbers based on show mode. 
  SetSpriteArrVisibility(m_rgbLabelSprites, 3, showPixel);
  SetHsvSpriteVisibility(showModel || showPixel);

  m_modeLabelSprites->SetVisible(showModel || showPixel);
  if(showModel) {
    m_modeLabelSprites->SetText("MODEL");
  }
  else if(showPixel) {
    m_modeLabelSprites->SetText("PIXEL");
  }

  // Setup selection sprite tiles.  Initially clear everything, then
  // add as appropriate
  SetCornerHighlight(m_msiSprites, 0, false);
  SetCornerHighlight(m_msiSprites, 3, false);
  m_msiSprites->SetTileIndex(BLANK_TILE);

  switch(m_modSel) {
  case MOD_TL_BR: 
    if(m_LR == 0) {
      SetCornerHighlight(m_msiSprites, 0, true);
    }
    else {
      SetCornerHighlight(m_msiSprites, 3, true);
    }
    break;
  case MOD_MOVE:
    m_msiSprites->SetTileIndex(GetLetterTile('M'));
    break;
  case MOD_RESIZE:
    m_msiSprites->SetTileIndex(GetLetterTile('R'));
    break;
  }
}

void 
CModelContext::SetHsvSpriteVisibility(bool visible)
{
  for(int i=0; i<3; i++) {
    m_hsvLabelSprites[i]->SetVisible(visible);
    m_hsvMinSprites[i]->SetVisible(visible);
    m_hsvMaxSprites[i]->SetVisible(visible);
  }
}

void
CModelContext::SetSpriteArrVisibility(CSprite *sa[], int n, bool visible)
{
  for(int i=0; i<n; i++) {
    sa[i]->SetVisible(visible);
  }
}

void 
CModelContext::SetCornerHighlight(CSpriteBox *sbox, int corner, bool enable)
{
  if(enable) {
    sbox->GetCornerSprite(corner)->SetTileIndex(SPRITE_HIGHLIGHT_TILE);
  }
  else {
    sbox->GetCornerSprite(corner)->SetTileIndex(SPRITE_DEFAULT_TILE);
  }
}
void 
CModelContext::SetDispOptions(const CVisionDispOptions &options)
{
  CVisionContext::SetDispOptions(options);
  SetScreenScale();
  UpdateStatusArea();
}

bool 
CModelContext::Blur()
{
  DBG(printf("Blurring CModelContext\n")); 
  CVisionContext::Blur();
  m_spriteSet->ReleaseDisplay();
  DBG(printf("done\n"));
  return(true);
}

bool 
CModelContext::Focus()
{
  static bool showUsage=true;

  DBG(printf("Focusing CModelContext\n"));
  DBG(m_options.PrintSettings());

  // Setup sprite palette and tiles
  m_spritePalette->WriteToGBA();
  m_tiles->WriteToGBA();

  // Setup vision context for displaying video
  CVisionContext::Focus();

  // Setup the registers, interrupt, and OAM memory to display sprites
  m_spriteSet->SetupDisplay();

  // Copy msimage onto the screen
  WriteMsiToScreen();

  // Set the mode sprites
  SetupModeSprites();

  // Setup the scale and lower image are to show the image and bars
  SetScreenScale();
  UpdateSprites();
  UpdateStatusArea();

  // Ignore whatever buttons are currently being pressed
  m_modelContextIgnoreBtns=true;

  if(showUsage) {
    Usage();
    showUsage=false;
    // Push text display onto top of stack 
    CTextContext::Push();
    return(true);
  }

  return(true);
}

// Set the scale of the screen to include the image at the top, and the 
// HSV bars at the bottom
void 
CModelContext::SetScreenScale()
{
  // Set texture scaling to show full picture
  GBA_REG_BG2PA = 1<<8;
  GBA_REG_BG2PD = 1<<8;
}

// These are helper functions to convert scale between image coords
// and sprite coords.  Sprite coords map directly to GBA screen
// pixels.  The image applies a scaling to the X direction to stretch
// DV_GRAB_COLS to cover the physical width of the screen.
short 
CModelContext::ImgToSpriteX(int pos) 
{
  return((short)((pos * GBA_DISP_NCOLS)/DV_GRAB_COLS));
}
short 
CModelContext::ImgToSpriteY(int pos) {
  return((short)(pos));
}

short
CModelContext::SpriteToImgX(int pos) 
{
  return((short)((pos * DV_GRAB_COLS)/GBA_DISP_NCOLS));
}
short
CModelContext::SpriteToImgY(int pos) {
  return((short)(pos));
}

void DrawHBar(short hmin, short hmax, Image &mimg,  
	      int rmin, int rmax)
{
  static int hscale = ((BAR_W - 2) << 8) / MAX_H;
  int hxmin = ((hmin * hscale)>>8)+1;
  int hxmax = ((hmax * hscale)>>8)+1;
  DBG(printf("Bar H (%d->%d): %d, %d, %d, %d\n", 
	     hmin, hmax,
	     hxmin, rmin+1, 
	     hxmax, rmax-3));
  for(int hx=hxmin; hx<=hxmax; hx++) {
    unsigned short hval = ((hx - 1)<<8)/hscale;
    unsigned char r, g, b;
    HsvToRgbOV6620(hval, MAX_S, MAX_V, r, g, b);
    DBG(printf("HsvToRgbOV6620(%d, %d, %d) -> (%d, %d, %d)\n", 
	       hval, MAX_S, MAX_V, r, g, b));
    mimg.draw_fillrect(hx, rmin+1, 
		       hx, rmax-3,
		       Image::make_rgb(r>>3, g>>3, b>>3));
  }
}

void 
CModelContext::UpdateStatusArea()
{
  int i, row;
  int rows[DV_MODELS];
  Image mimg(GBA_DISP_NROWS, GBA_DISP_NCOLS);

  // Clear the bar display area
  unsigned short bgColor = Image::make_rgb(0, 0, 0);
  unsigned short borderColor = Image::make_rgb(m_modelChanged, 0, 0);

  if(m_dispSel == SHOW_MODEL_TEXT) {
    // If not showing bars color background based on m_modelChanged
    bgColor = borderColor;
  }

  mimg.draw_fillrect(0, DV_GRAB_ROWS+1, DV_GRAB_COLS, GBA_NROWS-1, 
		     bgColor);

  // Copy msimage into the rightmost corner
  for(int y=DV_GRAB_ROWS; y<MSI_YSIZE; y++) {
    for(int x=0; x<MSI_XSIZE; x++) {
      mimg.pixel(x+DV_GRAB_COLS, y) = msimage[y][x];
    }
  }

  if(m_dispSel != SHOW_MODEL_TEXT) {
    // Draw open rectangles for bars
    for(i=0, row=DV_GRAB_ROWS + 1; i<DV_MODELS; i++, row += BAR_H+2) {
      rows[i] = row;
      DBG(printf("Box %d: %d, %d, %d, %d (borderColor 0x%x)\n", i, 
		 0, row, DV_GRAB_COLS-2, row + BAR_H, borderColor));
      mimg.draw_box(0, row, DV_GRAB_COLS-2, row + BAR_H, borderColor);
    }
    
    // These scales are << 8
    static int sscale = ((BAR_W - 2) << 8) / MAX_S;
    static int vscale = ((BAR_W - 2) << 8) / MAX_V;
    
    // Draw H bar
    if(m_model.hMin > m_model.hMax) {
      // Draw as two pieces
      DrawHBar(m_model.hMin, MAX_H, mimg, rows[0], rows[1]);
      DrawHBar(0, m_model.hMax, mimg, rows[0], rows[1]);
    }
    else {
      // Draw as one piece
      DrawHBar(m_model.hMin, m_model.hMax, mimg, rows[0], rows[1]);
    }

    // Draw S bar
    int sxmin = ((m_model.sMin * sscale)>>8)+1;
    int sxmax = ((m_model.sMax * sscale)>>8)+1;
    DBG(printf("Bar S (%d->%d): %d, %d, %d, %d\n", 
	       m_model.sMin, m_model.sMax,
	       sxmin, rows[1]+1, 
	       sxmax, rows[2]-3));
    mimg.draw_fillrect(sxmin, rows[1]+1, 
		       sxmax, rows[2]-3,
		       Image::make_rgb(0, 31, 0));
    // Draw V bar
    int vxmin = ((m_model.vMin * vscale)>>8)+1;
    int vxmax = ((m_model.vMax * vscale)>>8)+1;
    DBG(printf("Bar V (%d->%d): %d, %d, %d, %d\n", 
	       m_model.vMin, m_model.vMax,
	       vxmin, rows[2]+1, 
	       vxmax, rows[2]+BAR_H-1));
    mimg.draw_fillrect(vxmin, rows[2]+1, 
		       vxmax, rows[2]+BAR_H-1,
		       Image::make_rgb(0, 0, 31));
  }
  DBG(printf("Drawing\n"));
  mimg.write_to_gba_screen(0, DV_GRAB_ROWS);
}

void 
CModelContext::Usage()
{
  ptd->Clear();
  printf("Model Editing\n\n");
  printf("Bottom section: \n");
  printf(" Bars show HSV Ranges\n");
  printf(" Text shows HSV Min/Max\n");
  printf("\n");
  printf("Right section: \n");
  printf("  S, V max in center\n");
  printf("  S decreases left of center\n");
  printf("  V decreases right of center\n");
  printf("  H increases towards bottom\n");
  printf("\n");
  printf("Controls:\n");
  printf(" L/R:   Select corner to move\n");
  printf(" A:     Toggle video mode\n");
  printf(" B:     Exit context\n");
  printf(" START: Cycle corner, M, R\n");
#ifdef ENABLE_AUTOTRAIN
  printf(" START: Sample center colors\n");
#endif
  printf(" SEL:   Cycle bars/text disp\n");
  printf("\n");
  printf(" Press B to Start");
}

#define SPEED_DIV 5

void 
CModelContext::KeyToDeltaModel(int key, int speed, CColorModelHSV &deltaModel)
{
  // Setup helpers for mode
  bool mTL = (m_modSel == MOD_TL_BR && m_LR == 0);
  bool mM = (m_modSel == MOD_MOVE);
  bool mR = (m_modSel == MOD_RESIZE); 
  bool mBR = (m_modSel == MOD_TL_BR  && m_LR == 1);

  // Setup helpers for key
  bool kU = (key == CBtnState::UP_BUTTON);
  bool kD = (key == CBtnState::DOWN_BUTTON);
  bool kL = (key == CBtnState::LEFT_BUTTON);
  bool kR = (key == CBtnState::RIGHT_BUTTON);

  // Reset everything
  deltaModel.hMin=deltaModel.hMax=deltaModel.sMin=deltaModel.vMin=0;
  
  if(kD && !mBR) 
    deltaModel.hMin = speed;
  else if(kU && !mBR) 
    deltaModel.hMin = -speed;

  if(((mBR || mM) && kD) || (mR && kU)) 
    deltaModel.hMax = speed;
  else if(((mBR || mM) && kU) || (mR && kD))
    deltaModel.hMax = -speed;
  
  if(((mTL || mM) && kR) || (mR && kL)) 
    deltaModel.sMin = speed;
  else if(((mTL || mM) && kL) || (mR && kR))
    deltaModel.sMin = -speed;

  if(kL && !mTL) 
    deltaModel.vMin = speed;
  else if(kR && !mTL) 
    deltaModel.vMin = -speed;
}

//#define USE_DBG(x) x
#define USE_DBG(x) 

void 
CModelContext::ApplyDeltaModel(CColorModelHSV &m, const CColorModelHSV &d)
{
  CColorModelHSV nm;
  bool trunc=false;

  // Add delta
  nm.hMin = m_hp.WrapHue(m.hMin, d.hMin, &trunc);
  nm.hMax = m_hp.WrapHue(m.hMax, d.hMax, &trunc);
  nm.sMin = m.sMin + d.sMin;
  nm.vMin = m.vMin + d.vMin;  

  if(trunc && m_modSel == MOD_MOVE) {
    USE_DBG(printf("Hue truncated in move mode, aborting\n"));
    return;
  }
  USE_DBG(printf("Applying delta model, before limits\nOrig model:\n"));
  USE_DBG(m.Print());
  USE_DBG(printf("Delta:\n"));
  USE_DBG(d.Print());
  USE_DBG(printf("New:\n"));
  USE_DBG(nm.Print());

  // Apply limits
  int odist = CColorModelHSV::HueDist(m.hMin, m.hMax);
  int ndist = CColorModelHSV::HueDist(nm.hMin, nm.hMax);
  USE_DBG(printf("Hue dist: before %d, after %d\n", odist, ndist));

  if(ndist>MAX_HRANGE) {
    // Check if we were approaching it, if so just move it to
    // equal nhmin
    if(odist < abs(d.hMin + d.hMax)) {
      USE_DBG(printf("----Hue would move past, not changing\n"));
      return;
    }
    else {
      USE_DBG(printf("----Hue would move too far apart\n"));
      if(m_modSel == MOD_RESIZE) {
	USE_DBG(printf("Hit limit in resize mode, aborting\n"));
	return;
      }
      if(d.hMax == 0) {
	// hMin must be moving away, move hMax same amount as hMin
	nm.hMax = m_hp.WrapHue(m.hMax, d.hMin);
	USE_DBG(printf("-----Pulled hMax: %d\n", nm.hMax));
      }
      else {
	// hMax must be moving away, move hMin same amount as hMax
	nm.hMin = m_hp.WrapHue(m.hMin, d.hMax);
	USE_DBG(printf("-----Pulled hMin: %d\n", nm.hMin));
      }
    }
  }
  
  // Deal with sMin
  if(nm.sMin<m_hp.sMin) {
    if(m_modSel == MOD_MOVE) {
      // In move mode, just stop here and don't squash the range
      return;
    }
    nm.sMin = m_hp.sMin;
  }
  else if(nm.sMin>MAX_S) {
    if(m_modSel == MOD_MOVE) {
      // In move mode, just stop here and don't squash the range
      return;
    }
    nm.sMin=MAX_S;
  }

  // Deal with vMin
  if(nm.vMin<m_hp.vMin) {
    if(m_modSel == MOD_MOVE) {
      // In move mode, just stop here and don't squash the range
      return;
    }
    nm.vMin = m_hp.vMin;
  }
  else if(nm.vMin>MAX_V) {
    if(m_modSel == MOD_MOVE) {
      // In move mode, just stop here and don't squash the range
      return;
    }
    nm.vMin=MAX_V;
  }
  
  // Modify model passed in
  m.hMin = nm.hMin;
  m.hMax = nm.hMax;
  m.sMin = nm.sMin;
  m.vMin = nm.vMin;

  USE_DBG(printf("Applying delta model, after limits\nFinal model:\n"));
  USE_DBG(m.Print());
}

  
bool 
CModelContext::Process()
{
  static CBtnState btnState;
  static unsigned short chgCount=1;
  static unsigned short modelPhase=0;

  bool doRedraw = false;
  bool valueChanged = false;
  
  // Setup speed to move things this cycle
  int speed = chgCount/SPEED_DIV;
  int key=0;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  if(m_modelContextIgnoreBtns) {
    btnState.PollKeys();
    m_modelContextIgnoreBtns=false;
  }

  btnState.PollKeys();

  if(btnState.KeyHit(CBtnState::A_BUTTON)) {
    CycleRenderMode();
  } if (btnState.KeyHit(CBtnState::R_BUTTON)) {
    m_LR=1;
    m_modSel = MOD_TL_BR;
    SetupModeSprites();
    doRedraw = true;
  }
  else if (btnState.KeyHit(CBtnState::L_BUTTON)) {
    m_LR=0;
    m_modSel = MOD_TL_BR;
    SetupModeSprites();
    doRedraw = true;
  }
  else if (!(GBA_REG_P1&GBA_KEY_LFT)) {
    key = CBtnState::LEFT_BUTTON;
    valueChanged=true;
  }
  else if (!(GBA_REG_P1&GBA_KEY_RT)) {
    key = CBtnState::RIGHT_BUTTON;
    valueChanged=true;
  }
  else if (!(GBA_REG_P1&GBA_KEY_UP)) {
    key = CBtnState::UP_BUTTON;
    valueChanged=true;
  }
  else if (!(GBA_REG_P1&GBA_KEY_DWN)) {
    key = CBtnState::DOWN_BUTTON;
    valueChanged=true;
  }
  else if (btnState.KeyReleased(CBtnState::RIGHT_BUTTON) && 
	   chgCount<SPEED_DIV) {
    // Key tapped make sure to move one tick
    speed = 1;
    valueChanged=true;
    key = CBtnState::RIGHT_BUTTON;
  }
  else if (btnState.KeyReleased(CBtnState::LEFT_BUTTON) && 
	   chgCount<SPEED_DIV) {
    // Key tapped make sure to move one tick
    speed = 1;
    valueChanged=true;
    key = CBtnState::LEFT_BUTTON;
  }
  else if (btnState.KeyReleased(CBtnState::UP_BUTTON) && 
	   chgCount<SPEED_DIV) {
    // Key tapped make sure to move one tick
    speed = 1;
    valueChanged=true;
    key = CBtnState::UP_BUTTON;
  }
  else if (btnState.KeyReleased(CBtnState::DOWN_BUTTON) && 
	   chgCount<SPEED_DIV) {
    // Key tapped make sure to move one tick
    speed = 1;
    valueChanged=true;
    key = CBtnState::DOWN_BUTTON;
  }
#ifdef ENABLE_AUTOTRAIN
  else if (btnState.KeyHit(CBtnState::START_BUTTON)) {
    static bool histState=false;
    if(histState) {
      // Currently there, clear it
      WriteMsiToScreen();
    }
    else {
      if(m_options.mode == RM_RAW) {
	HistogramImageBBox();
      }
      else {
	// Not currently in raw mode, so put it in raw mode
	m_options.mode = RM_RAW;
	SetDispOptions(m_options);

	// Force it to try histogram again next time
	histState=true;
      }
    }
    histState=!histState;
  }
#else
  // While we can't do autotrain, use start to cycle mode
  else if (btnState.KeyHit(CBtnState::START_BUTTON)) {
    UpdateModMode(1);
  }
#endif
  else if (btnState.KeyHit(CBtnState::B_BUTTON)) {
    // Exit model context
    return(false);
  } else if(btnState.KeyHit(CBtnState::SELECT_BUTTON)) {
    UpdateShowMode();
    return(true);
  }

  if(valueChanged) {
    // Set m_modelChanged to CVision::MODEL_PARTS to indicate that
    // it's all going to have to change now
    m_modelChanged = CVision::MODEL_PARTS;

    // Calculate the new model values
    CColorModelHSV deltaModel;
    KeyToDeltaModel(key, speed, deltaModel);
    ApplyDeltaModel(m_model, deltaModel);

    UpdateSprites();
    if(chgCount<60) chgCount++;
  }
  else {
    chgCount=1;
  }

  if(m_modelChanged>0) {
    m_vision.UploadModelPartial(m_modelIndex, m_model, 
				modelPhase, modelPhase +1);
    modelPhase = (modelPhase + 1) % CVision::MODEL_PARTS;
    m_modelChanged--;
    doRedraw=true;
  }

  if(doRedraw || valueChanged) {
    UpdateStatusArea();
  }

  if(m_dispSel == SHOW_PIXEL) {
    // Sample center pixel
    unsigned char r, g, b;
    unsigned short h;
    unsigned char s, v;
    unsigned short x, y;
    // Get location of image sprite
    m_imgSprites->GetPos(x, y);
    GbaScreenImage.get_rgb(x, y, r, g, b);
    RgbToHsvOV6620(r<<3, g<<3, b<<3,
		   h, s, v);
    m_hsvMinSprites[0]->SetDecimalValue(h);
    m_hsvMinSprites[1]->SetDecimalValue(s);
    m_hsvMinSprites[2]->SetDecimalValue(v);
    
    m_hsvMaxSprites[0]->SetDecimalValue(r);
    m_hsvMaxSprites[1]->SetDecimalValue(g);
    m_hsvMaxSprites[2]->SetDecimalValue(b);
  }

  return(true);
}

// Model may have changed, read model from m_vision and update screen
void
CModelContext::SyncModelFromVision()
{
    // Read the most recent version of the model from m_vision
    m_vision.GetCurrentModel(m_modelIndex, m_model);
    UpdateSprites();
    UpdateStatusArea();
}

bool 
CModelContext::Push(CVision &vision, 
		    unsigned short modelIndex,
		    CColorModelHSV &model, 
		    const CVisionDispOptions &options)
{
  CModelContext *vc = dynamic_cast<CModelContext*>(DispContextStackSingleton.GetCurrentContext());

  // Check if top of stack is already CModelContext
  if(vc!=NULL) {
    // Top is already CModelContext, just set options
    // TODO!!!
    vc->SetDispOptions(options);
    return(false);
  }
  // Create a vision context and push it onto the stack above this menu
  CModelContext *tContext = new CModelContext(vision, modelIndex, 
					      model, options);
  DispContextStackSingleton.PushContext(tContext);
  return(true);
}

bool 
CModelContext::Pop()
{
  // Check if top of stack is already CModelContext
  if(dynamic_cast<CModelContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is vision context, pop it
    DispContextStackSingleton.PopContext();
    return(true);
  }
  else {
    // Top is not vision context, do nothing
    return(false);
  }
}

void 
CModelContext::UpdateSprites()
{
  // Update the sprites
  int left, top, right, bottom;
  m_hp.ModelToBBox(m_model, left, top, right, bottom);
  m_msiSprites->SetBBox(left + DV_GRAB_COLS, top, 
			right + DV_GRAB_COLS, bottom);

  bool showModel = ((m_dispSel == SHOW_BOTH) || 
		    (m_dispSel == SHOW_MODEL_TEXT));
  bool showPixel =  (m_dispSel == SHOW_PIXEL);
  if(showModel) {
    m_hsvMinSprites[0]->SetDecimalValue(m_model.hMin);
    m_hsvMinSprites[1]->SetDecimalValue(m_model.sMin);
    m_hsvMinSprites[2]->SetDecimalValue(m_model.vMin);
    
    m_hsvMaxSprites[0]->SetDecimalValue(m_model.hMax);
    m_hsvMaxSprites[1]->SetDecimalValue(m_model.sMax);
    m_hsvMaxSprites[2]->SetDecimalValue(m_model.vMax);
  }
  DBG(printf("Update: left=%d, right=%d\n", left, right));
  m_model.Print();
}
