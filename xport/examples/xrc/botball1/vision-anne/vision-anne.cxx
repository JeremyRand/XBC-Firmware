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
// 
//  END LICENSE BLOCK 

#include "xrc_image.h"
#include "blob.h"
#include "blob_image.h"
#include "../../libxrc/vision.h"
#include "tone.h"
#include <intcont.h>
#include <textdisp.h>
#include <gba.h>

#define MAX_HRANGE 120

#define BAR_H (GBA_NROWS - DV_GRAB_ROWS)/3 - 2
#define BAR_W DV_GRAB_COLS

// class CColorModelHSV {
// public:
//   CColorModelHSV(short hMin=0, short hMax=0, 
// 		 short sMin=0, short sMax=0,
// 		 short vMin=0, short vMax=0):
//     hMin(hMin), hMax(hMax), 
//     sMin(sMin), sMax(sMax), 
//     vMin(vMin), vMax(vMax) {
//   }
//   void Set(short hMin, short hMax, 
// 	   short sMin, short sMax,
// 	   short vMin, short vMax) {
//     hMin = hMin; 
//     sMin = sMin; 
//     vMin = vMin; 
//     hMax = hMax;    
//     sMax = sMax;
//     vMax = vMax;
//   }

//   void Upload(CVision &vis, int modelIndex) {
//     vis.UploadModel(modelIndex, hMin, hMax, 
// 		    sMin, sMax, 
// 		    vMin, vMax);
//   }
  
//   void Limit() {
//     if(hMin < 0) hMin = 0;
//     if(hMin > MAX_H) hMax = MAX_H;
//     if(hMax < hMin) hMax = hMin;
//     if(hMax - hMin > MAX_HRANGE) hMax = hMin + MAX_HRANGE;

//     if(sMin < 0) sMin = 0;
//     if(sMin > MAX_S) sMax = MAX_S;
//     if(sMax < sMin) sMax = sMin;

//     if(vMin < 0) vMin = 0;
//     if(vMin > MAX_V) vMax = MAX_V;
//     if(vMax < vMin) vMax = vMin;
//   }

// public:
//   short hMin, hMax;
//   short sMin, sMax;
//   short vMin, vMax;
// };

CColorModelHSV colorModels[DV_MODELS];

CBlob *find_biggest_blob(CBlob *blob, int minarea)
{
  int maxsize = 0;
  CBlob *maxblob = NULL;
  SMomentStats stats;

  while(blob!=NULL) {
    blob->moments.GetStats(stats);
    if(stats.area > minarea && stats.area > maxsize) {
      maxblob = blob;
      maxsize = stats.area;
    }
    blob = blob->next;
  }
  return(maxblob);
}

class CDerivedVision : public CVision {
public:
  CDerivedVision(CInterruptCont *pIntCont);

  bool GrabCameraImage(Image &img) {
    int i;
    unsigned short *sp; 
    unsigned short state;

    img.resize(DV_GRAB_ROWS, DV_GRAB_COLS);

    // wait for vsync
    while(!(*m_grabConfig&DV_GRAB_CONFIG_VSYNC));
    while((*m_grabConfig&DV_GRAB_CONFIG_VSYNC));
    // grab 128 lines
    for (i=0, sp=(unsigned short *)GBA_BASE_VRAM; i<DV_GRAB_ROWS; i++, sp+=240)
      {
	// wait for hsync
	state = *m_grabConfig&DV_GRAB_CONFIG_BANK;
	while((*m_grabConfig&DV_GRAB_CONFIG_BANK)==state);
	
	// setup DMA channel 3 to transfer a line of camera video to video memory
	GBA_REG_DMA3SAD = m_base+0x800;
	GBA_REG_DMA3DAD = (long)sp;
	GBA_REG_DMA3CNT_L = DV_GRAB_COLS;
	GBA_REG_DMA3CNT_H = 0x8000;
      }

#if USE_OLD_GRAB
    // wait for vsync
    while(!(*m_grabConfig&0x02));
    while((*m_grabConfig&0x02));
    // grab 128 lines onto gba screen
    for (i=0, sp=(unsigned short *)GBA_BASE_VRAM; i<DV_GRAB_ROWS; 
	 i++, sp+=240) {
      // wait for hsync
      state = *m_grabConfig&0x04;
      while((*m_grabConfig&0x04)==state);

      // setup DMA channel 3 to transfer a line of camera video to video memory
      GBA_REG_DMA3SAD = m_base+0x800;
      GBA_REG_DMA3DAD = (long)sp;
      GBA_REG_DMA3CNT_L = DV_GRAB_COLS;
      GBA_REG_DMA3CNT_H = 0x8000;
    }
#endif
    // Copy data into the image passed in
    img.read_from_gba_screen();
    return(true);
  }
protected:
  virtual void PostRender();

protected:
  bool do_blobify;
  bool showseg;
  bool showell;
  bool showtext;
  bool do_sound;
  CBlobAssembler bass;
  CTone tone;
};


CDerivedVision::CDerivedVision(CInterruptCont *pIntCont) : 
  CVision(pIntCont), do_blobify(true), showseg(false), showell(false), 
  showtext(false), do_sound(true)
{
  SetRenderMode(RM_PROCESSED);
}

void CDerivedVision::PostRender()
{
  if (!(GBA_REG_P1&GBA_KEY_A)) {
    SetRenderMode(RM_RAW);
    do_blobify=false;
  }
  else if(!(GBA_REG_P1&GBA_KEY_B)) {
    SetRenderMode(RM_PROCESSED);
  }
  else if(!(GBA_REG_P1&GBA_KEY_SL)) {
    do_sound= !do_sound;
    printf("Toggled do_sound to %s\n", 
	   do_sound?"true":"false");
    while(!(GBA_REG_P1&GBA_KEY_SL));
  }
  else if (!(GBA_REG_P1&GBA_KEY_LFT)) {
    if(!showseg || !do_blobify) {
      printf("Will show blob segments\n");
      showseg=true;
      do_blobify=true;
      SetRenderMode(RM_PROCESSED);
    }
  }
  else if (!(GBA_REG_P1&GBA_KEY_RT)) {
    if(showseg || !do_blobify) {
      printf("Will show blob without segments\n");
      showseg=false;
      do_blobify=true;
      SetRenderMode(RM_PROCESSED);
    }
  }
  else if (!(GBA_REG_P1&GBA_KEY_L)) {
    showtext=true;
  }
  else if (!(GBA_REG_P1&GBA_KEY_R)) {
    showtext=false;
  }

  if(do_blobify) {
    image_blobify(GbaScreenImage, bass, showseg, 
		  DV_GRAB_ROWS, DV_GRAB_COLS, this);
    show_blobs(GbaScreenImage, bass.finishedBlobs, 100, showseg, 
	       showseg, showell, showtext);
    CBlob *maxblob = find_biggest_blob(bass.finishedBlobs, 100);
    if(maxblob!=NULL) {
      SMomentStats stats;
      maxblob->moments.GetStats(stats);
      int freq = 64 + ((int)(stats.centroidX*1000.0)/DV_GRAB_COLS);
      if(showtext) {
	printf("Maxblob at X = %f, freq=%d", stats.centroidX, freq);
      }
      tone.SetFreq(freq);
      if(!tone.IsEnabled()) {
	tone.SetEnable(true);
      }
    }
    else {
      tone.SetEnable(false);
    }
  }
}

////////////////////////////////////////////////////////////////////////////
void modify_LUT(CDerivedVision &vis, int modelIndex)
{
  if(modelIndex<0 || modelIndex>DV_MODELS) {
    return;
  }

  int psel = 0;
  bool done=false;
  bool model_changed = false;

  CColorModelHSV &model = colorModels[modelIndex];
  Image mimg(GBA_NROWS, DV_GRAB_COLS);
  Image cimg(DV_GRAB_ROWS, DV_GRAB_COLS);

  printf("Img (%d,%d)\n", GBA_NROWS, DV_GRAB_COLS);

  mimg.read_from_gba_screen();

  while(!done) {
    int i, row;
    int rows[DV_MODELS];
    printf("psel = %d\n", psel);

    // Clear the bar display area
    mimg.draw_fillrect(0, DV_GRAB_ROWS+1, DV_GRAB_COLS, GBA_NROWS-1, 
		       Image::make_rgb(0, 0, 0));

    // Draw open rectangles for bars
    for(i=0, row=DV_GRAB_ROWS + 1; i<DV_MODELS; i++, row += BAR_H+2) {
      ushort color = Image::make_rgb(31, 31, 31);
      if(i == psel) {
	color = Image::make_rgb(31, 31, 0);
      }
      rows[i] = row;
      printf("Box %d: %d, %d, %d, %d (color 0x%x)\n", i, 
	     0, row, DV_GRAB_COLS-1, row + BAR_H, color);
      mimg.draw_box(0, row, DV_GRAB_COLS-1, row + BAR_H, color);
    }
  
    float hscale = (BAR_W - 2) / ((float)(MAX_H));
    float sscale = (BAR_W - 2) / ((float)(MAX_S));
    float vscale = (BAR_W - 2) / ((float)(MAX_V));

    // Draw H bar
    printf("Bar H (%d->%d): %d, %d, %d, %d\n", model.hMin, model.hMax,
	   (int)(model.hMin * hscale)+1, rows[0]+1, 
	   (int)(model.hMax * hscale)+1, rows[1]-3);
    mimg.draw_fillrect((int)(model.hMin * hscale)+1, rows[0]+1, 
		       (int)(model.hMax * hscale)+1, rows[1]-3,
		       Image::make_rgb(31, 0, 0));

    // Draw S bar
    printf("Bar S (%d->%d): %d, %d, %d, %d\n", model.sMin, model.sMax,
	   (int)(model.sMin * sscale)+1, rows[1]+1, 
	   (int)(model.sMax * sscale)+1, rows[2]-3);
    mimg.draw_fillrect((int)(model.sMin * sscale)+1, rows[1]+1, 
		       (int)(model.sMax * sscale)+1, rows[2]-3,
		       Image::make_rgb(0, 31, 0));
    // Draw V bar
    printf("Bar V (%d->%d): %d, %d, %d, %d\n", model.vMin, model.vMax,
	   (int)(model.vMin * vscale)+1, rows[2]+1, 
	   (int)(model.vMax * vscale)+1, rows[2]+BAR_H-1);
    mimg.draw_fillrect((int)(model.vMin * vscale)+1, rows[2]+1, 
		       (int)(model.vMax * vscale)+1, rows[2]+BAR_H-1,
		       Image::make_rgb(0, 0, 31));

    printf("Drawing\n");
    mimg.write_to_gba_screen(0, DV_GRAB_ROWS);
    //mimg.write_to_gba_screen();

    bool got_input = false;
    while(!got_input) {
      bool mod_a=!(GBA_REG_P1&GBA_KEY_A);
      bool mod_b=!(GBA_REG_P1&GBA_KEY_B);
      bool mod_none = !mod_a && !mod_b;
      int mod_dir = 0;
      
      if (!(GBA_REG_P1&GBA_KEY_UP)) {
	if(psel > 0) psel--;
	while(!(GBA_REG_P1&GBA_KEY_UP));
	break;
      }
      else if (!(GBA_REG_P1&GBA_KEY_DWN)) {
	if(psel < 2) psel++;
	while(!(GBA_REG_P1&GBA_KEY_DWN));
	break;
      }
      else if (!(GBA_REG_P1&GBA_KEY_LFT)) {
	printf("Left\n");
	mod_dir=-1;
	got_input=true;
      }
      else if (!(GBA_REG_P1&GBA_KEY_RT)) {
	printf("Right\n");
	mod_dir=1;
	got_input=true;
      }
      else if (!(GBA_REG_P1&GBA_KEY_ST)) {
	if(model_changed) {
	  printf("Uploading color model\n");
	  vis.UploadModel(modelIndex, model);
	  model_changed = false;
	}
	printf("Grabbing image\n");
	while(!(GBA_REG_P1&GBA_KEY_ST)) {
	  vis.SetRenderMode(RM_PROCESSED);
	  vis.GrabCameraImage(cimg);
	}
	printf("Image grab DONE\n");
	break;
      }
      else if (!(GBA_REG_P1&GBA_KEY_R)) {
	printf("EXITING TRAINING\n");
	while(!(GBA_REG_P1&GBA_KEY_R));
	break;
      }
      if(got_input) {
	if(mod_a || mod_none) {
	  if(psel == 0) model.hMax += mod_dir;
	  if(psel == 1) model.sMax += mod_dir;
	  if(psel == 2) model.vMax += mod_dir;
	}
	if(mod_b || mod_none) {
	  if(psel == 0) model.hMin += mod_dir;
	  if(psel == 1) model.sMin += mod_dir;
	  if(psel == 2) model.vMin += mod_dir;
	}
	if(mod_dir != 0) {
	  model_changed = true;
	  model.Limit();
	}
      }
    }
  }
  printf("DONE\n");
}

////////////////////////////////////////////////////////////////////////////
CTextDisp td(TDM_CPORT);

int main(void)
{
  int i,j;
  bool showell = false;

  CInterruptCont intCont;
  CDerivedVision vision(&intCont);
  CBlobAssembler bass;

  colorModels[0].Set(0, 15, 200, 224, 100, 224);
  vision.UploadModel(0, 0, 15, 200, 224, 100, 224);
  // colorModels[0].Upload(vision, 0);

  vision.SetRenderColor(0, DV_BUILD_COLOR(31, 0, 0));
  vision.SetRenderColor(1, DV_BUILD_COLOR(31, 31, 31));
  vision.SetRenderColor(2, DV_BUILD_COLOR(31, 31, 31));

  vision.SetRenderMode(RM_RAW);
  vision.SetRenderMode(RM_NONE);

  Image img(DV_GRAB_ROWS, DV_GRAB_COLS);

  for (i=0; i<DV_GRAB_ROWS; i++) {
    for (j=0; j<DV_GRAB_COLS; j++) {
      int n = (31-(i+j))%31;
      img.pixel(j,i) = DV_BUILD_COLOR(n,n,n);
    }
  }

  img.draw_fillrect(10, 10, DV_GRAB_COLS-10, DV_GRAB_ROWS-10, 
		    DV_BUILD_COLOR(0, 0, 31));
  img.draw_fillrect(20, 20, DV_GRAB_COLS-20, DV_GRAB_ROWS-20, 
		    DV_BUILD_COLOR(0, 31, 0));

  img.draw_fillrect(30, 30, DV_GRAB_COLS-30, DV_GRAB_ROWS-30,
		    DV_BUILD_COLOR(31, 0, 0));

  img.draw_cross(DV_GRAB_COLS/2, DV_GRAB_ROWS/2,20,
		 DV_BUILD_COLOR(31, 31, 31));

  img.write_to_gba_screen();

  printf("-------------------------------------------------\n");
  printf("Vision-anne v1.2\n");
  printf("Controls:\n");
  printf("\tA: Capture RAW image from camera\n");
  printf("\tB: Capture PROCESSED image from camera\n");
  printf("\tStart: Go into RenderLoop\n");

  printf("\nThe following operate on the current stored image\n");
  printf("\tLeft: Display blobs with segments\n");
  printf("\tRight: Display blobs without segments\n");
  printf("\tUp: Display image to screen (clear graphics)\n");

  printf("\nThe following set options\n");
  printf("\tDown: Toggle display of ellipses\n");
  printf("\tL: In RenderLoop, print text\n");
  printf("\tR: In RenderLoop, do not print text\n\n");

  while(1) {
    if (!(GBA_REG_P1&GBA_KEY_A)) {
      printf("Grabbing RAW image\n");
      // If A is held down grab raw images from the camera
      vision.SetRenderMode(RM_RAW);
      vision.GrabCameraImage(img);
      printf("Captured raw image from camera\n");
    }
    else if (!(GBA_REG_P1&GBA_KEY_B)) {
      printf("Grabbing PROCESSED image\n");
      // If B is held down, grab processed images from the camera
      vision.SetRenderMode(RM_PROCESSED);
      vision.GrabCameraImage(img);
      printf("Captured processed image from camera\n");
    }
    else if (!(GBA_REG_P1&GBA_KEY_UP)) {
      img.write_to_gba_screen();
      while(!(GBA_REG_P1&GBA_KEY_UP));
    }
    else if (!(GBA_REG_P1&GBA_KEY_DWN)) {
      showell = !showell;
      printf("Toggled show ellipse to %s\n", 
	     showell?"true":"false");
      while(!(GBA_REG_P1&GBA_KEY_DWN));
    }
    else if (!(GBA_REG_P1&GBA_KEY_LFT)) {
      printf("Blobifying image, showing segments\n");
      image_blobify(img, bass, true, DV_GRAB_ROWS, DV_GRAB_COLS, &vision);
      show_blobs(GbaScreenImage, bass.finishedBlobs, 100, true, true, showell);
      while(!(GBA_REG_P1&GBA_KEY_LFT));
    }
    else if (!(GBA_REG_P1&GBA_KEY_RT)) {
      printf("Blobifying image, not showing segments\n");
      image_blobify(img, bass, false, DV_GRAB_ROWS, DV_GRAB_COLS, &vision);
      show_blobs(GbaScreenImage, bass.finishedBlobs, 100, false, false, showell);
      while(!(GBA_REG_P1&GBA_KEY_RT));
    }
    else if (!(GBA_REG_P1&GBA_KEY_ST)) {
      printf("Entering RenderLoop, power cycle to get back to normal mode\n");
      break;
    }
    else if (!(GBA_REG_P1&GBA_KEY_SL)) {
      printf("Entering modify_LUT\n");
      modify_LUT(vision, 0);
    }
  }
	  
  vision.SetRenderMode(RM_PROCESSED);
  vision.RenderLoop();
}

