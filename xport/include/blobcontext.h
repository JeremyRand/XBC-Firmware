#ifndef CBLOBCONTEXT_H
#define CBLOBCONTEXT_H

#include "visioncontext.h"
#include "palette.h"
#include "display.h"
#include "sprites.h"
#include "spritebox.h"

#define NUM_CHANNEL_TILES 3

//  Modifying this number affects how many blobs show per channel.
//  Remember that there is a maximum of 128 sprites available in a
//  context.  Each spritebox takes 5 sprites and there are three
//  channels, so the blobs take 15*MAX_CHANNEL_BLOBS sprites.  Setting
//  this to 5 leaves 53 sprites free for use by subclasses.
#define MAX_CHANNEL_BLOBS 5

class CInterruptCont;

// This context holds onto a reference to an associated vision object,
// but does not own the storage for the vision.  It is up to the user
// to make sure that the context is freed before its vision, and that
// the vision is eventually freed.
class CBlobContext : public CVisionContext
{
public:
  CBlobContext(CInterruptCont *pIntCont, 
		 CVision &vision, 
		 const CVisionDispOptions &options = 
		 CVisionDispOptions());
  virtual ~CBlobContext();

  virtual bool Blur();	  // Newer context on top of stack, lose focus
  virtual bool Focus();	  // Newer context(s) done, top of stack again

  virtual bool Process();	// Do a quantum of work and return

  virtual void SetInterruptCont(CInterruptCont *pIntCont);
  CInterruptCont *GetInterruptCont() const {
    return(m_intCont);
  }

  virtual void SetDispOptions(const CVisionDispOptions &options);

  void HideBlobSprites(int channel, int bnum);
  void UpdateBlobSprites(int channel, int bnum, CBlob *blob);

  // Update blob display for a given channel
  void UpdateChannelBlobs(int channel, CBlob *blobs);
  
  // Push vision context with requested render mode and options onto
  // top of display stack if top is not already showing vision.
  // Otherwise set render mode and options.
  static bool Push(CInterruptCont *pIntCont, 
		   CVision &vision, 
		   const CVisionDispOptions &options = 
		   CVisionDispOptions(RM_COMBINED, true)); 

  // Pop off top of display stack if top is currently showing vision
  static bool Pop();

protected:
  // Blob Sprites
  CInterruptCont *m_intCont;
  CPalette *m_spritePalette;
  CTileSet *m_tiles;
  CSpriteSet *m_spriteSet;
  CSprite *m_blobColors[DV_MODELS];
  CSpriteBox *m_blobSprites[DV_MODELS][MAX_CHANNEL_BLOBS];
  unsigned int m_lastFrameNum;
  unsigned short m_lastShownBlobNum[DV_MODELS];

  CMultiChannelBlobAssembler m_blobAssm;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_blobContextIgnoreBtns;
};

#endif
