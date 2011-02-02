#include <gba.h>
#include "visioncontext.h"
#include "blobcontext.h"
#include "btnstate.h"
#include "textdisp.h"
#include "textcontext.h"

#include "blob_sprites.h"
#include "blob_sprites.c"

//#define DEBUG

#ifndef M_PI 
#define M_PI		3.14159265358979323846
#endif

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

CBlobContext::CBlobContext(CInterruptCont *pIntCont, 
			   CVision &vision, 
			   const CVisionDispOptions &options):
  CVisionContext(vision, options), 
  m_intCont(pIntCont), m_lastFrameNum(0), 
  m_blobAssm(0x7, options.sortBlobs)
{
  DBG(printf("Creating CBlobContext\n"));
  DBG(m_options.PrintSettings());

  DBG(printf(" Creating Palette\n"));
  m_spritePalette = new CPalette(CPalette::OBJ, blob_spritesPalette, 0);
  DBG(printf(" Creating Tiles\n"));
  m_tiles = new CTileSet(5, DV_MODELS*NUM_CHANNEL_TILES, blob_spritesData);
  DBG(printf(" Creating Sprites\n"));
  m_spriteSet = new CSpriteSet(m_intCont);

  DBG(printf("  Initialize Sprites\n"));
  // Setup blob sprites, initially hide them all
  for(int i=0; i<DV_MODELS; i++) {
    DBG(printf("   Create color %d\n", i));
    m_blobColors[i] = new CSprite(*m_tiles, i*NUM_CHANNEL_TILES, 
				  m_options.enable[i], 
				  8*i, 0);
    DBG(printf("   Reg color %d\n", i));
    m_spriteSet->RegisterSprite(*(m_blobColors[i]), true);
    for(int j=0; j<MAX_CHANNEL_BLOBS; j++) {
      DBG(printf("   Create blob (%d,%d)\n", i, j));
      m_blobSprites[i][j] = 
	new CSpriteBox(*m_tiles, (i*NUM_CHANNEL_TILES)+1, 
		       (i*NUM_CHANNEL_TILES)+2, false);
      DBG(printf("   Reg blob (%d,%d)\n", i, j));
      m_spriteSet->RegisterSprite(*(m_blobSprites[i][j]), true);
    }      
    m_lastShownBlobNum[i] = 0;
  }
  DBG(printf("  Setup ShadowMem\n"));
  m_spriteSet->SetupShadowMem();
  DBG(printf("done\n"));
}

CBlobContext::~CBlobContext() 
{
  DBG(printf("Destructing CBlobContext\n")); 
  delete m_spriteSet;
  delete m_tiles;
  delete m_spritePalette;
  DBG(printf("done\n"));
}

void 
CBlobContext::SetInterruptCont(CInterruptCont *pIntCont)
{
  m_intCont = pIntCont;
  // TODO: Disable/enable interrupts if we're showing?
}

void 
CBlobContext::SetDispOptions(const CVisionDispOptions &options)
{
  printf("Setting CVisionContext Display Options...");
  CVisionContext::SetDispOptions(options);
  m_options.PrintSettings();
  printf("done\nSetting CBlobContext Display Options...");
  for(int i=0; i<DV_MODELS; i++) {
    // If disabling a channel or now showing blobs at all, hide the
    // color standard and all blob sprites
    if(!m_options.enable[i] || !m_options.showBlobs) {
      m_blobColors[i]->Hide();
      for(int j=0; j<m_lastShownBlobNum[i] && j<MAX_CHANNEL_BLOBS; j++) {
	m_blobSprites[i][j]->Hide();
      }      
      m_lastShownBlobNum[i]=0;
    }
    else {
      // Enabling channel and showing blobs, show color standard
      m_blobColors[i]->Show();
    }
    // Set enable for the built-in blob assembler to match rendering
    m_blobAssm.SetEnable(i, m_options.enable[i]);
  }
  printf("done\n");
}

bool 
CBlobContext::Blur()
{
  DBG(printf("Blurring CBlobContext\n")); 
  CVisionContext::Blur();
  m_spriteSet->ReleaseDisplay();
  DBG(printf("done\n"));
  return(true);
}

bool 
CBlobContext::Focus()
{
  // Setup sprite palette and tiles
  m_spritePalette->WriteToGBA();
  m_tiles->WriteToGBA();

  // Setup vision context for displaying video
  CVisionContext::Focus();

  // Setup the registers, interrupt, and OAM memory to display sprites
  m_spriteSet->SetupDisplay();

  // Setup display options to make sure render enable is setup properly, etc.
  SetDispOptions(m_options);

  // Ignore whatever buttons are currently being pressed
  m_blobContextIgnoreBtns=true;

  return(true);
}

void 
CBlobContext::HideBlobSprites(int channel, int bnum)
{
  if(channel<0 || channel>=DV_MODELS || bnum<0 || bnum>=MAX_CHANNEL_BLOBS) {
    return;
  }
  m_blobSprites[channel][bnum]->Hide();
}

// These are helper functions to rescale imager coords onto screen.
// This is to account for both the division by two done by the
// subsampling and the render scaling setup by CVision::SetupDisplay:
//	SetRenderScaling((DV_GRAB_COLS<<8)/DV_GBA_SCREEN_WIDTH, 
//		(DV_GRAB_ROWS<<8)/DV_GBA_SCREEN_HEIGHT);
short ScaleX(int pos) {
  return((short)(((pos * DV_GBA_SCREEN_WIDTH)/2)/DV_GRAB_COLS));
}
short ScaleY(int pos) {
  return((short)(((pos * DV_GBA_SCREEN_HEIGHT)/2)/DV_GRAB_ROWS));
}

void 
CBlobContext::UpdateBlobSprites(int channel, int bnum, CBlob *blob)
{
  if(blob==NULL || channel<0 || channel>=DV_MODELS || 
     bnum<0 || bnum>=MAX_CHANNEL_BLOBS) {
    return;
  }

  // Get the stats for the blob
  SMomentStats stats;
  blob->moments.GetStats(stats);

  short scenterX = ScaleX((short)(stats.centroidX));
  short scenterY = ScaleY((short)(stats.centroidY));
  short left, top, right, bottom;
  blob->getBBox(left, top, right, bottom);

  short sleft, stop, sright, sbottom;
  sleft = ScaleX(left);
  stop = ScaleY(top);
  sright = ScaleX(right);
  sbottom = ScaleY(bottom);

  m_blobSprites[channel][bnum]->SetPos(scenterX, scenterY);
  m_blobSprites[channel][bnum]->SetBBox(sleft,
					stop,
					sright,
					sbottom);

  if(m_options.showBlobText) {
    printf("Blob %d:%d: area %d, centroid (%f, %f)\n", 
	   channel, bnum, stats.area, stats.centroidX, stats.centroidY);
    if(m_options.doBlobAxes) {
      printf("                axis %f, aspect %f\n", 
	     stats.angle*180.0/M_PI, 
	     stats.minorDiameter / stats.majorDiameter);
    }
  }

//   if(showell) {
//     float cscale = 2.0 * coordShift;
//     draw_ellipse(dest, stats.centroidX/cscale, 
// 		 stats.centroidY/cscale, 
// 		 stats.angle,
//                  stats.majorDiameter/(2.0 + cscale), 
// 		 stats.minorDiameter/(2.0 + cscale),
//                  accent_color);
//   }
}

void 
CBlobContext::UpdateChannelBlobs(int channel, CBlob *blobs)
{
  if(channel<0 || channel>=DV_MODELS) {
    return;
  }
  if(!m_options.enable[channel]) {
    // Hide all channel blobs
    for(int i=0; i<m_lastShownBlobNum[channel] && i<MAX_CHANNEL_BLOBS; i++) {
      HideBlobSprites(channel, i);
    }
    m_lastShownBlobNum[channel]=0;
    return;
  }

  // Channel is enabled, show all blobs up to MAX_CHANNEL_BLOBS
  int i=0;
  bool shownBlobs=false;

  while(blobs!=NULL && i<MAX_CHANNEL_BLOBS) {
    if(blobs->GetArea()>m_options.minBlobArea) {
      UpdateBlobSprites(channel, i++, blobs);
      shownBlobs=true;
    }
    blobs = blobs->next;
  }
  if(!m_options.showBlobText && shownBlobs) {
    printf(".");
  }
    
  // Hide any additional blobs that were shown last frame and are now gone
  for(int j=i; j<m_lastShownBlobNum[channel] && j<MAX_CHANNEL_BLOBS; j++) {
    HideBlobSprites(channel, j);
  }
  m_lastShownBlobNum[channel]=i;
}

bool 
CBlobContext::Process()
{
  static CBtnState btnState;
  bool setopt = false;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  if(m_blobContextIgnoreBtns) {
    btnState.PollKeys();
    m_blobContextIgnoreBtns=false;
  }

  btnState.PollKeys();

  if(btnState.KeyHit(CBtnState::A_BUTTON)) {
    CycleRenderMode();
  }
  if(btnState.KeyHit(CBtnState::UP_BUTTON)) {
    printf("Showing All\n");
    m_options.EnableAll();
    setopt = true;
  } else if(btnState.KeyHit(CBtnState::LEFT_BUTTON)) {
    printf("Showing only Ch 0\n");
    m_options.DisableAll();
    m_options.SetEnable(0, true);
    setopt = true;
  } else if(btnState.KeyHit(CBtnState::DOWN_BUTTON)) {
    printf("Showing only Ch 1\n");
    m_options.DisableAll();
    m_options.SetEnable(1, true);
    setopt = true;
  } else if(btnState.KeyHit(CBtnState::RIGHT_BUTTON)) {
    printf("Showing only Ch 2\n"); 
    m_options.DisableAll();
    m_options.SetEnable(2, true);
    setopt = true;
  } else if(btnState.KeyHit(CBtnState::SELECT_BUTTON)) {
    // Push text display onto top of stack 
    CTextContext::Push();
    return(true);
  } else if(btnState.KeyHit(CBtnState::B_BUTTON)) {
    // Exit vision processing
    printf("Exiting CBlobContext\n"); 
    return(false);
  }

  if(setopt) {
    printf("Setting Disp Options...");
    SetDispOptions(m_options);
    printf("done\n");
  }

  static int nskipped = 0;
  unsigned int currFrameNum = m_vision.GetCurrFrameNum();
  
  if(!m_vision.NewSegmentData()) {
    nskipped++;
  } else {
    if(m_options.showBlobText) {
      printf("Process: curr %d, last blobs %d\n  skipped %d loops, %sshowing blobs\n", 
	     currFrameNum, m_lastFrameNum, nskipped, 
	     m_options.showBlobs?"":"not ");
    }
    nskipped = 0;
    m_lastFrameNum = m_vision.BuildBlobs(m_blobAssm);

    if(m_options.showBlobs) {
      for (int i=0; i<DV_MODELS; i++) {
	UpdateChannelBlobs(i, m_blobAssm.GetChannelBlobs(i));
      }
      if(m_intCont == NULL) {
	// If interrupts are not enabled, manually update the OAM memory
	m_spriteSet->WriteUpdatesToGBA();
      }
    }
  }

  return(true);
}

bool 
CBlobContext::Push(CInterruptCont *pIntCont, 
		     CVision &vision, 
		     const CVisionDispOptions &options)
{
  CBlobContext *vc = dynamic_cast<CBlobContext*>(DispContextStackSingleton.GetCurrentContext());

  // Check if top of stack is already CBlobContext
  if(vc!=NULL) {
    // Top is already CBlobContext, just set options
    vc->SetDispOptions(options);
    return(false);
  }
  // Create a vision context and push it onto the stack above this menu
  CBlobContext *tContext = new CBlobContext(pIntCont, vision, options);
  DispContextStackSingleton.PushContext(tContext);
  return(true);
}

bool 
CBlobContext::Pop()
{
  // Check if top of stack is already CBlobContext
  if(dynamic_cast<CBlobContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is vision context, pop it
    DispContextStackSingleton.PopContext();
    return(true);
  }
  else {
    // Top is not vision context, do nothing
    return(false);
  }
}
