#ifndef _CAMERA_H
#define _CAMERA_H

// This class only supports the Omnivision 6620 and 6630

#define DC_SCCB_ADDR				0xc0
#define DC_COLOR_CONSISTENCY		100

#define CAMSET_CURR_VERSION	0x11
#define CAMSET_ORIG_VERSION	0x10

#include "sccb.h"

#define CAM_TEMP_RED_INDEX 	0
#define CAM_TEMP_BLUE_INDEX 	1
#define CAM_AWB_INDEX 		2

#define CAM_SATURATION_INDEX 	3
#define CAM_CONTRAST_INDEX 	4
#define CAM_BRIGHTNESS_INDEX 	5
#define CAM_MIRROR_INDEX 	6

#define CAM_AGC_ENABLE_INDEX	7 // Auto Gain Control: 1=enabled, 0=disabled
#define CAM_GAIN_INDEX		8 // Gain: 6-bit value

#define CAM_AEC_ENABLE_INDEX	9 // Auto Exposure Control: 1=enabled
#define CAM_EXPOSURE_INDEX	10 // Exposure time, read only

#define CAM_EXP_RL_INDEX 	11 // Exposure Reference Level
#define CAM_AEC_RATIO_INDEX 	12 // Ratio of AEW/AEB, controls AEC behavior

#define CAM_SHARP_INDEX 	13 // Sharpness, 4-bit value
#define CAM_SHARP_THRESH_INDEX  14 // Sharpness threshold, 4-bit value

#define CAM_MAXINDEX 14

/////////////////////////////////////////////////////////////////
// Camera setting default values.  See the OV6620 datasheet for details.
// As of 12/31/05 this is available at:
//   http://www.parallax.com/dl/docs/prod/robo/cmucamomnivis.pdf

#define CAM_TEMP_RED_DEFVAL	0x80 // Reg 0x2
#define CAM_TEMP_BLUE_DEFVAL 	0x80 // Reg 0x1
#define CAM_AWB_DEFVAL		1    // Reg 0x12, bit 2

#define CAM_SATURATION_DEFVAL 	0x80 // Reg 0x3
#define CAM_CONTRAST_DEFVAL 	0x48 // Reg 0x5
#define CAM_BRIGHTNESS_DEFVAL 	0x80 // Reg 0x6
#define CAM_MIRROR_DEFVAL 	0    // Reg 0x12, bit 6

#define CAM_AGC_ENABLE_DEFVAL	1    // Reg 0x12, bit 5
#define CAM_GAIN_DEFVAL		0x00 // Reg 0, bits [5:0]

#define CAM_AEC_ENABLE_DEFVAL	1 // Reg 0x29, !bit 7 (bit 7=0 enables AEC, 
				  //                   bit 7=1 disables AEC)
#define CAM_EXPOSURE_DEFVAL	0x9A // Reg 0x10

#define CAM_EXP_RL_DEFVAL 	0xA0 // Reg 0x4E
#define CAM_AEC_RATIO_DEFVAL 	65 // (0x33/0x97) // Reg 0x24/Reg 0x25

#define CAM_SHARP_DEFVAL 	0x6 // Reg 0x7, bits [3:0]
#define CAM_SHARP_THRESH_DEFVAL 0xc // Reg 0x7, bits [7:4]

//////////////////////////////////////////////////////////////////////
// Camera Settings class
class CCameraSettings {
public:
    enum CSetDataSource {CSET_DEFAULT=0, CSET_READ=1, CSET_MOD=2};

public:
    CCameraSettings();

    static const char *GetIndexName(int index);
    static int GetIndexDefval(int index);
    int GetValue(int index);
    bool SetValue(int index, short val);

public:

	// Total size in flash: 23 + the enum whatever (1 to 4, I think)

    // Record a version number in the first slot to allow for fields to
    // allow fields to be added to the end in the future and still be
    // able to safely restored from flash

    // The following fields were part of the original implementation
    // and are included in CAMSET_VERSION 0x10
    int  version;			
    bool awbEnabled;
    bool mirrorEnabled;
    unsigned char wbRed;
    unsigned char wbBlue;
    unsigned char saturation;
    unsigned char contrast;
    unsigned char brightness;

    // The following fields were added on 12/31/05 by Anne and are not 
    // included in CAMSET_VERSION 0x10
    bool agcEnabled;
    unsigned char gain;
  
    bool aecEnabled;
    unsigned char exposure;

    unsigned char expRefLevel;
    unsigned char aecRatio;	// Map ratio as 0=0%, 255=100%
  
    unsigned char sharpness;
    unsigned char sharpnessThresh;

    enum CSetDataSource dataSource;
    unsigned long readModCount;	// If dataSource = CAMSET_READ this
				// records the value of m_modCount in
				// the CCamera structure at the time
				// of read.
};

//////////////////////////////////////////////////////////////////////
// Camera Control class
class CCamera
{
public:
    CCamera(unsigned long baseAddr);
    int  Initialize();

    // return true if camera is connected
    bool Connected();

    // turn on/off auto white balance (turned off by default)
    int  SetAWB(bool awb); 

    // return when white balance has converged -- can take up to 20
    // seconds depending on how far off white the balance is.
    int  CalibrateWhite();

    // set color temperature
    int  SetWhiteBalance(unsigned char blue, unsigned char red);

    // reverse image (left to right)
    int  SetMirror(bool mirror); 

    // set the color saturation -- 0xff implies high saturation, default 0x80
    int  SetSaturation(unsigned char val);

    // set the contrast -- 0xff implies high contrast, default 0x48
    int  SetContrast(unsigned char val);

    // set the brightness -- 0xff implies high brightness, default 0x80
    int  SetBrightness(unsigned char val);

    // set the Auto Gain Control enable: 1 = AGC Enabled, 0 = disabled
    int SetAGCEnable(bool enable);

    // set the gain 
    int  SetGain(unsigned char val);

    // set the Auto Exposure Control enable: 1 = AEC Enabled, 0 = disabled
    int SetAECEnable(bool enable);

    // set the exposure reference level or ratio
    int  SetExposureRefLevel(unsigned char val);
    int  SetAECRatio(unsigned char val);

    // set the sharpness level or threshold
    int  SetSharpness(unsigned char val);
    int  SetSharpnessThresh(unsigned char val);

    // set an indexed parameter.  Only works for some parameters.
    bool SetValue(int index, short val);

    // This compares the value in m_settings to the value being requested
    // for a given value index and calls SetValue only if they differ.
    // Returns 0 if no change, 1 if changed, and -1 if error
    int SetValueIfDifferent(int index, short val);

    // Reset all registers to default values
    void ResetToDefaults();

    // Get/Set the current settings
    void CopyStoredSettings(CCameraSettings &settings) const;
    int ReadSettings(CCameraSettings &settings);
    int WriteSettings(const CCameraSettings &settings);

    // Read and print the current settings
    void PrintSettings();

protected:
    // Internal functions
    int _CalcAECRatio(unsigned char AEW, unsigned char AEB);
    // Read settings from camera into m_settings
    int _ReadSettings();

protected:
    CSccb m_sccb;
    bool m_connected;
    CCameraSettings m_settings;
    unsigned int m_modCount;
};

#endif

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//   End:
