#ifndef CCAMERACONTEXT_H
#define CCAMERACONTEXT_H

#include "visioncontext.h"
#include "blobtiles.h"
#include "spritestring.h"
#include "spritebox.h"
#include "spritenum.h"
#include "camera.h"

class CPalette;
class CTileSet;
class CSpriteSet;
class CSprite;
class CSpriteBox;
class CSpriteString;
class CSpriteNum;

#define NUM_DISP_SETTINGS 9

// This context holds onto a reference to an associated camera object,
// but does not own the storage for the camera.  It is up to the user
// to make sure that the context is freed before its camera, and that
// the camera is eventually freed.
class CCameraContext : public CVisionContext
{
public:
  struct CamSettingInfo {
    const char *name;
    int         row;
    int         cam_index;
    int         minval;
    int         maxval;
    int         skip;
  };

public:
  CCameraContext(CVision &vision, 
		 const CVisionDispOptions &options = CVisionDispOptions());
  virtual ~CCameraContext();

  virtual bool Blur();	  // Newer context on top of stack, lose focus
  virtual bool Focus();	  // Newer context(s) done, top of stack again
  virtual bool Process();	// Do a quantum of work and return

  // Set the scale of the screen to include the image at the top, and the 
  // text area at the bottom
  void SetScreenScale();
  virtual void SetDispOptions(const CVisionDispOptions &options);
  void ClearScreenEdges();
  void UpdateSettingSprites();
  void ChangeSelParam(int amt);
  void UpdateStatusForSelection();

  // Print usage instructions
  void Usage();
  void PrintCameraSettings();

  // Push camera context onto top of display stack if top is not
  // already showing it, otherwise set options
  static bool Push(CVision &vision, 
		   const CVisionDispOptions &options = CVisionDispOptions()); 
  // Pop off top of display stack if top is currently showing camera
  // context
  static bool Pop();		

protected:
  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_cameraContextIgnoreBtns;

  // Text Sprite support
  CBlobTiles m_blobTiles;
  CSpriteString m_statusSprites;
  CSpriteString m_usageSprites;
  CSpriteString *m_settingsLabelSprites[NUM_DISP_SETTINGS];
  CSpriteNum *m_settingsValueSprites[NUM_DISP_SETTINGS];
  CSpriteSet m_spriteSet;

  CSpriteBox m_cursor;
  int m_selParam;

  CCameraSettings m_settings;
  CCameraSettings m_origSettings;
};

#endif
