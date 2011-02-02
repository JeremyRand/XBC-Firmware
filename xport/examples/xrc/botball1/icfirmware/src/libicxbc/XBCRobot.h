#ifndef XRCROBOT_H
#define XRCROBOT_H

// For memcpy
#include <string.h>

#ifndef NO_IR_TRACK
	#include "sinlut.h"
#endif

#include "ICRobot.h"
#include "CInterruptContSingleton.h"
#include "CXBCGpio.h"
#include "rcservo.h"
//#include "rcservotraj.h"

// For Blob Pursue
#include "pursue.h"

#ifdef AUDIO_AHH
	#include "ahh.inl"
#endif
#ifdef AUDIO_BLUE
	#include "blue.inl"
#endif
#ifdef AUDIO_HOKEY
	#include "hokey.inl"
#endif
#ifdef AUDIO_MUCHACHA
	#include "muchacha.inl"
#endif
#ifdef AUDIO_OUCH
	#include "ouch.inl"
#endif
#ifdef AUDIO_PRADO
	#include "prado.inl"
#endif
#ifdef AUDIO_FOURBYTESOUND
	#include "fourbytesound.inl"
#endif
#ifdef AUDIO_READY
	#include "ready.inl"
#endif
#ifdef AUDIO_GO
	#include "go.inl"
#endif
#ifdef AUDIO_DEATH
	#include "death2.inl"
#endif
#ifdef AUDIO_INTRO
	#include "intro.inl"
#endif
#ifdef AUDIO_MUNCH
	#include "munch.inl"
#endif
#ifdef AUDIO_SIREN
	#include "siren.inl"
#endif
#ifdef AUDIO_WARNING
	#include "warning.inl"
#endif

#ifndef NO_IR_TRACK
	#include "table_ir_range.h"
#endif


#ifndef NO_VISION
#include "vision.h"
#include "blob.h"
#include "visioncontext.h"

class CVisionMenuHandler;

#define MAX_CH_BLOBS 32
struct SBlobInfo {
  CBlob *blob;
  bool statsValid;
  SMomentStats stats;

  SBlobInfo() : blob(0), statsValid(false), stats() {}
  void SetBlob(CBlob *b) {
    blob=b;
    statsValid=false;
  }
  void DoStats() {
    if(!statsValid) {
      blob->moments.GetStats(stats);
      statsValid=true;
    }
  }
};
#endif

// Support for setting models from IC
#define COLOR_CHECK_UNKNOWN 0
#define COLOR_CHECK_OK 1
#define COLOR_CHECK_BAD -1

class XBCRobot : public ICRobot
{
protected:
#ifndef NO_GPIO
	CXBCGpio m_gpio;
#endif
#ifndef NO_VISION
        CVision *m_vision;
        CVisionMenuHandler *m_vMenu;

        CMultiChannelBlobAssembler m_blobAssm;
        int m_bnum[DV_MODELS];
        SBlobInfo m_binfo[DV_MODELS][MAX_CH_BLOBS];
        CVisionDispOptions m_options;
  
        // Support for setting models from IC
        int m_prep_status[DV_MODELS];
        CColorModelHSV m_prep_model[DV_MODELS];
        bool m_prep_set[DV_MODELS][MODEL_MAXINDEX+1];

		// Blob following, from pickup demo
		CPursue  m_pursue;

#endif
#ifndef NO_ICXVIEW
	ICXView * m_xview;
#endif
	CRCServo m_servo;
public:

	XBCRobot();
	~XBCRobot();

#ifndef NO_VISION
        void SetVision(CVision *vision);
        void SetVisionMenu(CVisionMenuHandler *vMenu);
#endif
	virtual short CallML1Translator(const short& functionIndex, const short& argument1, void * pointer = 0);
	virtual short CallML2Translator(const short& functionIndex, const short& argument1, const short& argument2, void * pointer = 0, void * pointer2 =0 );
	virtual short CallML3Translator(const short& functionIndex, const short& argument1, const short& argument2, const short& argument3, void * pointer = 0);
	virtual void RunRobotLoop();
	virtual bool HandleMenuEvent(int eventType, CMenuElement &menu);
#ifndef NO_GPIO
	virtual unsigned short GetDigital(const unsigned short sensorPort);


	//Encoder functions
	virtual unsigned short SetEncoderPort(unsigned short sensorPort);
	virtual void UnsetEncoderPort(unsigned short sensorPort);
	virtual void ResetEncoderCount(unsigned short sensorPort);
	virtual unsigned int GetEncoderCount(unsigned short sensorPort);
	

	//Sonar functions
	virtual unsigned short SetSonarPort(unsigned short sensorPort);
	virtual void SendSonarRing(unsigned short sensorPort);
	virtual unsigned int GetSonarBlocking(unsigned short sensorPort);
	virtual unsigned int GetSonarTime(unsigned short sensorPort);

	virtual void SetPortDirection(unsigned short sensorPort, unsigned char portDirection);
	virtual void SetPortValue(unsigned short sensorPort, unsigned char portValue);
  virtual int GetPortDirection(unsigned short sensorPort);

#endif
	virtual unsigned char GetServoPosition(unsigned char servo);

#ifndef NO_VISION
        // Support for setting models from IC
	void color_reset_prep_model(int ch);
	void color_view_prep_model(int ch);

        bool color_prep_model_value(int ch, int argtype, int val);
	bool color_set_prep_model_from_flash(int ch);
	int color_check_prep_model(int ch);

	int color_apply_model_partial(int ch, int partBegin, int partEnd);
	void color_wrapup_model_update(int ch);
#endif

	void serial_write_long(short value_icptr);
	void serial_write_int(short value);
	void serial_read_long(short icptr);
	short serial_read_int();

	void play_sound_no_int(int sound_id);
	
	#ifndef NO_IR_TRACK
		// ToDo: deal with RAM regions for the huge arrays.  Might not be necessary, since we're using malloc().
		// ToDo: replace the division/modulus operations where appropriate, for extra speed
		
		// So the IR rangefinder sensor updates every ~38.3ms.  Periodic() executes at 200Hz, or every 5ms.
		// If we run the IRTrack code every 40ms, or every 8th Periodic() execution, we should be close to optimal.
		//#define IR_TRACK_FREQ 8
		
		// VBlank executes at 60Hz, or every 16.7ms.  So running every 33.3ms (every 2nd VBlank) should work.
		#define IR_TRACK_FREQ 2
		short ir_track_counter;
		
		#define IR_TRACK_ARRAY_SIZE 0x1000
		
		bool ir_track_active;
		volatile unsigned short ir_track_index;
		unsigned char ir_track_motor_port;
		unsigned char ir_track_ir_port;
		
		unsigned short ir_track_radius_offset; // If the sensor is not exactly between your wheels
		unsigned short ir_track_max_horizon; // sensor returns mostly noise past a certain distance
		unsigned short ir_track_max_slope; // unused... maybe I'll find a way to use it later
		unsigned short ir_track_max_depth; // Maximum depth difference between front and rear of blob
		unsigned long ir_track_min_angular_width; // really thin blobs are probably just noise
		
		// ToDo: implement GetMotor and GetSensor (and their callmls and IC functions) for IRTrack
		
		unsigned short ir_track_process_index;
		
		long * ir_track_positions;
		short * ir_track_ranges;
		
		// Data for blobs:
		//   start index
		//   end index
		//   closed (bool)
		// ToDo: Make this dynamically allocated
		#define IR_TRACK_MAX_BLOBS 30
		unsigned short ir_track_blob_start[IR_TRACK_MAX_BLOBS];
		unsigned short ir_track_blob_end[IR_TRACK_MAX_BLOBS];
		bool ir_track_blob_started[IR_TRACK_MAX_BLOBS];
		bool ir_track_blob_closed[IR_TRACK_MAX_BLOBS];
		unsigned short ir_track_current_blob;
		
		void Interrupt(unsigned char vector);
		
		void IRTrackStart(short velocity, long delta_pos);
		void IRTrackStop();
		void IRTrackProcess();
		short IRRange(short port);
		short IRRangeFast(short port);
		void IRRangeBenchmark(short port);
		void IRTrackSetMotor(unsigned char port);
		void IRTrackSetSensor(unsigned char port);
		void IRTrack_Display(long ticks_at_0_rad, long ticks_at_pi_rad, long mm_per_screen_height);
		void IRTrackDump();
	#endif
	
	// getpwm log
	short getpwm_logs[4][0x20];
	short getpwm_index;
	
	void Periodic();
	
	double MapRange(double x, double start1, double end1, double start2, double end2);
	long MapRange(long x, long start1, long end1, long start2, long end2);
	long long MapRange(long long x, long long start1, long long end1, long long start2, long long end2);
	
	
};

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 8
//    c-basic-offset: 8
//   End:

#endif
