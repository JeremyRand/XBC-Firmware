#ifndef CMODELCONTEXT_H
#define CMODELCONTEXT_H

#include "visioncontext.h"
#include "colormodel.h"

class CPalette;
class CTileSet;
class CSpriteSet;
class CSprite;
class CSpriteBox;
class CSpriteString;
class CSpriteNum;

// This context holds onto a reference to an associated model object,
// but does not own the storage for the model.  It is up to the user
// to make sure that the context is freed before its model, and that
// the model is eventually freed.
class CModelContext : public CVisionContext
{
public:
  enum {
    BLANK_TILE = 9, 
    W_CORNER_TILE, 
    CYAN_CORNER_TILE,
    MAGENTA_CORNER_TILE,
    YELLOW_CORNER_TILE,
    ZERO_TILE=14,
    A_TILE=24,
    NUM_TILES=50 };

  enum {
    MOD_TL_BR=0,
    MOD_MOVE,
    MOD_RESIZE,
    MOD_NUM_MODES
  };

  enum {
    SHOW_BOTH=0,
    SHOW_MODEL_TEXT,
    SHOW_BARS,
    SHOW_TEXTBUF,
    SHOW_NUM_MODES,
    // To Re-enable show, momve this up above to where you want it to 
    // be in the rotation of display modes
    SHOW_PIXEL,
  };
  
  CModelContext(CVision &vision, 
		unsigned short modelIndex,
		CColorModelHSV &model, 
		const CVisionDispOptions &options = 
		CVisionDispOptions());
  virtual ~CModelContext();

  virtual bool Blur();	  // Newer context on top of stack, lose focus
  virtual bool Focus();	  // Newer context(s) done, top of stack again
  virtual bool Process();	// Do a quantum of work and return

  virtual void SetDispOptions(const CVisionDispOptions &options);

  // Set the scale of the screen to include the image at the top, and the 
  // HSV bars at the bottom
  void SetScreenScale();

  // Copy msimage onto the screen
  void WriteMsiToScreen();
  void HistogramImageBBox();

  // Set delta model based on mode and key press
  void KeyToDeltaModel(int key, int speed, CColorModelHSV &deltaModel);
  void ApplyDeltaModel(CColorModelHSV &m, const CColorModelHSV &d);

  // Set the position of the bounding box sprites based on the model
  void UpdateShowMode();
  void UpdateModMode(int dir);
  void SetupModeSprites();
  void UpdateSprites();
  void SetHsvSpriteVisibility(bool visible);
  void SetSpriteArrVisibility(CSprite *sa[], int n, bool visible);

  // Support for highlighting/dehighlighting selected corners
  void SetCornerHighlight(CSpriteBox *sbox, int corner, bool enable);

  // Draw horizontal bars on the screen image corresponding to the 
  // model HSV range
  void UpdateStatusArea();
  int GetLetterTile(char c);
  
  // These are helper functions to convert scale between image coords
  // and sprite coords.  Sprite coords map directly to GBA screen
  // pixels.  The image applies a scaling to the X direction to stretch
  // DV_GRAB_COLS to cover the physical width of the screen.
  short ImgToSpriteX(int pos);
  short ImgToSpriteY(int pos);
  short SpriteToImgX(int pos);
  short SpriteToImgY(int pos);

  // Model may have changed, read model from m_vision and update screen
  void SyncModelFromVision();

  // Print usage instructions
  void Usage();

  // Push model context onto top of display stack if top is not
  // already showing it, otherwise set options
  static bool Push(CVision &vision, 
		   unsigned short modelIndex,
		   CColorModelHSV &model, 
		   const CVisionDispOptions &options = 
		   CVisionDispOptions()); 
  // Pop off top of display stack if top is currently showing model
  // context
  static bool Pop();		

protected:
  CColorModelHSV &m_model;
  CHsvPlane m_hp;

  unsigned short m_modelIndex;
  // Which mode is currently selected for modification
  unsigned short m_modSel;
  // Which mode is currently selected for display
  unsigned short m_dispSel;
  // Which of L or R was last selected: 0 = left, 1 = right
  unsigned short m_LR;

  // Set to CVision::MODEL_PARTS when the model changes, counts down
  // to 0 as incremental changes are made.  When it is 0 it means that
  // the LUT matches the model.
  int m_modelChanged;

  // Sprite support
  CPalette *m_spritePalette;
  CTileSet *m_tiles;
  CSpriteSet *m_spriteSet;

  CSpriteBox *m_imgSprites;
  CSpriteBox *m_msiSprites;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_modelContextIgnoreBtns;

  // The following are for HSV value display
  CSpriteString *m_modeLabelSprites;
  CSprite *m_hsvLabelSprites[3];
  CSprite *m_rgbLabelSprites[3];
  CSpriteNum *m_hsvMinSprites[3];
  CSpriteNum *m_hsvMaxSprites[3];
};

#endif
