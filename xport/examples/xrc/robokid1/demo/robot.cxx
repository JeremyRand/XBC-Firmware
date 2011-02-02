 //  BEGIN LICENSE BLOCK
// 
//  Version: MPL 1.1
// 
//  The contents of this file are subject to the Mozilla Public License Version
//  1.1 (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
// 
//  Software distributed under the License is distributed on an "AS IS" basis,
//  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
//  for the specific language governing rights and limitations under the
//  License.
// 
//  The Original Code is The Xport Software Distribution.
// 
//  The Initial Developer of the Original Code is Charmed Labs LLC.
//  Portions created by the Initial Developer are Copyright (C) 2003
//  the Initial Developer. All Rights Reserved.
// 
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
//					Cody Lodrige
//					Anne Wright
//  END LICENSE BLOCK 

#include <stdio.h>
#include <textdisp.h>
#include <axesc.h>
#include <string.h>
#include "vision.h"
#include "intcont.h"
#include "gba.h"
#include "visioncontext.h"
#include "visionmenu.h"
#include "menucontext.h"
#include "textcontext.h"
#include "blobcontext.h"
#include "btnstate.h"
#include "botball1.h"
#include "spritestring.h"
#include "blobtiles.h"
#include "gba_image.h"

#define STATUS_TEXT_XPOS 2
#define STATUS_TEXT_YPOS (DV_GRAB_ROWS+2)
#define STATUS_TEXT_LEN (DV_GRAB_COLS/8)

//  Increase this number if you want to show more blobs per channel,
//  but remember that there is a maximum of 128 sprites available in a
//  context.  Each spritebox takes 5 sprites and there are three
//  channels, so the blobs take 15*MAX_DISP_BLOBS sprites, and you
//  need enough left over for the sprite strings...
#define MAX_DISP_BLOBS 1

////////////////////////////////////////////////////////////////////////
// Ignore all this...
class CodyContext : public CVisionContext
{
public:
  CodyContext(CAxesClosed &ac, CVision &vision);
  
  virtual ~CodyContext() {}
  bool Process();	// Do a quantum of work and return
  
  bool Focus();
  bool Blur();
  void SetScreenScale();
  virtual void SetDispOptions(const CVisionDispOptions &options);
  void ClearScreenEdges();

  bool AnalyzeBlob(CBlob *blob);
  virtual short ScaleX(int pos);
  virtual short ScaleY(int pos);

  void UpdateBlobSprites(int channel, int bnum, CBlob *blob);

protected:
  CAxesClosed &m_ac;
  CVision &m_vision;
  CMultiChannelBlobAssembler m_blobAssm; 
  CBlobTiles m_blobTiles;
  CSpriteBox *m_blobSprites[DV_MODELS][MAX_CHANNEL_BLOBS];
  CSpriteString m_statusSprites;
  CSpriteSet m_spriteSet;
  unsigned short m_lastShownBlobNum[DV_MODELS];
  bool m_codyIgnoreBtns;
};

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

CodyContext::CodyContext(CAxesClosed &ac, CVision &vision) :
  CVisionContext(vision),
  m_ac(ac),
  m_vision(vision),
  m_blobAssm(),
  m_blobTiles(CPalette::OBJ, 5), 
  m_statusSprites(m_blobTiles.GetTileSet(), 
		  STATUS_TEXT_LEN, 
		  CBlobTiles::A_TILE, 
		  CBlobTiles::BLANK_TILE, 
		  true, STATUS_TEXT_XPOS, STATUS_TEXT_YPOS),
  m_spriteSet(vision.GetInterruptCont())
{
  // Setup blob sprites, initially hide them all
  for(int i=0; i<DV_MODELS; i++) {
    DBG(printf("   Create color %d\n", i));
//     m_blobColors[i] = new CSprite(*m_tiles, i*NUM_CHANNEL_TILES, 
// 				  m_options.enable[i], 
// 				  8*i, 0);
    DBG(printf("   Reg color %d\n", i));
//     m_spriteSet->RegisterSprite(*(m_blobColors[i]), true);
    for(int j=0; j<MAX_DISP_BLOBS; j++) {
      DBG(printf("   Create blob (%d,%d)\n", i, j));
      m_blobSprites[i][j] = 
	new CSpriteBox(m_blobTiles.GetTileSet(), (i*NUM_CHANNEL_TILES)+1, 
		       (i*NUM_CHANNEL_TILES)+2, false);
      DBG(printf("   Reg blob (%d,%d)\n", i, j));
      m_spriteSet.RegisterSprite(*(m_blobSprites[i][j]), true);
    }      
    m_lastShownBlobNum[i] = 0;
  }

  // Make corner tiles more visible
  for(int corner=0; corner<4; corner++) {
    m_blobSprites[0][0]->GetCornerSprite(corner)->
      SetTileIndex(CBlobTiles::CYAN_CORNER_TILE);
  }
   
  m_statusSprites.SetText("Lodrige robotics demo");
  m_spriteSet.RegisterSprite(m_statusSprites, false);
  m_spriteSet.SetupShadowMem();

  //  char buf[256];

  // sprintf(buf, "hi %d\n", 2);
  //  m_statusSprites.SetText(buf);
  //  m_statusSprites.SetText("Hi Cody");
  // m_spriteSet.RegisterSprite(m_statusSprites, false);
  // m_spriteSet.SetupShadowMem();
}
short  CodyContext::ScaleX(int pos) {
  return((short)(((pos * DV_GBA_SCREEN_WIDTH)/2)/DV_GRAB_COLS));
}
short  CodyContext::ScaleY(int pos) {
  return((short)(((pos * DV_GBA_SCREEN_HEIGHT)/2)/DV_GRAB_ROWS));
}

void CodyContext::UpdateBlobSprites(int channel, int bnum, CBlob *blob)
{
  if(blob==NULL || channel<0 || channel>=DV_MODELS || 
     bnum<0 || bnum>=MAX_DISP_BLOBS) {
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

// Set the scale of the screen to include the image at the top, and the 
// HSV bars at the bottom
void CodyContext::SetScreenScale()
{
  // Set texture scaling to show full picture
  GBA_REG_BG2PA = 1<<8;
  GBA_REG_BG2PD = 1<<8;
}
void CodyContext::SetDispOptions(const CVisionDispOptions &options)
{
  CVisionContext::SetDispOptions(options);
  SetScreenScale();
  // Clear the edges not covered by camera DMA to be black
  ClearScreenEdges();
}

void CodyContext::ClearScreenEdges()
{
  // Clear anything extra to the right
  for(int y=0; y<DV_GRAB_ROWS; y++) {
    for(int x=DV_GRAB_COLS; x<GBA_DISP_NCOLS; x++) {
      GbaScreenImage.pixel(x, y) = 0;
    }
   }
  // Clear anything extra below
  for(int y=DV_GRAB_ROWS; y<GBA_DISP_NROWS; y++) {
    for(int x=0; x<GBA_DISP_NCOLS; x++) {
      GbaScreenImage.pixel(x, y) = 0;
    }
  }
}

bool CodyContext::Focus()
{
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
  m_codyIgnoreBtns=true;

  printf("Cody context focused\n");
  return(true);
}

bool 
CodyContext::Blur()
{
  DBG(printf("Blurring CodyContext\n")); 
  CVisionContext::Blur();
  m_spriteSet.ReleaseDisplay();
  DBG(printf("done\n"));
  return(true);
}

bool CodyContext::Process() 
{
  static CBtnState btnState;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  if(m_codyIgnoreBtns) {
    btnState.PollKeys();
    m_codyIgnoreBtns=false;
  }

  // Read the buttons
  btnState.PollKeys();
  
  if(btnState.KeyHit(CBtnState::A_BUTTON)) {
    CycleRenderMode();
  } else if(btnState.KeyHit(CBtnState::SELECT_BUTTON)) {
    // Stop the motors
    m_ac.Stop(1);
    m_ac.Stop(3);
    m_ac.Execute();
    m_ac.SetPWM(1, 0);
    m_ac.SetPWM(3, 0);
    m_ac.Execute();

    // Push text display onto top of stack 
    CTextContext::Push();
    return(true);
  }
  else if(btnState.KeyHit(CBtnState::B_BUTTON)) {
    // Exit CodyContext
    return(false);
  }
  
  if(!m_vision.NewSegmentData()){ 
    return(true);
  }

  int frameNum = m_vision.BuildBlobs(m_blobAssm); 
  CBlob *biggest = m_blobAssm.GetChannelBlobs(0);

  if(biggest == NULL) {
    printf(".");
  }    

  if(AnalyzeBlob(biggest) == true){
    UpdateBlobSprites(0, 0, biggest);
    printf("Yay!\n");
  }
  else {
    printf("-");
  }
  return(true);
}

class CodyMenuHandler : public IMenuHandler
{
public:
  CodyMenuHandler(CAxesClosed &ac, CVision &vision) : m_ac(ac), m_vision(vision) {
  }
  virtual ~CodyMenuHandler() {}

  virtual bool HandleMenuEvent(int eventType, 
			       CMenuElement &menu)
  {
      static CBtnState btnState;
    if(eventType == 0){
    printf("Creating CodyContext\n");
    // Push a CodyContext on the top of the stack
    CodyContext *cody = new CodyContext(m_ac, m_vision);
    DispContextStackSingleton.PushContext(cody);
    printf("Returning from HandleMenuEvent\n");
    return(true);
    }
    else if(eventType == 1){
      CTextContext::Push();
      printf("MOTOR SETUP\n\n");
      //      m_ac. SyncTrajectory(1);
      m_ac.Move(1, 0, 10000, 10000);
      m_ac.Move(3, 0, 10000, 10000);
      m_ac.Execute();
       while(!btnState.KeyHit(CBtnState::B_BUTTON))		{
	 btnState.PollKeys();
 			printf("%d, %d\n", m_ac.GetPosition(1), m_ac.GetPosition(3));
// 		  // move to position 50000 
// 		m_ac.MoveVelocity(1, 60000, 100000);
// 	       	m_ac.MoveVelocity(3, 60000, 100000);
		
// 		m_ac.Execute();
// 		while(!m_ac.Done(0))
// 			printf("%d, %d\n", m_ac.GetPosition(1), m_ac.GetPosition(3));
// 	// move to position 50000 
// 	       	m_ac.MoveVelocity(1, 60000, 100000);
// 	       	m_ac.MoveVelocity(3, 60000, 100000);
		
// 		m_ac.Execute();
// 		while(!m_ac.Done(0))
// 			printf("%d, %d\n", m_ac.GetPosition(1), m_ac.GetPosition(3));

		
		}
       printf("motor test complete");
       CTextContext::Pop();
    }
    return(true);
  }
protected:
   CAxesClosed &m_ac;
   CVision &m_vision;
};

////////////////////////////////////////////////////////////////////////
// Change this

#define MIN_HAPPY_SIZE 160
#define CENTER_X 355/2
#define X_OK_DIST 45

bool CodyContext::AnalyzeBlob(CBlob *blob)
{  
  DBG(printf("Entering CodyContext::AnalyzeBlob, blob=0x%x\n", blob)); 
  //ClearScreenEdges();
 // Decide if blob is happy, return true if so
  if( (blob->GetArea()) < MIN_HAPPY_SIZE) {
    DBG(printf("  Too small, return false\n")); 
    return(false);
  }

  // Get the stats for the blob
  SMomentStats stats;
  blob->moments.GetStats(stats);

  // Find out how far from the center we are
  int error = ((int)stats.centroidX - CENTER_X);

  if(abs(error) <= X_OK_DIST) {
 printf("%d \n", (blob->GetArea()));   
    m_statusSprites.SetText("center");
    m_blobSprites[0][0]->SetTileIndex(m_blobTiles.GetLetterTile('c'));
    printf("Center: error %d, x=%.1f, %d\n", error, stats.centroidX, m_ac.GetPosition(1));
     if( (blob->GetArea()) > 10000) {
      m_ac.MoveVelocity(1, 60000, 10000);
      m_ac.MoveVelocity(3, 60000, 10000);
		
       // m_ac.Stop(1);
       // m_ac.Stop(3);
       //   m_ac.Execute();   
       // m_ac.Hold(2, 0);
       //m_ac.SyncTrajectory(1);
       //m_ac.SyncTrajectory(3); 
       // m_ac.Move(3, (m_ac.GetPosition(3)+2050), 10000, 10000);
       // m_ac.Move(1, (m_ac.GetPosition(1)+2050), 10000, 10000);
       m_ac.Execute();
 
     } else if( (blob->GetArea()) > 2800) {
       m_ac.Stop(1);
       m_ac.Stop(3);
       m_ac.Execute();
       printf("Im staying here");

     }  else if( (blob->GetArea()) > 2100) { 
      m_ac.MoveVelocity(1, -60000, 10000);
      m_ac.MoveVelocity(3, -60000, 10000);    
 //  m_ac.Stop(1);
     // m_ac.Stop(3);
     // m_ac.Execute(); 
       // m_ac.Move(3, (m_ac.GetPosition(3)-2050), 10000, 10000);
       // m_ac.Move(1, (m_ac.GetPosition(1)-2050), 10000, 10000);
        m_ac.Execute();
     }
  }
  else if(error < 0) {
  
  // Blob is to the left
    m_blobSprites[0][0]->SetTileIndex(m_blobTiles.GetLetterTile('l'));
    m_statusSprites.SetText("left");
    printf("Left:  error %d, x=%.1f\n", error, stats.centroidX);
      m_ac.MoveVelocity(1, -4000, 5000);
      m_ac.MoveVelocity(3, 4000, 5000);    
    // m_ac.Move(3, (m_ac.GetPosition(3)+450), 10000, 10000);
    // m_ac.Move(1, (m_ac.GetPosition(1)-450), 10000, 10000);
    m_ac.Execute();
  
  }
  else {
    m_statusSprites.SetText("right");
    // Blob is to the right
     m_blobSprites[0][0]->SetTileIndex(m_blobTiles.GetLetterTile('r'));
    printf("Right:  error %d, x=%.1f\n", error, stats.centroidX);
    m_ac.MoveVelocity(1, 4000, 5000);
    m_ac.MoveVelocity(3, -4000, 5000);    
    // m_ac.Move(3, (m_ac.GetPosition(3)-450), 10000, 10000);
    // m_ac.Move(1, (m_ac.GetPosition(1)+450), 10000, 10000);
    m_ac.Execute();
   
  }
  DBG(printf("Exiting CodyContext::AnalyzeBlob\n", blob)); 
  return(true);
}


CTextDisp td(TDM_LCD_AND_CPORT);
 
int main(void) 
{ 
 CInterruptCont intCont;

 // Note that this wouldn't work if we were to use the default args for the second and third arguments
 CAxesClosed ac(&intCont, DAO_BEMF_BASE, 20);


 ac.SetPIDGains(500, 0, 650);
//  ac.SyncTrajectory(1);
//  ac.MoveVelocity(1, 60000, 100000);
//  ac.Execute(); 
//  while(1)
//    {
//      printf("%d\n", ac.GetPosition(1));
//    }
  printf("Initializing Vision systems...\n\n");


 CVision vision(&intCont);


  CBlob::recordSegments= true;
  SMoments::computeAxes= true; 
 
  // This is to setup the vision menu 
  printf("Setting up vision menu...\n");

CodyMenuHandler chandler(ac, vision);

   CMenuDisp menu(&chandler);

  menu.AddMenuItem("  Ball Following  ", 0);
  menu.AddMenuItem("  Motor Setup  ", 1);

CVisionMenuHandler vmenu(vision);

  CMenuElement *topElt = menu.GetCurrentElement();
  vmenu.PopulateMenu(*topElt);

  CMenuContext *mContext = new CMenuContext(menu);
  DispContextStackSingleton.PushContext(mContext);
   
  while(1) { 
    // If there's something on the stack, like a blob context, let it 
    // do some stuff now
    DispContextStackSingleton.Process();
  } 
}
