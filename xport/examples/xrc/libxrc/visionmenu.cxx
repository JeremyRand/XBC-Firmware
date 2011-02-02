#include <stdio.h>

#include "visionmenu.h"
#include "blobcontext.h"
#include "modelcontext.h"
#include "cameracontext.h"
#include "textcontext.h"
#include "btnstate.h"
#include "flash.h"
#include <string.h>
#include <textdisp.h>

#define MODEL_FLASH_ADDR 0x8180000
#define PRE_CAMSET_MODEL_MAGIC 0xdeadf001
#define MODEL_MAGIC 0xdeadf002

//#define DEBUG

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

/////////////////////////////////////////////////////////////////////
// Vision Options
CVisionOptionsHandler::CVisionOptionsHandler(CVision &vision, 
					     const CVisionDispOptions &options) :
  m_vision(vision), m_options(options), 
  m_enableEnt0("enable0", m_options.enable[0]), 
  m_enableEnt1("enable1", m_options.enable[1]), 
  m_enableEnt2("enable2", m_options.enable[2]), 
  m_showBlobsEnt("showBlobs", m_options.showBlobs),
  m_showBlobTextEnt("showBlobText", m_options.showBlobText),
  m_minBlobAreaEnt("minBlobArea", m_options.minBlobArea), 
  m_sortBlobsEnt("sortBlobs", m_options.sortBlobs),
  m_doBlobAxesEnt("doBlobAxes", m_options.doBlobAxes),
  m_menuElt(0)
{
}

void 
CVisionOptionsHandler::PopulateMenu(CMenuElement &menuElt) 
{
  CMenuElement *subElem = menuElt.AddMenuLink("Display Options", this);
  assert(subElem!=NULL);
  m_enableEnt0.SetName(*subElem->AddMenuItem("", 0));
  m_enableEnt1.SetName(*subElem->AddMenuItem("", 1));
  m_enableEnt2.SetName(*subElem->AddMenuItem("", 2));
  m_showBlobsEnt.SetName(*subElem->AddMenuItem("", 3));
  m_showBlobTextEnt.SetName(*subElem->AddMenuItem("", 4));
  m_minBlobAreaEnt.SetName(*subElem->AddMenuItem("", 5));
  m_sortBlobsEnt.SetName(*subElem->AddMenuItem("", 6));
  m_doBlobAxesEnt.SetName(*subElem->AddMenuItem("", 7));
//   subElem->AddMenuItem("Revert Options", 6);
//   subElem->AddMenuItem("Apply Options", 7);
  m_menuElt = subElem;
}

void 
CVisionOptionsHandler::RefreshMenuEntries()
{
  m_enableEnt0.SetName(*m_menuElt->GetItemAt(0));
  m_enableEnt1.SetName(*m_menuElt->GetItemAt(1));
  m_enableEnt2.SetName(*m_menuElt->GetItemAt(2));
  m_showBlobsEnt.SetName(*m_menuElt->GetItemAt(3));
  m_showBlobTextEnt.SetName(*m_menuElt->GetItemAt(4));
  m_minBlobAreaEnt.SetName(*m_menuElt->GetItemAt(5));
  m_sortBlobsEnt.SetName(*m_menuElt->GetItemAt(6));
  m_doBlobAxesEnt.SetName(*m_menuElt->GetItemAt(7));
}

bool 
CVisionOptionsHandler::HandleMenuEvent(int eventType, 
				       CMenuElement &menu) 
{
  CMenuItem *item = menu.GetItemAt(eventType);
  if(item == NULL) {
    return(false);
  }

  switch(eventType) {
  case 0:
    m_enableEnt0.Update(*item);
    break;
  case 1:
    m_enableEnt1.Update(*item);
    break;
  case 2:
    m_enableEnt2.Update(*item);
    break;
  case 3:
    m_showBlobsEnt.Update(*item);
    break;
  case 4:
    m_showBlobTextEnt.Update(*item);
    break;
  case 5:
    m_minBlobAreaEnt.Update(*item);
    break;
  case 6:
    m_sortBlobsEnt.Update(*item);
    break;
  case 7:
    m_doBlobAxesEnt.Update(*item);
    break;
  default:
    return(false);
  }
  return(true);
}

/////////////////////////////////////////////////////////////////////
// Vision Model
CVisionModelHandler::CVisionModelHandler(CVision &vision) :
  m_vision(vision)
{
}

CVisionModelHandler::~CVisionModelHandler()
{
}

void 
CVisionModelHandler::PopulateMenu(CMenuElement &menuElt) 
{
  CMenuElement *subElem = menuElt.AddMenuLink("Color Models", this);
  assert(subElem!=NULL);
  subElem->AddMenuItem("Modify Model 0", 0);
  subElem->AddMenuItem("Modify Model 1", 1);
  subElem->AddMenuItem("Modify Model 2", 2);

  subElem->AddMenuItem("Print Models", 8);
  
//   CMenuElement *flashElem = subElem->AddMenuLink("Flash Memory", this);
//   flashElem->AddMenuItem("Save Models To Flash", 10);
//   flashElem->AddMenuItem("Load Models From Flash", 11);
//   flashElem->AddMenuItem("Clear Models From Flash", 13);

  subElem->AddMenuItem("Restore To Default", 12);
}

void 
CVisionModelHandler::PrintModels() 
{
  for(int i=0; i<DV_MODELS; i++) {
    unsigned short rColor = m_vision.GetRenderColor(i);
    unsigned char r= (unsigned char) (rColor&0x1f), 
      g= (unsigned char) (rColor>>5)&0x1f, 
      b= (unsigned char) (rColor>>10)&0x1f;

    printf("Model index %d:\n", i);
    // Read the most recent version of the model from m_vision
    m_vision.GetCurrentModel(i, m_models[i]);
    m_models[i].Print();
    printf("  Render color (%d,%d,%d)\n", r,g,b);
  }
}

void 
CVisionModelHandler::RestoreToDefault() 
{
  // Restore models to Default
  unsigned char r[3] = {31, 31, 0}; // red
  unsigned char g[3] = {0, 31, 31}; // yellow
  unsigned char b[3] = {0, 0, 0};	// green

  printf("Set Render Colors to Default\n");
  for (int i=0; i<DV_MODELS; i++) {
    m_vision.SetRenderColor(i, 
			    DV_BUILD_COLOR(r[i], g[i], b[i]));
  }
  printf("Restore Models to Default\n");
  m_models[0].Set(0, 15, 150, 224, 100, 224); // red 
  m_models[1].Set(50, 70, 150, 224, 100, 224); // yellow
  m_models[2].Set(100, 160, 100, 224, 75, 224); // green
  for(int i=0; i<DV_MODELS; i++) {
    m_vision.UploadModel(i, m_models[i]);
  }

//   printf("Reset Camera Vals to Default\n");
//   m_vision.m_camera.ResetToDefaults();
  
  printf("  Done\n");
  PrintModels();
}


bool 
CVisionModelHandler::HandleMenuEvent(int eventType, 
				     CMenuElement &menu) 
{
  DBG(printf("Handling Model Event %d\n", eventType));
  
  if(eventType>=0 && eventType<DV_MODELS) {
    DBG(printf("Pushing CModelContext %d\n", eventType));
    // Read the most recent version of the model from m_vision
    m_vision.GetCurrentModel(eventType, m_models[eventType]);
    CModelContext::Push(m_vision, eventType, m_models[eventType]);
  } else if(eventType == 8) {
    ptd->Clear();
    printf("------------------\nCurrently Loaded Models:\n");
    PrintModels();
    printf("\nPress B to continue...\n");
    // Push text display onto top of stack 
    CTextContext::Push();
//   } else if(eventType == 10) {
//     SaveToFlash();
//   } else if(eventType == 11) {
//     LoadFromFlash();
  } else if(eventType == 12) {
    RestoreToDefault();
//   } else if(eventType == 13) {
//     ClearFlash();
  }

  return(true);
}

/////////////////////////////////////////////////////////////////////
// Vision Flash 
struct CVisionFlashHandler::CVFSettingInfo 
cfs_info[CVisionFlashHandler::CVF_SETNUM] = {
  {"All Models", true, 0x7}, 
  {"Camera Config", false, 0x0}, 
  {"Model 0", true, 0x1}, 
  {"Model 1", true, 0x2}, 
  {"Model 2", true, 0x4}
};
  
CVisionFlashHandler::CVisionFlashHandler(CVision &vision) :
  m_vision(vision), 
  m_setSel(CVF_MOD_ALL), 
  m_setItem(NULL), 
  m_menuElt(0)
{
  m_flash = new CFlash(m_vision.GetInterruptCont());
  if(CheckFlashMagic()) {
    LoadFromFlash(CVF_MOD_ALL);
    LoadFromFlash(CVF_CAM_CONFIG);
  }
  else {
    SetRAMToDefault();
  }
}

CVisionFlashHandler::~CVisionFlashHandler()
{
  delete m_flash;
}

void 
CVisionFlashHandler::PopulateMenu(CMenuElement &menuElt) 
{
  CMenuElement *subElem = menuElt.AddMenuLink("Flash Memory", this);
  assert(subElem!=NULL);

  // Setup settings entry
  m_setItem = subElem->AddMenuItem("", 0);

  subElem->AddMenuItem("View Values", 1);
  subElem->AddMenuItem("Save to Flash", 2);
  subElem->AddMenuItem("Load from Flash", 3);
  subElem->AddMenuItem("Set to Default", 4);
  //  subElem->AddMenuItem("Set Flash to Factory Default", 4);
  m_menuElt = subElem;

  // Update the settings entry to be appropriate
  UpdateSettingEntry();
}

void
CVisionFlashHandler::UpdateSettingEntry() 
{
  // Use the value of m_setSel to set the string displayed by 
  // m_setItem
  char buf[60];

  // Enforce that m_setSel is in bounds
  if(m_setSel < CVF_MOD_ALL) m_setSel=CVF_MOD_ALL;
  else if(m_setSel > CVF_MAX) m_setSel=CVF_MAX;

  // Mark if can go left or right with angle brackets
  const char *prefstr = (m_setSel>0)?" < ":"   "; 
  const char *sufstr = (m_setSel<CVF_SETNUM-1)?" >":"";
  const char *setname = cfs_info[m_setSel].name;

  // Create a string appropriate for the settings entry
  sprintf(buf, "Setting: %s%s%s", prefstr, setname, sufstr);
  DBG(printf("  %s\n", buf));

  // Set the name of the menu item 
  if(m_setItem != NULL) {
    m_setItem->SetName(buf);
  }
}

void
CVisionFlashHandler::ViewValues(int setting) 
{
  if(setting <= CVF_MOD_ALL) {
    // Show each of the models sequentially
    for(int i=CVF_MOD_0; i<=CVF_MOD_2; i++) {
      ViewValues(i);
    }
  }
  else if(setting == CVF_CAM_CONFIG) {
    CCameraSettings ramSettings;
    CCameraSettings flashSettings;
    CCameraSettings defSettings; // Setup by default to default vals

    // Read curr camera values over I2C from the camera
    m_vision.m_camera.ReadSettings(ramSettings); 

    // Get the saved camera values
    bool flashValid=GetSavedCameraConfig(flashSettings);

    printf("Camera settings:\n");
    printf("  %8s RAM Flash Defval\n", "Param");
    for(int i=0; i<CAM_SHARP_INDEX; i++) {
	const char *iname = CCameraSettings::GetIndexName(i);
	printf("  %8s %3d  %3d  %3d\n", iname, 
	       ramSettings.GetValue(i), 
	       flashSettings.GetValue(i), 
	       defSettings.GetValue(i)); 
    }
  }
  else if(setting >= CVF_MOD_0 && setting <= CVF_MOD_2) {
    int index = setting - CVF_MOD_0;

    CColorModelHSV flashModel, ramModel, defModel;
    
    m_vision.GetCurrentModel(index, ramModel);
    GetSavedModel(index, flashModel);
    GetDefaultModel(index, defModel);

    printf("Model %d:\n", index);
    printf("  %5s RAM Flash Defval\n", "Param");
    for(int i=0; i<=MODEL_MAXINDEX; i++) {
	const char *iname = CColorModelHSV::GetIndexName(i);
	printf("  %5s %3d  %3d  %3d\n", iname, 
	       ramModel.GetValue(i), 
	       flashModel.GetValue(i),
	       defModel.GetValue(i)); 
	if(i%2 == 1) {
	  printf("\n");
	}
    }
  }
}
bool
CVisionFlashHandler::HandleMenuEvent(int eventType, 
				     CMenuElement &menu) 
{
  DBG(printf("Handling Flash Event %d\n", eventType));
  
  if(eventType==0) {
    // Requesting change to which setting is being affected
    // Read buttons and modify value
    CBtnState bstate;
    if(!(bstate.CurrState() & CBtnState::LEFT_BUTTON)) {
      // Decrement m_setSel if we can
      if(m_setSel > 0) m_setSel--;
      DBG(printf("Setting m_setSel to %d", m_setSel));

      // Update display of menu entry to reflect potential change
      UpdateSettingEntry();
    } else if(!(bstate.CurrState() & CBtnState::RIGHT_BUTTON)) {
      // Increment m_setSel if we can
      if(m_setSel < CVF_SETNUM-1) m_setSel++;
      DBG(printf("Setting m_setSel to %d", m_setSel));

      // Update display of menu entry to reflect potential change
      UpdateSettingEntry();
    }
    else {
      return(false);
    }
  } else if(eventType == 1) {
    ptd->Clear();
    printf("------------------\n\n\n");
    ViewValues(m_setSel);
    if(m_setSel!=CVF_CAM_CONFIG) printf("\n");
    printf("Press B to continue...\n\n\n");
    // Push text display onto top of stack 
    CTextContext::Push();
  } else if(eventType == 2) {
    SaveRAMSettingToFlash(m_setSel);
  } else if(eventType == 3) {
    LoadFromFlash(m_setSel);
  } else if(eventType == 4) {
    SetRAMSettingToDefault(m_setSel);
  }

  return(true);
}

//////////////////////////////////////////////////////////////////////
// Flash support

// Default models
unsigned char def_r[3] = {31, 31, 0}; // red
unsigned char def_g[3] = {0, 31, 31}; // yellow
unsigned char def_b[3] = {0, 0, 0};	// green


bool
CVisionFlashHandler::CheckFlashMagic() 
{
  unsigned int *fmaddr = (unsigned int *)(MODEL_FLASH_ADDR);
  bool magicOK = (*fmaddr == MODEL_MAGIC || 
		  *fmaddr == PRE_CAMSET_MODEL_MAGIC);
  DBG(printf("Checking Flash Magic\n  Expect 0x%x\n  Found  0x%x\n  Result %s\n",
	     MODEL_MAGIC, *fmaddr, magicOK?"true":"false"));
  return(magicOK);
}

void 
CVisionFlashHandler::_LoadFlashToPrep()
{
  ModelFlashStruct *fms = (ModelFlashStruct*)(MODEL_FLASH_ADDR);
  m_prepFlashData = *fms;

  // Set current magic into prep
  m_prepFlashData.magic = MODEL_MAGIC;
}

void
CVisionFlashHandler::_SetPrepModel(int index, CColorModelHSV &model)
{
  m_prepFlashData.models[index]=model;
}

void 
CVisionFlashHandler::_SetPrepRenderColor(int index, unsigned short color)
{
  m_prepFlashData.renderColor[index]=color;
}

void
CVisionFlashHandler::_SetPrepCameraSettings(CCameraSettings &settings)
{
  m_prepFlashData.camSettings = settings;
}

bool
CVisionFlashHandler::_SetPrepSettingFromRAM(int setting)
{
  if(setting <= CVF_MOD_ALL) {
    // Recursively call this function for each model
    for(int i=CVF_MOD_0; i<=CVF_MOD_2; i++) {
      _SetPrepSettingFromRAM(i);
    }
  }
  else if(setting == CVF_CAM_CONFIG) {
    CCameraSettings ramSettings;

    // Read curr camera values over I2C from the camera
    if(m_vision.m_camera.ReadSettings(ramSettings)==-1) {
      return(false);
    }

    // Write to the prep area for writing to flash at the end of the func
    _SetPrepCameraSettings(ramSettings);
  }
  else if(setting >= CVF_MOD_0 && setting <= CVF_MOD_2) {
    int index = setting - CVF_MOD_0;
    CColorModelHSV ramModel;

    // Fill ramModel with the current model for this channel
    m_vision.GetCurrentModel(index, ramModel);

    // Write the model and render color to the prep area for writing to flash
    _SetPrepModel(index, ramModel);
    _SetPrepRenderColor(index, m_vision.GetRenderColor(index));
  }
  else {
    return(false);
  }
  return(true);
}

bool 
CVisionFlashHandler::_WritePrepToFlash()
{
  int len = sizeof(ModelFlashStruct);

  // Write prep to flash.  Have to erase first and write whole block
  m_flash->Erase(MODEL_FLASH_ADDR, len);
  m_flash->Program(MODEL_FLASH_ADDR,(unsigned short*)(&m_prepFlashData), 
		   len);
  // TODO: Check for failure conditions?
  return(true);
}

bool
CVisionFlashHandler::SaveRAMSettingToFlash(int setting)
{
  // Copy the current state of flash to the prep area
  _LoadFlashToPrep();

  // Fill in the prep area from RAM for the requested setting
  if(!_SetPrepSettingFromRAM(setting)) {
    return(false);
  }
  
  // If everything was successful write to RAM
  return(_WritePrepToFlash());
}

bool
CVisionFlashHandler::SaveModelToFlash(int index, CColorModelHSV &model)
{
  if(index<0 || index>DV_MODELS) return(false);
  _LoadFlashToPrep();
  _SetPrepModel(index, model);
  return(_WritePrepToFlash());
}

bool
CVisionFlashHandler::SaveCameraSettingsToFlash(CCameraSettings &settings)
{
  _LoadFlashToPrep();
  _SetPrepCameraSettings(settings);
  return(_WritePrepToFlash());
}

// void 
// CVisionFlashHandler::SaveToFlash() 
// {
//   int len = sizeof(ModelFlashStruct);
//   ModelFlashStruct mfs;

//   // Write magic at top
//   mfs.magic = MODEL_MAGIC;

//   // Copy models and render colors to rest of mfs
//   for(int i=0; i<DV_MODELS; i++) {
//     // Read the most recent version of the model from m_vision
//     m_vision.GetCurrentModel(i, m_models[i]);
//     mfs.models[i] = m_models[i];
//     mfs.renderColor[i] = m_vision.GetRenderColor(i);
//   }
//   // Copy camera settings to mfs
//   m_vision.m_camera.ReadSettings(mfs.camSettings);

//   printf("Save Models to Flash...");
//   m_flash->Erase(MODEL_FLASH_ADDR, len);
//   m_flash->Program(MODEL_FLASH_ADDR,(unsigned short*)(&mfs), 
// 		   len);
  
//   printf("Done\n");
// }

void 
CVisionFlashHandler::LoadFromFlash(int setting) 
{
  ModelFlashStruct *fms = (ModelFlashStruct*)(MODEL_FLASH_ADDR);

  if(!CheckFlashMagic()) {
    printf("Flash Models Invalid\n");
  }

  if(setting <= CVF_MOD_ALL) {
    // Load each of the models sequentially
    for(int i=CVF_MOD_0; i<=CVF_MOD_2; i++) {
      LoadFromFlash(i);
    }
  }
  else if(setting == CVF_CAM_CONFIG) {
    // Camera settings weren't added to flash initially, make sure 
    // the magic number is current enough before trying to restore it
    unsigned int *fmaddr = (unsigned int *)(MODEL_FLASH_ADDR);
    if(*fmaddr >= MODEL_MAGIC) {
      printf("Load Camera Cfg from Flash...\n");
      if(m_vision.m_camera.WriteSettings(fms->camSettings) == -1) {
	printf("WARNING: WriteSettings failed, ignoring write request\n");
      }
    }
    else {
      printf("No Cam Vals in Flash\n");
    }
  }
  else if(setting >= CVF_MOD_0 && setting <= CVF_MOD_2) {
    int index = setting - CVF_MOD_0;

    printf("Load Model %d from Flash...\n", index);
    m_vision.SetRenderColor(index, fms->renderColor[index]);
    m_vision.UploadModel(index, fms->models[index]);
  }
  else {
    printf("Invalid setting %d\n", setting);
  }    
}

bool
CVisionFlashHandler::GetSavedModel(unsigned char index, 
				   CColorModelHSV &model) 
{
  ModelFlashStruct *fms = (ModelFlashStruct*)(MODEL_FLASH_ADDR);

  if(!CheckFlashMagic() || index>=DV_MODELS) {
    return(false);
  }
  model = fms->models[index];
  return(true);
}

bool
CVisionFlashHandler::GetSavedCameraConfig(CCameraSettings &settings) 
{
  ModelFlashStruct *fms = (ModelFlashStruct*)(MODEL_FLASH_ADDR);

  if(!CheckFlashMagic()) {
    return(false);
  }
  settings = fms->camSettings;
  return(true);
}

void 
CVisionFlashHandler::ClearFlash() 
{
  int len = sizeof(int);
  int zero= 0;
  printf("Clear Flash...");
  m_flash->Erase(MODEL_FLASH_ADDR, len);
  m_flash->Program(MODEL_FLASH_ADDR,(unsigned short*)(&zero),
		   len);
  printf("Done\n");
}

bool
CVisionFlashHandler::GetDefaultModel(int index, CColorModelHSV &model) 
{
  switch(index) {
  case 0:
    model.Set(0, 15, 150, 224, 100, 224); // red 
    break;
  case 1:
    model.Set(50, 70, 150, 224, 100, 224); // yellow
    break;
  case 2:
    model.Set(100, 160, 100, 224, 75, 224); // green
    break;
  default:
    return(false);
  }
  return(true);
}

bool
CVisionFlashHandler::SetRAMSettingToDefault(int setting) 
{
  if(setting <= CVF_MOD_ALL) {
    // Set each of the models to default sequentially
    for(int i=CVF_MOD_0; i<=CVF_MOD_2; i++) {
      SetRAMSettingToDefault(i);
    }
  }
  else if(setting == CVF_CAM_CONFIG) {
    m_vision.m_camera.ResetToDefaults();
  }
  else if(setting >= CVF_MOD_0 && setting <= CVF_MOD_2) {
    int index = setting - CVF_MOD_0;
    CColorModelHSV model;

    // Set model to default
    GetDefaultModel(index, model);
    m_vision.UploadModel(index, model);

    // Set render color to default
    m_vision.SetRenderColor(index, 
			    DV_BUILD_COLOR(def_r[index], 
					   def_g[index], 
					   def_b[index]));
  }
  else {
    return(false);
  }
  return(true);
}
void 
CVisionFlashHandler::SetRAMToDefault() 
{
  printf("Set Render Colors to Default\n");
  for (int i=0; i<DV_MODELS; i++) {
    m_vision.SetRenderColor(i, 
			    DV_BUILD_COLOR(def_r[i], def_g[i], def_b[i]));
  }
  printf("Restore Models to Default\n");
  CColorModelHSV model;
  for(int i=0; i<DV_MODELS; i++) {
    GetDefaultModel(i, model);
    m_vision.UploadModel(i, model);
  }

  printf("Reset Camera Vals to Default\n");
  m_vision.m_camera.ResetToDefaults();

  printf("  Done\n");
}


// /////////////////////////////////////////////////////////////////////
// // Vision Model
// CVisionCameraHandler::CVisionCameraHandler(CVision &vision) :
//   m_vision(vision)
// {
// }

// CVisionCameraHandler::~CVisionCameraHandler()
// {
// }

// void 
// CVisionCameraHandler::PopulateMenu(CMenuElement &menuElt) 
// {
//   CMenuElement *subElem = menuElt.AddMenuLink("Camera", this);
//   assert(subElem!=NULL);
//   subElem->AddMenuItem("Calibrate White", 0);
//   subElem->AddMenuItem("Enable Auto White Balance", 1);
//   subElem->AddMenuItem("Disable Auto White Balance", 2);
// }

// bool 
// CVisionCameraHandler::HandleMenuEvent(int eventType, 
// 				     CMenuElement &menu) 
// {
//   DBG(printf("Handling Camera Event %d\n", eventType));

//   switch(eventType) {
//   case 0:
//     // Calibrate White
//     CCameraContext::Push(m_vision);
//     m_vision.m_camera.CalibrateWhite();
//     return(true);
//   case 1:
//     // Enable AWB
//     CCameraContext::Push(m_vision);
//     m_vision.m_camera.SetAWB(true);
//     return(true);
//   case 2:
//     // Disable AWB
//     CCameraContext::Push(m_vision);
//     m_vision.m_camera.SetAWB(true);
//     return(true);
//   }
//   return(false);
// }

/////////////////////////////////////////////////////////////////////
// Main Vision Menu
CVisionMenuHandler::CVisionMenuHandler(CVision &vision, 
				       const CVisionDispOptions &options) :
  m_vision(vision), m_optHandler(vision, options), 
  m_modelHandler(vision), m_flashHandler(vision)
{
}

void 
CVisionMenuHandler::PopulateMenu(CMenuElement &menuElt)
{
  CMenuElement *subElem = menuElt.AddMenuLink("Vision", this);
  assert(subElem!=NULL);

  subElem->AddMenuItem("Live Video", 0);
  subElem->AddMenuItem("Processed Video", 1);
  subElem->AddMenuItem("Blob Tracking", 2);
  m_optHandler.PopulateMenu(*subElem);
  m_modelHandler.PopulateMenu(*subElem);
  subElem->AddMenuItem("Camera Config", 3);
  m_flashHandler.PopulateMenu(*subElem);
  //m_cameraHandler.PopulateMenu(*subElem);
}
bool 
CVisionMenuHandler::HandleMenuEvent(int eventType, 
				    CMenuElement &menu)
{
  CVisionDispOptions opts;
  m_optHandler.GetOptions(opts);

  switch(eventType) {
  case 0:
    // Live video
    opts.mode = RM_RAW;
    CVisionContext::Push(m_vision, opts);
    break;
  case 1:
    // Processed video
    opts.mode = RM_PROCESSED;
    CVisionContext::Push(m_vision, opts);
    break;
  case 2:
    // Blobs
    opts.showBlobs= true;
    CBlobContext::Push(m_vision.GetInterruptCont(), m_vision, opts);
    break;
  case 3:
    // Camera
    CCameraContext::Push(m_vision);
    break;
  case 4:
    // Camera
    CCameraContext::Push(m_vision);
    break;
  default:
    return(false);
  }
  return(true);
}

