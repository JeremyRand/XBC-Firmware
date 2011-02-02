#include <stdio.h>
#include <gba.h>
#include "string.h"
#include "vision.h"
#include "btnstate.h"
#include "gba_image.h"

#include "cameracontext.h"
#include "textcontext.h"
#include "spritestring.h"
#include "blobtiles.h"

#include <textdisp.h>

#define BAR_H (GBA_NROWS - DV_GRAB_ROWS)/3 - 2
#define BAR_W DV_GRAB_COLS

#define STATUS_TEXT_XPOS 0
#define STATUS_TEXT_YPOS (DV_GRAB_ROWS+1)
#define STATUS_TEXT_LEN 30

#define USAGE_TEXT_XPOS 2
#define USAGE_TEXT_YPOS (DV_GRAB_ROWS+1+8)
#define USAGE_TEXT_LEN 30

#define SLABEL_TEXT_XPOS (DV_GRAB_COLS + 3)
#define SLABEL_TEXT_YPOS 2
#define SLABEL_TEXT_LEN 4

#define SVALUE_TEXT_XPOS (SLABEL_TEXT_XPOS + (SLABEL_TEXT_LEN*8) + 3)
#define SVALUE_TEXT_YPOS 2

// Center cursor on text/value area to the left of the screen in X
#define CURSOR_XPOS ((DV_GRAB_COLS + GBA_NCOLS)/2 - 2)
// Make cursor span text area left to right
#define CURSOR_XSIZE (GBA_NCOLS - DV_GRAB_COLS - 2) 
// Make cursor center on text of row in Y
#define CURSOR_YPOS(row) (SVALUE_TEXT_YPOS + 8*(row) + 2)
// Make cursor span a line of text top to bottom
#define CURSOR_YSIZE 10

// #define DEBUG

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

#define CSI_AWB_INDEX 0
#define CSI_RED_INDEX 1
#define CSI_BLUE_INDEX 2

#define CSI_AEC_INDEX 3
#define CSI_EXP_INDEX 4

#define CSI_AGC_INDEX 5
#define CSI_GAIN_INDEX 6

#define CSI_ERAT_INDEX 7
#define CSI_ERLV_INDEX 8

#define CSI_MIN_INDEX 0
#define CSI_MAX_INDEX 8

  // Setup settings sprites
struct CCameraContext::CamSettingInfo cs_info[NUM_DISP_SETTINGS] = {
  {"AWB", 0, CAM_AWB_INDEX, 0, 1, 1}, 
  {"Red", 1, CAM_TEMP_RED_INDEX, 0, 255, 1}, 
  {"Blue", 2, CAM_TEMP_BLUE_INDEX, 0, 255, 1}, 

  {"AEC", 4, CAM_AEC_ENABLE_INDEX, 0, 1, 1}, 
  {"Exp", 5, CAM_EXPOSURE_INDEX, 0, 255, 1}, 

  {"AGC", 7, CAM_AGC_ENABLE_INDEX, 0, 1, 1}, 
  {"Gain", 8, CAM_GAIN_INDEX, 0, 248, 4},

  {"ERat", 10, CAM_AEC_RATIO_INDEX, 1, 254, 4},
  {"ERLv", 11, CAM_EXP_RL_INDEX, 0, 0xE0, 0x20}
};

/////////////////////////////////////////////////////////////////////////
// Camera Context
CCameraContext::CCameraContext(CVision &vision,
			       const CVisionDispOptions &options) :
  CVisionContext(vision, options),
  m_blobTiles(CPalette::OBJ, 5), 
  m_statusSprites(m_blobTiles.GetTileSet(), 
		  STATUS_TEXT_LEN, 
		  CBlobTiles::A_TILE, 
		  CBlobTiles::BLANK_TILE, 
		  true, STATUS_TEXT_XPOS, STATUS_TEXT_YPOS),
  m_usageSprites(m_blobTiles.GetTileSet(), 
		 USAGE_TEXT_LEN, 
		 CBlobTiles::A_TILE, 
		 CBlobTiles::BLANK_TILE, 
		 true, USAGE_TEXT_XPOS, USAGE_TEXT_YPOS),
  m_spriteSet(vision.GetInterruptCont()), 
  m_cursor(m_blobTiles.GetTileSet(), 
	   CBlobTiles::BLANK_TILE, 
	   CBlobTiles::IMAGE_CORNER_TILE, true, 
	   CURSOR_XPOS, 
	   CURSOR_YPOS(0),
	   CURSOR_XSIZE,
	   CURSOR_YSIZE),
  m_selParam(0)
{
  DBG(printf("Creating CCameraContext\n"));

  // Setup status sprites, setup zero index so it can also display numbers
  m_statusSprites.SetZeroIndex(CBlobTiles::ZERO_TILE);
  UpdateStatusForSelection();
  m_spriteSet.RegisterSprite(m_statusSprites, false);

  m_usageSprites.SetText("Select button shows usage");
  m_spriteSet.RegisterSprite(m_usageSprites, false);

  // Register cursor with sprite set
  m_spriteSet.RegisterSprite(m_cursor, false);

  // Setup settings sprites
  for(int i=0; i<NUM_DISP_SETTINGS; i++) {
    const char *name = cs_info[i].name; 
    int row = cs_info[i].row; 

    if(name == NULL) {
      name = "none";
    }
    
    DBG(printf("Creating sprites for %s, row %d\n", 
	       name, row));
    m_settingsLabelSprites[i] = new CSpriteString(m_blobTiles.GetTileSet(), 
						  SLABEL_TEXT_LEN, 
						  CBlobTiles::A_TILE, 
						  CBlobTiles::BLANK_TILE, 
						  true, SLABEL_TEXT_XPOS, 
						  SLABEL_TEXT_YPOS + row*8);
    m_spriteSet.RegisterSprite(*m_settingsLabelSprites[i], true);
    m_settingsLabelSprites[i]->SetText(name);

    m_settingsValueSprites[i] = new CSpriteNum(m_blobTiles.GetTileSet(), 
					       3, 
					       CBlobTiles::ZERO_TILE, 
					       CBlobTiles::BLANK_TILE, 
					       true, SVALUE_TEXT_XPOS, 
					       SVALUE_TEXT_YPOS + row*8);
    m_spriteSet.RegisterSprite(*m_settingsValueSprites[i], true);
  }

  m_spriteSet.SetupShadowMem();
  UpdateSettingSprites();
}

CCameraContext::~CCameraContext() 
{
}

bool 
CCameraContext::Focus()
{
  static bool showUsage=true;

  DBG(printf("Focusing CCameraContext\n"));

  // Setup blob tiles
  m_blobTiles.WriteToGBA();

  // Setup vision context for displaying video
  CVisionContext::Focus();

  // Setup the registers, interrupt, and OAM memory to display sprites
  m_spriteSet.SetupDisplay();

  // Clear the edges not covered by camera DMA to be black
  ClearScreenEdges();

  // Setup the scale to show full area
  SetScreenScale();

  // Ignore whatever buttons are currently being pressed
  m_cameraContextIgnoreBtns=true;

  // Print usage info
  Usage();

  if(showUsage) {
    showUsage=false;
    // Push text display onto top of stack first time using this mode
    CTextContext::Push();
    return(true);
  }

  // Record the original settings in m_origSettings
  m_vision.m_camera.ReadSettings(m_origSettings);

  return(true);
}

bool 
CCameraContext::Blur()
{
  DBG(printf("Blurring CCameraContext\n")); 
  CVisionContext::Blur();
  m_spriteSet.ReleaseDisplay();
  DBG(printf("done\n"));
  return(true);
}

// Set the scale of the screen to include the image at the top, and the 
// HSV bars at the bottom
void 
CCameraContext::SetScreenScale()
{
  // Set texture scaling to show full picture
  GBA_REG_BG2PA = 1<<8;
  GBA_REG_BG2PD = 1<<8;
}
void 
CCameraContext::SetDispOptions(const CVisionDispOptions &options)
{
  CVisionContext::SetDispOptions(options);
  SetScreenScale();
  // Clear the edges not covered by camera DMA to be black
  ClearScreenEdges();
}

void 
CCameraContext::ClearScreenEdges()
{
  // Clear anything extra to the right
  for(int y=0; y<DV_GRAB_ROWS; y++) {
    for(int x=DV_GRAB_COLS; x<GBA_NCOLS; x++) {
      GbaScreenImage.pixel(x, y) = 0;
    }
  }
  // Clear anything extra below
  for(int y=DV_GRAB_ROWS; y<GBA_NROWS; y++) {
    for(int x=0; x<GBA_NCOLS; x++) {
      GbaScreenImage.pixel(x, y) = 0;
    }
  }
}

#define SPEED_DIV 5
#define UPDATE_DIV 5

void 
CCameraContext::ChangeSelParam(int amt) 
{
  CamSettingInfo &si = cs_info[m_selParam];

  int oldval=m_settings.GetValue(si.cam_index);
  int newval=oldval + amt * si.skip;

  if(newval < si.minval) newval = si.minval;
  if(newval > si.maxval) newval = si.maxval;

  m_vision.m_camera.SetValue(si.cam_index, newval);

#ifdef DEBUG 
  m_vision.m_camera.ReadSettings(m_settings);
  printf("Changing setting for %s (%d)\n  Was %d, now %d, read %d\n", 
	 si.name, si.cam_index, oldval, newval, 
	 m_settings.GetValue(si.cam_index));
#endif
}

void 
CCameraContext::UpdateStatusForSelection() 
{
  // Make newstr longer than needed to help avoid memory corruption in
  // the case that the format string below is too long
  char newstr[STATUS_TEXT_LEN*2];
  int index = cs_info[m_selParam].cam_index;

  sprintf(newstr, "%8s: orig %3d, def %3d", 
	  CCameraSettings::GetIndexName(index), 
	  m_origSettings.GetValue(index),
	  CCameraSettings::GetIndexDefval(index));
  m_statusSprites.SetText(newstr);
}

bool 
CCameraContext::Process()
{
  static CBtnState btnState;
  static unsigned short updateCount=0;
  bool doUpdate = true;
  bool ret=true;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  if(m_cameraContextIgnoreBtns) {
    btnState.PollKeys();
    m_cameraContextIgnoreBtns=false;
  }

  btnState.PollKeys();

  if (btnState.KeyHit(CBtnState::START_BUTTON)) {
    // Calibrate
    m_statusSprites.SetText("Starting calibration");
    m_vision.m_camera.CalibrateWhite();
    m_statusSprites.SetText("Done");
  } if (btnState.KeyHit(CBtnState::RIGHT_BUTTON)) {
    // Modify selected parameter
    ChangeSelParam(1);
    doUpdate = true;
  }
  else if (btnState.KeyHit(CBtnState::LEFT_BUTTON)) {
    // Modify selected parameter
    ChangeSelParam(-1);
    doUpdate = true;
  }
  else if (btnState.KeyHit(CBtnState::UP_BUTTON)) {
    if(m_selParam>CSI_MIN_INDEX) m_selParam--;
    if(m_settings.awbEnabled && m_selParam == CSI_BLUE_INDEX) {
      // Can't set Red/Blue if auto AWB so skip up to AWB
      m_selParam = CSI_AWB_INDEX;
    }
    else if(m_selParam == CSI_EXP_INDEX) {
      // Can't set exposure, so skip up to AEC
      m_selParam = CSI_AEC_INDEX;
    }
    else if(m_settings.agcEnabled && m_selParam == CSI_GAIN_INDEX) {
      // Can't set gain if auto AGC so skip up to AGC
      m_selParam = CSI_AGC_INDEX;
    }

    UpdateStatusForSelection();
    doUpdate = true;
  }
  else if (btnState.KeyHit(CBtnState::DOWN_BUTTON)) {
    if(m_selParam<CSI_MAX_INDEX) m_selParam++;
    if(m_settings.awbEnabled && m_selParam == CSI_RED_INDEX) {
      // Can't set Red/Blue if auto AWB so skip down to AEC
      m_selParam = CSI_AEC_INDEX;
    }
    else if(m_selParam == CSI_EXP_INDEX) {
      // Can't set exposure, so skip down to AGC
      m_selParam = CSI_AGC_INDEX;
    }
    else if(m_settings.agcEnabled && m_selParam == CSI_GAIN_INDEX) {
      // Can't set gain if auto AGC so skip down to ERAT
      m_selParam = CSI_ERAT_INDEX;
    }

    UpdateStatusForSelection();
    doUpdate = true;
  }
  else {
    ret = CVisionContext::Process();
  }

  if(doUpdate || updateCount++ > UPDATE_DIV) {
    updateCount = 0;
    UpdateSettingSprites();
  }
  return(ret);
}

void 
CCameraContext::Usage()
{
  ptd->Clear();
  printf("Camera Settings\n\n");
  printf("Right section: \n");
  printf(" AWB/AEC/AGC: \n");
  printf("   0=Manual, 1=Auto\n");
  printf(" Others:\n");
  printf("   Parameter value in use\n");
  printf("\n");
  printf("Controls:\n");
  printf(" Up/Down: select param\n");
  printf(" Left/Right: -/+ sel param\n");
  printf(" Start: Calibrate White\n");
  printf(" A:     Toggle video mode\n");
  printf(" B:     Exit context\n");
  printf("\n");
  printf(" Press B to Start");
}

bool 
CCameraContext::Push(CVision &vision,
		     const CVisionDispOptions &options)
{
  CCameraContext *vc = dynamic_cast<CCameraContext*>(DispContextStackSingleton.GetCurrentContext());

  // Check if top of stack is already CCameraContext
  if(vc!=NULL) {
    // Top is already CCameraContext, just set options
    // TODO!!!
    vc->SetDispOptions(options);
    return(false);
  }
  // Create a camera context and push it onto the stack
  CCameraContext *tContext = new CCameraContext(vision, options);
  DispContextStackSingleton.PushContext(tContext);
  return(true);
}

bool 
CCameraContext::Pop()
{
  // Check if top of stack is already CCameraContext
  if(dynamic_cast<CCameraContext*>(DispContextStackSingleton.GetCurrentContext())) {
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
CCameraContext::PrintCameraSettings()
{
  m_vision.m_camera.PrintSettings();
}

void 
CCameraContext::UpdateSettingSprites()
{
  m_vision.m_camera.ReadSettings(m_settings);

  for(int i=0; i<NUM_DISP_SETTINGS; i++) {
    int val = m_settings.GetValue(cs_info[i].cam_index);
    m_settingsValueSprites[i]->SetDecimalValue(val);
  }

  // Set the cursor position
  int row = cs_info[m_selParam].row;
  m_cursor.SetPos(CURSOR_XPOS, CURSOR_YPOS(row));
}
