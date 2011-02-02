#ifndef CVISIONMENU_H
#define CVISIONMENU_H

#include "menudisp.h"
#include "vision.h"
#include "visioncontext.h"
#include "colormodel.h"

////////////////////////////////////////////////////////////////////
// Options
class CVisionOptionsHandler : public IMenuHandler
{
public:
  CVisionOptionsHandler(CVision &vision, 
			const CVisionDispOptions &options = 
			CVisionDispOptions());
  virtual ~CVisionOptionsHandler() {}

  void GetOptions(CVisionDispOptions &options) {
    options = m_options;
  }

  //void SetOptions(const CvisionDispOptions &options);
  void RefreshMenuEntries();

  void PopulateMenu(CMenuElement &menuElt);
  virtual bool HandleMenuEvent(int eventType, 
			       CMenuElement &menu);
protected:
  CVision &m_vision;
  CVisionDispOptions m_options;
  CBoolEntry m_enableEnt0, m_enableEnt1, m_enableEnt2;
  CBoolEntry m_showBlobsEnt;
  CBoolEntry m_showBlobTextEnt;
  CIntEntry m_minBlobAreaEnt;
  CBoolEntry m_sortBlobsEnt;
  CBoolEntry m_doBlobAxesEnt;
  CMenuElement *m_menuElt;
};

////////////////////////////////////////////////////////////////////
// Model
class CFlash;

class CVisionModelHandler : public IMenuHandler
{
public:
  CVisionModelHandler(CVision &vision);
  virtual ~CVisionModelHandler();

  // Flash Support
//   bool CheckFlashMagic();
//   void SaveToFlash();
//   void LoadFromFlash();
//   void ClearFlash();

//   // Misc
  void PrintModels();
  void RestoreToDefault();
//   void UpdateSettingEntry();

  // Menu Support
  void PopulateMenu(CMenuElement &menuElt);
  virtual bool HandleMenuEvent(int eventType, 
			       CMenuElement &menu);

protected:
  CVision &m_vision;
//   CFlash *m_flash;
  CColorModelHSV m_models[DV_MODELS];
};

////////////////////////////////////////////////////////////////////
// Flash
class CVisionFlashHandler : public IMenuHandler
{
public:
  enum CVF_Setting {CVF_MOD_ALL=0, 
		    CVF_CAM_CONFIG,
		    CVF_MOD_0, 
		    CVF_MOD_1, 
		    CVF_MOD_2, 
		    CVF_MAX=CVF_MOD_2, 
		    CVF_SETNUM };

  struct CVFSettingInfo {
    const char *name;
    bool  is_model;
    int   modmask;
  };

  // Structure to make dealing with flash easier
  struct ModelFlashStruct // total size: 72 to 75 bytes, definitely less than one 128KiB sector
  {	// 4 bytes
    unsigned int magic;	// 2 bytes
    unsigned short numModels;	// 2 bytes/model * 3 models = 6 bytes
    unsigned short renderColor[DV_MODELS];	// 12 bytes/model * 3 models = 36 bytes
    CColorModelHSV models[DV_MODELS];	// 24 to 27 bytes (not sure on enum in camera.h)
    CCameraSettings camSettings;
  };

public:
  CVisionFlashHandler(CVision &vision);
  virtual ~CVisionFlashHandler();

  void PopulateMenu(CMenuElement &menuElt);
  virtual bool HandleMenuEvent(int eventType, 
			       CMenuElement &menu);

  // Flash Support
  bool CheckFlashMagic();
  void LoadFromFlash(int setting);
  void ClearFlash();

  // Read value from either m_vision or m_camera and write to flash
  bool SaveRAMSettingToFlash(int setting);

  // Save a given model or camera setting structure to flash
  bool SaveModelToFlash(int index, CColorModelHSV &model);
  bool SaveCameraSettingsToFlash(CCameraSettings &settings);

  // Query support
  bool GetSavedModel(unsigned char index, CColorModelHSV &model);
  bool GetDefaultModel(int index, CColorModelHSV &model);
  bool GetSavedCameraConfig(CCameraSettings &settings);

  // Misc
  void ViewValues(int setting);
  void UpdateSettingEntry();

  void SetRAMToDefault();
  bool SetRAMSettingToDefault(int setting);

protected:
  void _LoadFlashToPrep();
  void _SetPrepModel(int index, CColorModelHSV &model);
  void _SetPrepRenderColor(int index, unsigned short color);
  void _SetPrepCameraSettings(CCameraSettings &settings);
  bool _SetPrepSettingFromRAM(int setting);
  bool _WritePrepToFlash();

protected:
  CVision &m_vision;
  CFlash *m_flash;
  
  int m_setSel;
  CMenuItem   *m_setItem;
  CMenuElement *m_menuElt;
  
  // Copy of flash to prepare for writing
  ModelFlashStruct m_prepFlashData;
};

////////////////////////////////////////////////////////////////////
// Camera

// class CVisionCameraHandler : public IMenuHandler
// {
// public:
//   CVisionCameraHandler(CVision &vision);
//   virtual ~CVisionCameraHandler();

//   void SetAWB(bool awb);
//   void CalibrateWhite();

//   // Menu Support
//   void PopulateMenu(CMenuElement &menuElt);
//   virtual bool HandleMenuEvent(int eventType, 
// 			       CMenuElement &menu);
// protected:
//   CVision &m_vision;
// };

////////////////////////////////////////////////////////////////////
// Main Vision Menu
class CVisionMenuHandler : public IMenuHandler
{
public:
  CVisionMenuHandler(CVision &vision, 
		     const CVisionDispOptions &options = 
		     CVisionDispOptions());
  virtual ~CVisionMenuHandler() {}
  void PopulateMenu(CMenuElement &menuElt);
  virtual bool HandleMenuEvent(int eventType, 
			       CMenuElement &menu);

public:
  CVision &m_vision;
  CVisionOptionsHandler m_optHandler;
  CVisionModelHandler m_modelHandler;
  CVisionFlashHandler m_flashHandler;
};

#endif
