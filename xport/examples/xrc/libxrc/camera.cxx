#include "camera.h"
#include <textdisp.h>

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

////////////////////////////////////////////////////////////////////////
// CCameraSettings

// Record setting default values in an array indexed according to the
// INDEX defines
int cs_defvals[CAM_MAXINDEX+1] = 
    {
	CAM_TEMP_RED_DEFVAL, 
	CAM_TEMP_BLUE_DEFVAL, 
	CAM_AWB_DEFVAL, 

	CAM_SATURATION_DEFVAL, 
	CAM_CONTRAST_DEFVAL, 
	CAM_BRIGHTNESS_DEFVAL, 
	CAM_MIRROR_DEFVAL, 

	CAM_AGC_ENABLE_DEFVAL, 
	CAM_GAIN_DEFVAL, 

	CAM_AEC_ENABLE_DEFVAL, 
	CAM_EXPOSURE_DEFVAL, 

	CAM_EXP_RL_DEFVAL, 
	CAM_AEC_RATIO_DEFVAL, 

	CAM_SHARP_DEFVAL, 
	CAM_SHARP_THRESH_DEFVAL
    };


CCameraSettings::CCameraSettings() : version(CAMSET_CURR_VERSION), 
				     awbEnabled(CAM_AWB_DEFVAL), 
				     mirrorEnabled(CAM_MIRROR_DEFVAL), 
				     wbRed(CAM_TEMP_RED_DEFVAL), 
				     wbBlue(CAM_TEMP_BLUE_DEFVAL), 
				     saturation(CAM_SATURATION_DEFVAL), 
				     contrast(CAM_CONTRAST_DEFVAL), 
				     brightness(CAM_BRIGHTNESS_DEFVAL), 
				     
				     agcEnabled(CAM_AGC_ENABLE_DEFVAL), 
				     gain(CAM_GAIN_DEFVAL),
				     aecEnabled(CAM_AEC_ENABLE_DEFVAL),
				     exposure(CAM_EXPOSURE_DEFVAL),
				     expRefLevel(CAM_EXP_RL_DEFVAL), 
				     aecRatio(CAM_AEC_RATIO_DEFVAL),
				     sharpness(CAM_SHARP_DEFVAL),
				     sharpnessThresh(CAM_SHARP_THRESH_DEFVAL),
				     
				     dataSource(CSET_DEFAULT)
{}

const char *CCameraSettings::GetIndexName(int index) 
{
    switch(index) {
    case CAM_TEMP_RED_INDEX: return("Red");
    case CAM_TEMP_BLUE_INDEX: return("Blue");
    case CAM_AWB_INDEX: return("AWB");
    case CAM_SATURATION_INDEX: return("Sat"); 
    case CAM_CONTRAST_INDEX: return("Cont");
    case CAM_BRIGHTNESS_INDEX: return("Bright");
    case CAM_MIRROR_INDEX: return("Mirror");

    case CAM_AGC_ENABLE_INDEX: return("AGC");
    case CAM_GAIN_INDEX: return("Gain");

    case CAM_AEC_ENABLE_INDEX: return("AEC");
    case CAM_EXPOSURE_INDEX: return("Exp");

    case CAM_EXP_RL_INDEX: return("ExpRL"); 
    case CAM_AEC_RATIO_INDEX: return("AERatio");

    case CAM_SHARP_INDEX: return("Sharp");
    case CAM_SHARP_THRESH_INDEX: return("ShThresh");

    default:
	return("BAD");
    }
}

int CCameraSettings::GetIndexDefval(int index) 
{
    if(index<0 || index>CAM_MAXINDEX) {
	return(-1);
    }
    else {
	return(cs_defvals[index]);
    }
}

int CCameraSettings::GetValue(int index) 
{
    switch(index) {
    case CAM_TEMP_RED_INDEX: return(wbRed);
    case CAM_TEMP_BLUE_INDEX: return(wbBlue);
    case CAM_AWB_INDEX: return(awbEnabled);
    case CAM_SATURATION_INDEX: return(saturation); 
    case CAM_CONTRAST_INDEX: return(contrast);
    case CAM_BRIGHTNESS_INDEX: return(brightness);
    case CAM_MIRROR_INDEX: return(mirrorEnabled);

    case CAM_AGC_ENABLE_INDEX: return(agcEnabled);
    case CAM_GAIN_INDEX: return(gain);

    case CAM_AEC_ENABLE_INDEX: return(aecEnabled);
    case CAM_EXPOSURE_INDEX: return(exposure);

    case CAM_EXP_RL_INDEX: return(expRefLevel); 
    case CAM_AEC_RATIO_INDEX: return(aecRatio);

    case CAM_SHARP_INDEX: return(sharpness);
    case CAM_SHARP_THRESH_INDEX: return(sharpnessThresh);

    default:
	return(-1);
    }
}

bool CCameraSettings::SetValue(int index, short val) 
{
    switch(index) {
    case CAM_TEMP_RED_INDEX: wbRed = val; break;
    case CAM_TEMP_BLUE_INDEX: wbBlue = val; break;
    case CAM_AWB_INDEX: awbEnabled = val; break;
    case CAM_SATURATION_INDEX: saturation = val; break; 
    case CAM_CONTRAST_INDEX: contrast = val; break;
    case CAM_BRIGHTNESS_INDEX: brightness = val; break;
    case CAM_MIRROR_INDEX: mirrorEnabled = val; break;

    case CAM_AGC_ENABLE_INDEX: agcEnabled = val; break;
    case CAM_GAIN_INDEX: gain = val; break;

    case CAM_AEC_ENABLE_INDEX: aecEnabled = val; break;
    case CAM_EXPOSURE_INDEX: exposure = val; break;

    case CAM_EXP_RL_INDEX: expRefLevel = val; break; 
    case CAM_AEC_RATIO_INDEX: aecRatio = val; break;

    case CAM_SHARP_INDEX: sharpness = val; break;
    case CAM_SHARP_THRESH_INDEX: sharpnessThresh = val; break;
    default:
	return(false);
    }
    dataSource = CSET_MOD;
    return(true);
}


////////////////////////////////////////////////////////////////////////
// CCamera
CCamera::CCamera(unsigned long baseAddr) : m_sccb(baseAddr, DC_SCCB_ADDR), 
					   m_connected(false), 
					   m_settings(), 
					   m_modCount(0)
	{
	Initialize();
	}

int CCamera::Initialize()
	{
	unsigned char val;

	// check manufactuerer ID (see OV6620 datasheet)
	m_connected = m_sccb.Read(0x1d)==0xa2;

	if (!m_connected)
		return -1;

	m_sccb.Write(0x12, 0x80); // reset camera
	while(1)
		{
		val = m_sccb.Read(0x12);
		if (!(val&0x80))
			break;
		}

	// set camera to RGB, etc
	m_sccb.Write(0x28, 0x05); // set raw RGB mode
	m_sccb.Write(0x12, 0x2c); // set raw RGB mode

	// turn off auto white balanced
	SetAWB(false);
	return 0;
	}

bool CCamera::Connected()
	{
	return m_connected;
	}


int CCamera::SetAWB(bool enable)
	{
	unsigned char val;

	if (!m_connected)
		return -1;
	val = m_sccb.Read(0x12);
	if (enable)
		val |= 0x04;
	else
		val &= ~0x04;

	m_sccb.Write(0x12, val);
	m_settings.awbEnabled = enable;

	// Increment m_modCount to indicate previous CCameraSettings
	// no longer up-to-date
	m_modCount++;

	return 0;
	}

int CCamera::CalibrateWhite()
	{
	unsigned short blueCount=0, redCount=0;
	unsigned char blue, red;
	unsigned char lastBlue=0, lastRed=0;

	if (!m_connected)
		return -1;

	// turn on auto white balance
	SetAWB(true);

	// wait for gains to settle
	while(1)
		{
		blue = m_sccb.Read(0x01);	
		red = m_sccb.Read(0x02);

		if (lastBlue==blue)
			blueCount++;
		else
			blueCount = 0;

		if (lastRed==red)
			redCount++;
		else
			redCount = 0;

		if (blueCount>DC_COLOR_CONSISTENCY && redCount>DC_COLOR_CONSISTENCY)
			break;

		lastBlue = blue;
		lastRed = red;
		}

	// freeze values
	SetAWB(false);
	m_settings.wbBlue = blue;
	m_settings.wbRed = red;
	m_settings.awbEnabled = false;

	return 0;
	}

int CCamera::SetMirror(bool mirror)
	{
	unsigned char val;

	if (!m_connected)
		return -1;
	val = m_sccb.Read(0x12);
	if (mirror)
		val |= 0x40;
	else
		val &= ~0x40;

	m_sccb.Write(0x12, val);
	m_settings.mirrorEnabled = mirror;
	
	// Increment m_modCount to indicate previous CCameraSettings
	// no longer up-to-date
	m_modCount++;

	return 0;
	}

int  CCamera::SetWhiteBalance(unsigned char blue, unsigned char red)
	{
	if (!m_connected)
		return -1;
	m_sccb.Write(0x01, blue);
	m_sccb.Write(0x02, red);
	m_settings.wbBlue = blue;
	m_settings.wbRed = red;

	// Increment m_modCount to indicate previous CCameraSettings
	// no longer up-to-date
	m_modCount++;

	return 0;
	}

int CCamera::SetSaturation(unsigned char val)
	{
	if (!m_connected)
		return -1;
	m_sccb.Write(0x03, val);
	m_settings.saturation = val;

	// Increment m_modCount to indicate previous CCameraSettings
	// no longer up-to-date
	m_modCount++;

	return 0;
	}

int CCamera::SetContrast(unsigned char val)
	{
	if (!m_connected)
		return -1;
	m_sccb.Write(0x05, val);
	m_settings.contrast = val;

	// Increment m_modCount to indicate previous CCameraSettings
	// no longer up-to-date
	m_modCount++;

	return 0;
	}

int CCamera::SetBrightness(unsigned char val)
	{
	if (!m_connected)
		return -1;
	m_sccb.Write(0x06, val);
	m_settings.brightness = val;

	// Increment m_modCount to indicate previous CCameraSettings
	// no longer up-to-date
	m_modCount++;

	return 0;
	}

/////////////////////////////////////////////////////////////////
// Settings added by Anne Wright 12/31/05
int CCamera::SetAGCEnable(bool enable)
{
    // AGC is controlled by COMA (Reg 0x12), bit 5
    unsigned char val;

    if (!m_connected)
	return -1;

    val = m_sccb.Read(0x12);
    if (enable)
	val |= 0x20;
    else
	val &= ~0x20;

    m_sccb.Write(0x12, val);
    m_settings.agcEnabled = enable;

    // Increment m_modCount to indicate previous CCameraSettings
    // no longer up-to-date
    m_modCount++;

    return 0;
}

int CCamera::SetGain(unsigned char val)
{
    if (!m_connected)
	return -1;

    // If AGC is enabled, disable it
    SetValueIfDifferent(CAM_AGC_ENABLE_INDEX, 0);
    
    // Gain is a 6-bit number in Reg 0x00.  Shift val right by two bits
    // and drop the 2 LSBs in order to map to values between 0 and
    // 255.

    // Write the clamped value to , the Gain register
    m_sccb.Write(0x00, val>>2);

    // Record the gain in m_settings
    m_settings.gain = val;

    // Increment m_modCount to indicate previous CCameraSettings
    // no longer up-to-date
    m_modCount++;

    return 0;
}

int CCamera::SetAECEnable(bool enable)
{
    // AGC is DISABLED by COMI (Reg 0x29), bit 7: 
    //     0 enables AEC, 1 disables AEC)
    unsigned char val;

    if (!m_connected)
	return -1;

    val = m_sccb.Read(0x29);
    if (enable)
	val &= ~0x80;
    else
	val |= 0x80;

    m_sccb.Write(0x29, val);
    m_settings.aecEnabled = enable;

    // Increment m_modCount to indicate previous CCameraSettings
    // no longer up-to-date
    m_modCount++;

    return 0;
}

int CCamera::SetExposureRefLevel(unsigned char val)
{
    if (!m_connected)
	return -1;

    // ARL is a 3-bit number in bits [7:5] of Reg 0x4E.  
    // Strip off the least significant 5 bits before writing
    val &= 0xe0;
    m_sccb.Write(0x4E, val);

    // Record the exposure reference level in m_settings
    m_settings.expRefLevel = val;

    // Increment m_modCount to indicate previous CCameraSettings
    // no longer up-to-date
    m_modCount++;

    return 0;
}

int CCamera::SetAECRatio(unsigned char val)
{
    if (!m_connected)
	return -1;

    // AE Ratio is AEW (Reg 0x24)/AEB (Reg 0x25), constraints are: 
    //  -  AReg 0x24 + Reg 0x25 >= 0xCA
    //  -  Range is 0x01 - 0xCA
    // Target 0x24 + 0x25 = 0xCA and map val as follows:
    //    0   => 0x01/0xC9
    //    255 => 0xC9/0x01

    // Algorithm by Randy Sargent
    //  - $val is percent of "bright" pixels
    //  - 0 = 0%
    //  - 255 = 100%
    //  - Valid range is actually 1-254
    unsigned char AEW = ((val*0xCA + 0xCA/2) / 255);
    if (AEW < 1) { AEW= 1; }
    if (AEW > 0xC9) { AEW= 0xC9; }
    unsigned char AEB = 0xCA - AEW;

#ifdef DEBUG
    int percent_white= AEW/(AEW+AEB);
    int inverse= _CalcAECRatio(AEW, AEB);
    printf("val=%d, AEW=0x%x, AEB=0x%x, sum=0x%x, % white %f, inverse %d, diff=%d\n", 
	   val, AEW, AEB, AEW+AEB, percent_white*100, inverse, val-inverse);
#endif

    m_sccb.Write(0x24, AEW);
    m_sccb.Write(0x25, AEB);

    // Record the exposure reference level in m_settings
    m_settings.expRefLevel = val;

    // Increment m_modCount to indicate previous CCameraSettings
    // no longer up-to-date
    m_modCount++;

    return 0;
}

int CCamera::SetSharpness(unsigned char val)
{
    if (!m_connected)
	return -1;

    // Sharpness is Reg 0x7, bits [3:0].  Range is 0x - 8x, step 0.5x
    // Since it's just a 4-bit value, shift val right by 4 to put it in the 
    // range of 0 - 255 and just ignore the least significant 4 bits
    unsigned char reg;
    reg = m_sccb.Read(0x7);

    // Strip off least 4 bits of reg and replace with val >> 4
    reg = (reg & 0xf0) | (val >> 4);

    m_sccb.Write(0x7, reg);

    // Record the sharpness with the 4 LSBs stripped off in m_settings
    m_settings.sharpness = val & 0xf0;

    // Increment m_modCount to indicate previous CCameraSettings
    // no longer up-to-date
    m_modCount++;

    return 0;
}

int CCamera::SetSharpnessThresh(unsigned char val)
{
    if (!m_connected)
	return -1;

    // Sharpness thresh is Reg 0x7, bits [7:4].  Range is 0 - 80 mV,
    // step 5 mV.  Since it's just a 4-bit value, just ignore the least
    // significant 4 bits
    unsigned char reg;
    reg = m_sccb.Read(0x7);

    // Strip off most significant 4 bits of reg and replace with val & 0xf0
    reg = (reg & 0x0f) | (val & 0xf0);

    m_sccb.Write(0x7, reg);

    // Record the sharpness with the 4 LSBs stripped off in m_settings
    m_settings.sharpnessThresh = val & 0xf0;

    // Increment m_modCount to indicate previous CCameraSettings
    // no longer up-to-date
    m_modCount++;

    return 0;
}

/////////////////////////////////////////////////////////////////
void CCamera::ResetToDefaults()
{
    // Do a soft reset, set camera to RGB, etc.
    Initialize();
    // Turn on AWB (since that's the factory default but Initialize 
    // explicitly disables it)
    SetAWB(true);
    // Read the settings into m_settings
    _ReadSettings();
}

void CCamera::CopyStoredSettings(CCameraSettings &settings) const
{
    // Just copy stored settings from last read and/or set operations
    // for each parameter
    settings = m_settings;
}

int CCamera::ReadSettings(CCameraSettings &settings)
{
    // Read settings from camera into m_settings
    int ret = _ReadSettings();

    // Copy from m_settings to settings
    CopyStoredSettings(settings);

    return(ret);
}

int CCamera::_CalcAECRatio(unsigned char AEW, unsigned char AEB)
{
  // Make sure AEW and AEB are > 0
  if (AEW == 0) { AEW= 1; }
  if (AEB == 0) { AEB= 1; }
  // Calculate aecRatio, valid output range is 1 - 254
  return((AEW*255+255/2)/(AEB+AEW));
}

// Internal version, reads settings into m_settings
int CCamera::_ReadSettings()
{
  // Force a read of the camera registers over I2C and store the results
  if(!m_connected) {
    // In case camera isn't connected, return failure
    return(-1);
  }

  // Read COMA register, address 0x12, which contains various status bits
  unsigned char COMA = m_sccb.Read(0x12);

  // AWB: bit 2 of COMA is AWB bit
  m_settings.awbEnabled = !!(COMA & 0x04);

  // Mirror: bit 6 of COMA is mirror bit
  m_settings.mirrorEnabled = !!(COMA & 0x40);

  // Read color temperature
  m_settings.wbBlue = m_sccb.Read(0x01);	
  m_settings.wbRed = m_sccb.Read(0x02);

  // Read saturation, contrast, and brightness
  m_settings.saturation = m_sccb.Read(0x03);
  m_settings.contrast = m_sccb.Read(0x05);
  m_settings.brightness = m_sccb.Read(0x06);

  // Read gain-related settings
  m_settings.agcEnabled = !!(COMA & 0x20); // Reg 0x12, bit 5
  // If AGC is enabled, read register 0x00 and shift left by 2 since
  // it's a 6-bit number.  If not, leave the existing value in
  // m_settings.gain, which should be the last value written to the
  // camera, alone
  if(m_settings.agcEnabled) {
      m_settings.gain = m_sccb.Read(0x00) << 2; // Reg 0, bits [5:0]
  }

  // Read exposure-related settings
  //   Read COMA register, address 0x29, which contains AEC status bits
  unsigned char COMI = m_sccb.Read(0x29);

  // COMI bit 7:  0 enables AEC, 1 disables AEC
  m_settings.aecEnabled = !(COMI & 0x80); 
  m_settings.exposure = m_sccb.Read(0x10); // Reg 0x10 is AEC register

  // Reg 0x4E bits [7:5] are ARL register, higher values = brighter final
  // stable image.  Lower 5 bits are reserved and must be masked off
  m_settings.expRefLevel = m_sccb.Read(0x4E) & 0xe0;		

  // AE Ratio is Reg 0x24/Reg 0x25, constraints are: 
  //  -  Reg 0x24 + Reg 0x25 >= 0xCA
  //  -  Range is 0x01 - 0xCA
  // Map ratio as 0=0%, 255=100%
  unsigned char AEW = m_sccb.Read(0x24);
  unsigned char AEB = m_sccb.Read(0x25);

  // Calculate aecRatio, valid output range is 1 - 254
  m_settings.aecRatio = _CalcAECRatio(AEW, AEB);

  // Read sharpness settings
  unsigned char sharpReg = m_sccb.Read(0x07);
  m_settings.sharpness = sharpReg & 0x0f;    	// Reg 0x7, bits [3:0]
  m_settings.sharpnessThresh = sharpReg >> 4;	// Reg 0x7, bits [7:4]

  // Record that these settings were read from the camera at this modCount
  m_settings.dataSource = CCameraSettings::CSET_READ;
  m_settings.readModCount = m_modCount;

  return(0);
}

int CCamera::WriteSettings(const CCameraSettings &settings)
{
    if(!m_connected) {
	return(-1);
    }

    if(settings.version < CAMSET_ORIG_VERSION || 
       settings.version > CAMSET_CURR_VERSION) {
	// This version is out of bounds
	printf("WARNING: Ignoring settings, bad version 0x%x\n", 
	       settings.version);
	return(-1);
    }

    DBG(printf("Writing camera settings, version 0x%x:\n", 
	       settings.version));

    // Read current settings from the camera into m_settings
    _ReadSettings();

    // For each setting that is settable, set if different than current value
    int changed=0;

    // Deal with COMA register (0x12) that includes AWB, AGC, and mirror
    unsigned char COMA_orig = m_sccb.Read(0x12);
    unsigned char COMA_new = COMA_orig;

    if (settings.awbEnabled)	// AWB bit = COMA[2]
	COMA_new |= 0x04;
    else
	COMA_new &= ~0x04;

    if (settings.agcEnabled)	// AGC bit = COMA[5]
	COMA_new |= 0x20;
    else
	COMA_new &= ~0x20;

    if (settings.mirrorEnabled)	// MIRR bit = COMA[6]
	COMA_new |= 0x40;
    else
	COMA_new &= ~0x40;
    
    if(COMA_new != COMA_orig) {
	m_sccb.Write(0x12, COMA_new); // Set COMA on camera to new value
	changed=1;
    }
    DBG(printf("  COMA=0x%x (orig=0x%x)\n", COMA_new, COMA_orig));

    // If white balance is not now enabled but either used to be, or 
    // had different red/blue settings, set the color temperature now
    if(!settings.awbEnabled && 
       (m_settings.awbEnabled || 
	settings.wbBlue != m_settings.wbBlue ||
	settings.wbRed != m_settings.wbRed)) {
	SetWhiteBalance(settings.wbBlue, settings.wbRed);
	changed=1;
	DBG(printf("  MWB Red=%d, Blue=%d\n", 
		   settings.wbRed, settings.wbBlue));
    }
    else {
	DBG(printf("  AWB Red=%d, Blue=%d\n", 
	    m_settings.wbRed, m_settings.wbBlue));
    }

    // For other parameters that are settable, set if different
    changed |= SetValueIfDifferent(CAM_SATURATION_INDEX, 
				   settings.saturation);
    changed |= SetValueIfDifferent(CAM_CONTRAST_INDEX, 
				   settings.contrast);
    changed |= SetValueIfDifferent(CAM_BRIGHTNESS_INDEX, 
				   settings.brightness);

    if(settings.version == CAMSET_ORIG_VERSION) {
	// In the original version, none of the other settings 
	// were there yet, so return now.
	if(changed) {
	    // Increment m_modCount to indicate previous CCameraSettings
	    // no longer up-to-date
	    m_modCount++;
	}	    
	DBG(printf("  Old version, done setting\n"));
	return(changed);
    }
    DBG(printf("  New version, setting new parameters\n"));

    /////////////////////////////////////////////////////////////////
    // The following settings were added on 12/31/05 and require
    // version > CAMSET_ORIG_VERSION

    // If AGC is not now enabled but either used to be, or 
    // had different gain settings, set the gain now
    if(!settings.agcEnabled && 
       (m_settings.agcEnabled || 
	settings.gain != m_settings.gain)) {
	SetGain(settings.gain);
	changed=1;
    }

    // For other parameters that are settable, set if different
    changed |= SetValueIfDifferent(CAM_AEC_ENABLE_INDEX, 
				   settings.aecEnabled);

    changed |= SetValueIfDifferent(CAM_EXP_RL_INDEX, 
				   settings.expRefLevel);
    changed |= SetValueIfDifferent(CAM_AEC_RATIO_INDEX, 
				   settings.aecRatio);
    changed |= SetValueIfDifferent(CAM_SHARP_INDEX, 
				   settings.sharpness);
    changed |= SetValueIfDifferent(CAM_SHARP_THRESH_INDEX, 
				   settings.sharpnessThresh);

    DBG(printf("  Done, changed=%d\n", changed));
    return(changed);
}

//////////////////////////////////////////////////////////////////////
// Support indexed parameters

// set an indexed parameter.  Only works for some parameters.
bool CCamera::SetValue(int index, short val) 
{
    switch(index) {
    case CAM_AWB_INDEX: 		SetAWB(val); break;

    case CAM_TEMP_RED_INDEX:
    case CAM_TEMP_BLUE_INDEX: {
	// For color temperature, special case to turn AWB off if it
	// isn't already and to use the other parameter from camera
	// settings and just change the one passed in

	// Set the camera AWB to be off
	if(SetValueIfDifferent(CAM_AWB_INDEX, false)==-1) {
	    // Failed to read values from camera
	    return(false);
	}

	// Read the most up-to-date settings into m_settings
	if(_ReadSettings()==-1) {
	    // Failed to read values from camera
	    return(false);
	}
	// Change the slot for the argtype we're changing in the
	// m_settings structure to match the value passed in
	m_settings.SetValue(index, val);

	// Write the settings to the camera
	if(SetWhiteBalance(m_settings.wbBlue, m_settings.wbRed)==-1) {
	    return(false);
	}
	break;
    }

    case CAM_SATURATION_INDEX: 	SetSaturation(val); break; 
    case CAM_CONTRAST_INDEX: 	SetContrast(val); break;
    case CAM_BRIGHTNESS_INDEX: 	SetBrightness(val); break;
    case CAM_MIRROR_INDEX: 		SetMirror(val); break;

    case CAM_AGC_ENABLE_INDEX: 	SetAGCEnable(val); break;
    case CAM_GAIN_INDEX: 		SetGain(val); break;

    case CAM_AEC_ENABLE_INDEX: 	SetAECEnable(val); break;
	// The OV6620 doesn't support setting the exposure directly.
	// Instead, use SetExposureRefLevel or SetAECRatio
	//case CAM_EXPOSURE_INDEX: 	SetExposure(val); break;

    case CAM_EXP_RL_INDEX: 		SetExposureRefLevel(val); break; 
    case CAM_AEC_RATIO_INDEX: 	SetAECRatio(val); break;

    case CAM_SHARP_INDEX: 		SetSharpness(val); break;
    case CAM_SHARP_THRESH_INDEX: 	SetSharpnessThresh(val); break;

    default:
	DBG(printf("FAILED Setting %s=%d\n", 
		   CCameraSettings::GetIndexName(index), 
		   val));
	return(false);
    }
    DBG(printf("Setting %s=%d\n", 
	       CCameraSettings::GetIndexName(index), 
	       val));
    return(true);
}

// This compares the value in m_settings to the value being requested
// for a given value index and calls SetValue only if they differ.
// Returns 0 if no change, 1 if changed, and -1 if error
int CCamera::SetValueIfDifferent(int index, short val) 
{
    if(m_settings.GetValue(index) == val) {
	DBG(printf("Preserving %s=%d\n", 
		   CCameraSettings::GetIndexName(index), 
		   val));
	return(0);
    }
    else {
	// Requesting different value, call SetValue
	if(SetValue(index, val)) 
	    return(1);
	else 
	    return(-1);
    }
}

void CCamera::PrintSettings() 
{
    if(_ReadSettings()==-1) {
	printf("Can't read camera settings!\n");
    }
    printf("Camera settings:\n");
    printf("  %8s\tValue\n", "Param");
    for(int i=0; i<=CAM_MAXINDEX; i++) {
	const char *iname = CCameraSettings::GetIndexName(i);
	printf("  %8s\t%3d\n", iname, m_settings.GetValue(i)); 
    }
}

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//   End:
