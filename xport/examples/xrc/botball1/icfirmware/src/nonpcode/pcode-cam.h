#ifndef PCODE_CAM_H
#define PCODE_CAM_H

//#include "nonpcode.h"

//class PcodeCam: public NonPcode
//{
	///////////////////////////////////////////////////////////////////
	// Tracking control
	void init_camera()
	{
		callml1(1000, 0);
	}

	void track_update()
	{
		callml1(1001, 0);
	}

	long track_get_frame()
	{
		unsigned short ret;
		callml1(1007, 0, &ret);
		return ret;
	}

	int track_is_new_data_available()
	{
		return(callml1(1024, 0));
	}

	///////////////////////////////////////////////////////////////////
	// Normal stats
	int track_count(int ch)
	{
		return(callml1(1002, ch));
	}

	int track_size(int ch, int i)
	{
		return(callml2(2003, ch, i));
	}
	int track_x(int ch, int i)
	{
		return(callml2(2004, ch, i));
	}
	int track_y(int ch, int i)
	{
		return(callml2(2005, ch, i));
	}
	int track_confidence(int ch, int i)
	{
		return(callml2(2006, ch, i));
	}

	// Bounding box stats
	int track_bbox_top(int ch, int i)
	{
		return(callml2(2007, ch, i));
	}
	int track_bbox_bottom(int ch, int i)
	{
		return(callml2(2008, ch, i));
	}
	int track_bbox_left(int ch, int i)
	{
		return(callml2(2009, ch, i));
	}
	int track_bbox_right(int ch, int i)
	{
		return(callml2(2010, ch, i));
	}
	int track_bbox_width(int ch, int i)
	{
		return(callml2(2011, ch, i));
	}
	int track_bbox_height(int ch, int i)
	{
		return(callml2(2012, ch, i));
	}
	int track_bbox_area(int ch, int i)
	{
		return(callml2(2013, ch, i));
	}

	///////////////////////////////////////////////////////////////////
	// Orientation stats
	float track_angle(int ch, int i)
	{
		float ret;
		callml3(3010, ch, i, 0, &ret);
		return(ret);
	}

	float track_major_axis(int ch, int i)
	{
		float ret;
		callml3(3011, ch, i, 0, &ret);
		return(ret);
	}

	float track_minor_axis(int ch, int i)
	{
		float ret;
		callml3(3012, ch, i, 0,&ret);
		return(ret);
	}

	///////////////////////////////////////////////////////////////////
	// Set/Get preferences

	#define TRACK_MINAREA_INDEX  0
	#define TRACK_ENABLE_CH0_INDEX  1
	#define TRACK_ENABLE_CH1_INDEX   2
	#define TRACK_ENABLE_CH2_INDEX   3

	#define TRACK_RENDER_COLOR_CH0_INDEX 4
	#define TRACK_RENDER_COLOR_CH1_INDEX 5
	#define TRACK_RENDER_COLOR_CH2_INDEX 6
	#define TRACK_ENABLE_ORIENT_INDEX 7

	// Orientation
	void track_enable_orientation()
	{
		_track_set_pref_value(TRACK_ENABLE_ORIENT_INDEX, 1);
		// Formerly: callml(1020, 1);
	}

	void track_disable_orientation()
	{
		_track_set_pref_value(TRACK_ENABLE_ORIENT_INDEX, 0);
		// Formerly: callml(1020, 0);
	}

	void track_set_orientation_enable(int val)
	{
		_track_set_pref_value(TRACK_ENABLE_ORIENT_INDEX, val);
	}

	int track_orientation_enabled()
	{
		return _track_get_pref_value(TRACK_ENABLE_ORIENT_INDEX);
		// Formerly: return(callml(1021, 0));
	}

	// Minimum blob area
	void track_set_minarea(int minarea)
	{
		_track_set_pref_value(TRACK_MINAREA_INDEX, minarea);
		// Formerly: callml(1022, minarea);
	}

	int track_get_minarea()
	{
		return(_track_get_pref_value(TRACK_MINAREA_INDEX));
		// Formerly: return(callml(1023, 0));
	}

	// Channel enable
	void track_set_ch_enable(int ch, int val) 
	{
		if(ch<0 || ch>2) return;
		_track_set_pref_value(TRACK_ENABLE_CH0_INDEX+ch, val);
	} 
	int track_get_ch_enable(int ch) 
	{
		if(ch<0 || ch>2) return -1;
		return(_track_get_pref_value(TRACK_ENABLE_CH0_INDEX+ch));
	} 

	////////////////////////////////////////////
	// Internal functions to support tracking preferences
	int _track_get_pref_value(int argtype)
	{
		return(callml1(1023, argtype));
	}
	int _track_set_pref_value(int argtype, int val)
	{
		return(callml2(2024, argtype, val));
	}

	///////////////////////////////////////////////////////////////////
	// Color model query/set support
	#define MAX_H 360
	#define MAX_S 224
	#define MAX_V 224

	#define HMIN_INDEX 0
	#define HMAX_INDEX 1

	#define SMIN_INDEX 2
	#define SMAX_INDEX 3

	#define VMIN_INDEX 4
	#define VMAX_INDEX 5

	#define MODEL_PARTS 32
	#define MODEL_CHUNK_SIZE 4

	//////////////////////////////////////////////////////////////////
	// Official API's from Dave:

	// where model_num is the model number 0,1, or 2 model is an int array
	// of length 4 {hue_low, hue_hi, saturation, value}.  
	// Returns 0 on success, -1 on failure

	// In order to use this from the Interaction window you need to do 
	// something like the following:
	//   {int model[]={0, 100, 200, 200}; color_set_model(0,model);}

	int color_set_model(int model_num, int model[]) 
	{
		if(_array_size(model)!=4) {
			return(-1);
		}
		return(color_set_ram_model_advanced(model_num, 
											model[0], model[1], 
											model[2], MAX_S, 
											model[3], MAX_V));
	}

	// parameters are as before
	// In order to use this from the Interaction window you need to do 
	// something like the following:
	//   {int model[4]; color_get_model(0, model); printf("H=(%d->%d), S>=%d, V>=%d\n", model[0], model[1], model[2], model[3]);}

	int color_get_model(int model_num, int model[]) 
	{
		if(_array_size(model)!=4) {
			return(-1);
		}
		model[0] = _color_get_ram_value(model_num, HMIN_INDEX);
		model[1] = _color_get_ram_value(model_num, HMAX_INDEX);
		model[2] = _color_get_ram_value(model_num, SMIN_INDEX);
		model[3] = _color_get_ram_value(model_num, VMIN_INDEX);
		return(0);
	}

	//////////////////////////////////////////////////////////////////
	// APIs Added by Anne:

	////////////////////////////////////////////
	// Get/Set currently active color models in RAM
	//   Get Hue
	int color_get_ram_hmin(int ch)
	{
		return(_color_get_ram_value(ch, HMIN_INDEX));
	}
	int color_get_ram_hmax(int ch)
	{
		return(_color_get_ram_value(ch, HMAX_INDEX));
	}

	//   Get Saturation
	int color_get_ram_smin(int ch)
	{
		return(_color_get_ram_value(ch, SMIN_INDEX));
	}
	int color_get_ram_smax(int ch)
	{
		return(_color_get_ram_value(ch, SMAX_INDEX));
	}

	//   Get Value
	int color_get_ram_vmin(int ch)
	{
		return(_color_get_ram_value(ch, VMIN_INDEX));
	}

	int color_get_ram_vmax(int ch)
	{
		return(_color_get_ram_value(ch, VMAX_INDEX));
	}

	// Set model parameters compatible with UI, always uses maximum values
	// for max saturation and value so that the rainbow is always included
	int color_set_ram_model(int ch, int hmin, int hmax, int smin, int vmin)
	{
		return(color_set_ram_model_advanced(ch, hmin, hmax, smin, MAX_S, 
											vmin, MAX_V));
	}

	// Set model parameters advanced, allows max saturation and value to be 
	// set to be less than the maximum and exclude the rainbow.  
	// USE WITH GREAT CAUTION!
	int color_set_ram_model_advanced(int ch, int hmin, int hmax, 
									int smin, int smax, 
									int vmin, int vmax)
	{
	    
		int i;
	    
		_color_reset_prep_model(ch);
		_color_prep_model_value(ch, HMIN_INDEX, hmin);
		_color_prep_model_value(ch, HMAX_INDEX, hmax);
		_color_prep_model_value(ch, SMIN_INDEX, smin);
		_color_prep_model_value(ch, SMAX_INDEX, smax);
		_color_prep_model_value(ch, VMIN_INDEX, vmin);
		_color_prep_model_value(ch, VMAX_INDEX, vmax);
	    
		if(_color_check_prep_model(ch)==-1) {
			// Not a valid model, abort
			return(-1);
		}
	    
		//    printf("Writing model, ch %d:\n", ch);
		for(i=0; i<MODEL_PARTS/MODEL_CHUNK_SIZE; i++) {
			int start = i*MODEL_CHUNK_SIZE;
			int end =  (i+1)*MODEL_CHUNK_SIZE;
			//        printf("  %d: %d -> %d\n", i, start, end);
			_color_apply_model_partial(ch, start, end);
		}
		//    _color_view_prep_model(ch);
	    
		// Inform the firmware that the model for this channel is done
		// being updated
		_color_wrapup_model_update(ch);
	    
		// Success!
		return(1);
	}

	////////////////////////////////////////////
	// Get saved color models from flash
	//   Get Hue
	int color_get_flash_hmin(int ch)
	{
		return(_color_get_flash_value(ch, HMIN_INDEX));
	}
	int color_get_flash_hmax(int ch)
	{
		return(_color_get_flash_value(ch, HMAX_INDEX));
	}

	//   Get Saturation
	int color_get_flash_smin(int ch)
	{
		return(_color_get_flash_value(ch, SMIN_INDEX));
	}
	int color_get_flash_smax(int ch)
	{
		return(_color_get_flash_value(ch, SMAX_INDEX));
	}

	//   Get Value
	int color_get_flash_vmin(int ch)
	{
		return(_color_get_flash_value(ch, VMIN_INDEX));
	}

	int color_get_flash_vmax(int ch)
	{
		return(_color_get_flash_value(ch, VMAX_INDEX));
	}

	int color_load_flash_model(int ch)
	{
		int i;
	    
		if(_color_set_prep_model_from_flash(ch) == -1) {
			return(-1);
		}
	    
		if(_color_check_prep_model(ch)==-1) {
			// Not a valid model, abort
			return(-1);
		}
	    
		//    printf("Writing model, ch %d:\n", ch);
		for(i=0; i<MODEL_PARTS/MODEL_CHUNK_SIZE; i++) {
			int start = i*MODEL_CHUNK_SIZE;
			int end =  (i+1)*MODEL_CHUNK_SIZE;
			//        printf("  %d: %d -> %d\n", i, start, end);
			_color_apply_model_partial(ch, start, end);
		}
		//    _color_view_prep_model(ch);
	    
		// Inform the firmware that the model for this channel is done
		// being updated
		_color_wrapup_model_update(ch);
	    
		// Success!
		return(1);
	}

	////////////////////////////////////////////
	// Internal functions to support color model query/modification
	int _color_get_ram_value(int ch, int argtype)
	{
		return(callml2(2030, ch, argtype));
	}
	int _color_get_flash_value(int ch, int argtype)
	{
		return(callml2(2031, ch, argtype));
	}

	void _color_reset_prep_model(int ch)
	{
		callml1(1031, ch);
	}

	int _color_check_prep_model(int ch)
	{
		return(callml1(1032, ch));
	}

	void _color_view_prep_model(int ch)
	{
		callml1(1033, ch);
	}

	int _color_prep_model_value(int ch, int argtype, int val)
	{
		return(callml3(3020, ch, argtype, val));
	}
	int _color_set_prep_model_from_flash(int ch)
	{
		return(callml1(1035, ch));
	}
	int _color_apply_model_partial(int ch, int partBegin, int partEnd)
	{
		return(callml3(3021, ch, partBegin, partEnd));
	}
	void _color_wrapup_model_update(int ch)
	{
		// This lets the firmware know that the color model update is done
		// so it can wrapup anything that might need to know that the model 
		// has changed
		callml1(1034, ch);
	}

	///////////////////////////////////////////////////////////////////
	// Display
	void track_show_display(int show_processed, int frameskip, int channel_mask)
	{
		callml3(3030, show_processed, frameskip, channel_mask);
	}

	///////////////////////////////////////////////////////////////////
	// Camera settings query/modification

	#define CAM_TEMP_RED_INDEX  0
	#define CAM_TEMP_BLUE_INDEX  1
	#define CAM_AWB_INDEX   2

	#define CAM_SATURATION_INDEX  3
	#define CAM_CONTRAST_INDEX  4
	#define CAM_BRIGHTNESS_INDEX  5
	#define CAM_MIRROR_INDEX  6

	#define CAM_AGC_ENABLE_INDEX 7 // Auto Gain Control: 1=enabled, 0=disabled
	#define CAM_GAIN_INDEX  8 // Gain: 6-bit value

	#define CAM_AEC_ENABLE_INDEX 9 // Auto Exposure Control: 1=enabled
	#define CAM_EXPOSURE_INDEX 10 // Exposure time, read only

	#define CAM_EXP_RL_INDEX  11 // Exposure Reference Level
	#define CAM_AEC_RATIO_INDEX  12 // Ratio of AEW/AEB, controls AEC behavior

	#define CAM_SHARP_INDEX  13 // Sharpness, 4-bit value
	#define CAM_SHARP_THRESH_INDEX  14 // Sharpness threshold, 4-bit value

	#define CAM_MAXINDEX 14

	//////////////////////////////////////////////////////////////////
	// Official API's from Dave:

	// returns 0 if AWB is off and 1 if AWB is on
	int camera_get_awb() 
	{
		return(_camera_get_param_value(CAM_AWB_INDEX));
	}

	// turns AWB off (enable=0) or on (enable=1)
	int camera_set_awb(int enable) 
	{
		return(_camera_set_param_value(CAM_AWB_INDEX, enable));
	}

	// where color is an int array of length 2 and color[0] is red and
	// color[1] is blue note that color_temp has dual meaning as both
	// color temperature, and that this is a temporary setting as a side
	// effect this function should turn off AWB.  Returns 0 on success, -1
	// on failure

	// In order to use this from the Interaction window you need to do 
	// something like the following:
	//   {int color[]={100,200}; camera_set_wb_color_temp(color);}

	int camera_set_wb_color_temp(int color[]) 
	{
		if(_array_size(color)!=2) {
			return(-1);
		}
		return(_camera_set_wb_color_temp(color[0], color[1]));
	}

	// where color is an array whose 0th element will be set to the red
	// value and 1st element set to blue

	// In order to use this from the Interaction window you need to do 
	// something like the following:
	//   {int color[2]; camera_get_wb_color_temp(color); printf("Red=%d, Blue=%d\n", color[0], color[1]);}

	int camera_get_wb_color_temp(int color[]) 
	{
		int red=_camera_get_param_value(CAM_TEMP_RED_INDEX);
		int blue=_camera_get_param_value(CAM_TEMP_BLUE_INDEX);
	    
		if(red == -1 || blue == -1 || _array_size(color)!=2) {
			return(-1);
		}
		else {
			color[0]=red;
			color[1]=blue;
			return(0);
		}
	}

	//////////////////////////////////////////////////////////////////
	// APIs Added by Anne:

	int camera_get_wb_red_temp()
	{
		return(_camera_get_param_value(CAM_TEMP_RED_INDEX));
	}
	int camera_get_wb_blue_temp()
	{
		return(_camera_get_param_value(CAM_TEMP_BLUE_INDEX));
	}

	///////////////////////
	// Auto Exposure Control
	// returns 0 if AEC is off and 1 if AEC is on
	int camera_get_aec() 
	{
		return(_camera_get_param_value(CAM_AEC_ENABLE_INDEX));
	}

	// turns AEC off (enable=0) or on (enable=1)
	int camera_set_aec(int enable) 
	{
		return(_camera_set_param_value(CAM_AEC_ENABLE_INDEX, enable));
	}

	// Get the current camera exposure
	int camera_get_exposure() 
	{
		return(_camera_get_param_value(CAM_EXPOSURE_INDEX));
	}

	int camera_get_exp_ref_level()
	{
		return(_camera_get_param_value(CAM_EXP_RL_INDEX));
	}

	int camera_set_exp_ref_level(int val)
	{
		return(_camera_set_param_value(CAM_EXP_RL_INDEX, val));
	}

	int camera_get_aec_ratio()
	{
		return(_camera_get_param_value(CAM_AEC_RATIO_INDEX));
	}

	int camera_set_aec_ratio(int val)
	{
		return(_camera_set_param_value(CAM_AEC_RATIO_INDEX, val));
	}

	///////////////////////
	// Auto Gain Control
	// returns 0 if AGC is off and 1 if AGC is on
	int camera_get_agc() 
	{
		return(_camera_get_param_value(CAM_AGC_ENABLE_INDEX));
	}

	// turns AGC off (enable=0) or on (enable=1)
	int camera_set_agc(int enable) 
	{
		return(_camera_set_param_value(CAM_AGC_ENABLE_INDEX, enable));
	}

	// Get the current camera gain
	int camera_get_gain() 
	{
		return(_camera_get_param_value(CAM_GAIN_INDEX));
	}

	// Set the current camera gain
	int camera_set_gain(int value) 
	{
		return(_camera_set_param_value(CAM_GAIN_INDEX, value));
	}

	////////////////////////////////////////////
	// Internal functions to support camera setting query/modification
	int _camera_get_param_value(int argtype)
	{
		return(callml1(1040, argtype));
	}
	int _camera_set_param_value(int argtype, int val)
	{
		return(callml2(2041, argtype, val));
	}
	void _camera_view_params()
	{
		callml1(1042, 0);
	}
	int _camera_set_wb_color_temp(int red, int blue)
	{
		return(callml2(2043, red, blue));
	}
//};
#endif
