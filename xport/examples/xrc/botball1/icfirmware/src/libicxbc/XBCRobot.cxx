#include "XBCRobot.h"
#include "CPrintBuffer.h"
#include "textdisp.h"
#include "ICRobotContext.h"
#include "pcodesim.h"
#include "ICVersion.h"
#include "xport.h"
#include "CBluetoothDevice.h"
#include "regbits.h"
#include "gba_image.h"

#ifdef USE_SIM

#include "CNullCommDevice.h"
#else
#include "CUartDevice.h"
#endif

#ifndef NO_VISION
#include "blobcontext.h"
#include "modelcontext.h"
#include "visionmenu.h"
#endif

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

#ifdef USE_OWN_CLASS
#include "nonpcode.h"
#endif

// For the copied ICRobot functions
#define CHK_MNUM(x) if(x<0 || x>= NUM_MOTORS) return(-1);

XBCRobot::XBCRobot()   
: ICRobot(20),
#ifndef NO_VISION
m_vision(0), m_vMenu(0), m_blobAssm(0x7, true), 
m_options(RM_RAW, true, true, false, 100, true),
m_pursue(this, m_vision),
#endif
#ifndef NO_ICXVIEW
m_xview(),
#endif
m_servo(4, 0x9ffd400)
{
	printf("Initializing XBCRobot\n");

#ifndef NO_ICXVIEW
	m_xview = new ICXView();
#endif

	printf("Initializing Servos\n");
	m_servo.Disable();
	m_servo.SetBounds(0, 0, 255);
	m_servo.SetBounds(1, 0, 255);
	m_servo.SetBounds(2, 0, 255);
	m_servo.SetBounds(3, 0, 255);

#ifndef NO_MOTOR
	//SetRCServoTraj(&m_servo);
#endif

#ifdef USE_SIM
	printf("Initializing Null Comm\n");
	m_commDevice = new CNullCommDevice();
#else
	printf("Initializing Serial Port\n");
	m_commDevice = new CUartDevice();
#endif
#ifndef USE_OWN_CLASS
	setMLTranslator(this);
#else
	printf("Initializing NonPcode\n");
	NonPcode::setMLTranslatorNPC(this);
#endif /* #ifndef USE_OWN_CLASS */

#ifndef NO_VISION
        for(int i=0; i<DV_MODELS; i++) {
            color_reset_prep_model(i);
        }
#endif

#ifndef NO_IR_TRACK
	ir_track_active = false;
	ir_track_index = 0;
	ir_track_motor_port = 0;
	ir_track_ir_port = 0;

	ir_track_radius_offset = 0;
	
	ir_track_max_horizon = 1000; // 1 meter; the sensor only supports out to about 80cm anyway
	ir_track_max_slope = 1000; // This is never used, and no units were established... only initialized for completeness
	ir_track_max_depth = 65; // Pringles Can default
	ir_track_min_angular_width = 0; // don't filter thin blobs by default
	
	ir_track_process_index = 0;
	ir_track_current_blob = 0;

	for(int count=0; count<IR_TRACK_MAX_BLOBS; count++)
	{
		ir_track_blob_start[count] = ir_track_blob_end[count] = 0;
		ir_track_blob_started[count] = ir_track_blob_closed[count] = false;
	}
	
	ir_track_counter = 0;
	
	ir_track_positions = NULL;
	ir_track_ranges = NULL;
#endif

getpwm_index = 0;
}

XBCRobot::~XBCRobot()
{
	if(m_commDevice) delete m_commDevice;
}

void XBCRobot::RunRobotLoop()
{
	// Create an ICRobotContext and push onto the top of the
	// context stack
	HappyBeep();
	HappyBeep();
	HappyBeep();
	HappyBeep();
	HappyBeep();
	ICRobotContext::Push(*this);
	HappyBeep();
	HappyBeep();
	HappyBeep();
	HappyBeep();
	HappyBeep();

	PrintToICBuffer("Welcome to IC\nBy KISS Institute for\nPractical Robotics\nFirmware version %d.%02d\nBitstream Version 0x%4x\nBuilt %s, %s\n", 
			IC_VERSION_MAJOR, IC_VERSION_MINOR, XP_REG_IDENTIFIER,  __DATE__, __TIME__);
	PrintToICBuffer("\nStudent Edition by:\nJeremy Rand\nFahrzin Hemmati\n");
	PrintToICBuffer("\nLicensed to:\n%s", LICENSEE); // removed \n to make it all fit vertically
	while(1)
	{	
		// Process communications and virtual machine
		ICRobot::RunRobotLoop();
/*
#ifdef USE_OWN_CLASS
		ICRobot::HandleMenuEvent(0, *(CMenuElement *)0);
		static NonPcode * m_nonpcode = new NonPcode();
		m_nonpcode->start(this);
#endif
*/
		// Process whatever is on the top of the context stack
		DispContextStackSingleton.Process();
	}
}

unsigned char XBCRobot::GetServoPosition(unsigned char servo)
{
	return m_servo.GetPosition(servo);
}


#ifndef NO_GPIO
unsigned short XBCRobot::GetDigital(const unsigned short sensorPort)
{
	return m_gpio.GetDigital(sensorPort);
}

unsigned short XBCRobot::SetEncoderPort(unsigned short sensorPort)
{
	return m_gpio.SetEncoderPort(sensorPort);
}

void XBCRobot::UnsetEncoderPort(unsigned short sensorPort)
{
	m_gpio.UnsetEncoderPort(sensorPort);
}

void XBCRobot::ResetEncoderCount(unsigned short sensorPort)
{
	m_gpio.ResetEncoderCount(sensorPort);
}

unsigned int XBCRobot::GetEncoderCount(unsigned short sensorPort)
{
	return m_gpio.GetEncoderCount(sensorPort);
}

unsigned short XBCRobot::SetSonarPort(unsigned short sensorPort)
{
	return m_gpio.SetSonarPort(sensorPort);
}

void XBCRobot::SendSonarRing(unsigned short sensorPort)
{
	return m_gpio.SendSonarRing(sensorPort);
}

unsigned int XBCRobot::GetSonarBlocking(unsigned short sensorPort)
{
	return m_gpio.GetSonarBlocking(sensorPort);
}

unsigned int XBCRobot::GetSonarTime(unsigned short sensorPort)
{
	return m_gpio.GetSonarTime(sensorPort);
}

void XBCRobot::SetPortDirection(unsigned short sensorPort, unsigned char portDirection)
{
  m_gpio.SetPortDirection(sensorPort, portDirection);
}

void XBCRobot::SetPortValue(unsigned short sensorPort, unsigned char portValue)
{
  m_gpio.SetPortValue(sensorPort, portValue);
}

int XBCRobot::GetPortDirection(unsigned short sensorPort)
{
  return m_gpio.GetPortDirection(sensorPort);
}

#endif

#ifndef NO_VISION
void XBCRobot::SetVision(CVision *vision)
{
	m_vision = vision;
}
void XBCRobot::SetVisionMenu(CVisionMenuHandler *vMenu)
{
    m_vMenu = vMenu;
}
#endif

bool XBCRobot::HandleMenuEvent(int eventType, CMenuElement &menu)
{
	if(eventType < 100)
	{
		return ICRobot::HandleMenuEvent(eventType, menu);
	}else{
		switch(eventType)
		{
#ifndef NO_ICXVIEW
		case 250:
			m_xview->SetViewOn();
#endif
		}
	}
	return true;
}

void XBCRobot::serial_write_long(short value_icptr)
{
	int value_1520;
#ifndef USE_OWN_CLASS
	value_1520 = MEM4((unsigned short)value_icptr);
#endif /* #ifndef USE_OWN_CLASS */
	m_commDevice->Write((char *)&value_1520,4);
}

void XBCRobot::serial_write_int(short value)
{
	m_commDevice->Write((char *)&value,2);
}

void XBCRobot::serial_read_long(short icptr)
{
	if(m_commDevice->QueryReadCount()) 
	{
		int data_1522;
		m_commDevice->Read((char *)&data_1522,4); // get serial data
		//SET_MEM4(((unsigned short)icptr), *(long int *)(&data_1522)); // write long to IC var
#ifndef USE_OWN_CLASS
		SET_MEM4(((unsigned short)icptr), data_1522); 
#endif /* #ifndef USE_OWN_CLASS */
		// write long to IC var; removed float pointer stuff
	}
#ifndef USE_OWN_CLASS
	SET_MEM4(((unsigned short)icptr), -1); // write -1 to IC var if no data available
#endif /* #ifndef USE_OWN_CLASS */
}

short XBCRobot::serial_read_int()
{
	if(m_commDevice->QueryReadCount()) 
	{
		short data_1523;
		m_commDevice->Read((char *)&data_1523,2); // get serial data
		return(data_1523);
	}
	return(-1); // Return -1 if no data available
}

short XBCRobot::CallML1Translator(const short& functionIndex, const short& argument1, void * pointer)
{
#ifdef DEBUG
	PrintToBuffer("callml(%d, %d);\n", 
		(int)functionIndex, (int)argument1);
#endif
	switch(functionIndex) {
		///////////////////////////////////////////////////////////////////
		// Tracking control
	case 1000:
		// void init_camera()
		break;
	case 1001: 
#ifndef NO_VISION
		{
		//int track_update()
		int ch, i;
		CBlob *blobs;

		if(!m_vision->NewSegmentData()) {
                    // No new data, just keep the data we got
                    break;
                }

		// 	  PrintToBuffer("Track update:\n");
		m_vision->BuildBlobs(m_blobAssm);
		for(ch=0; ch<DV_MODELS; ch++) {
			blobs = m_blobAssm.GetChannelBlobs(ch);
			i=0;
			while(blobs!=NULL && i<MAX_CH_BLOBS) {
				if(blobs->GetArea()>m_options.minBlobArea) {
					m_binfo[ch][i++].SetBlob(blobs);
				}
				blobs = blobs->next;
			}
			m_bnum[ch] = i;
			// 	    PrintToBuffer("  Ch %d: %d blobs\n", ch, i);
		}
			   }
#endif
			   break;
	case 1002:
#ifndef NO_VISION
		//////////////////////////////////////////////////////////////////
		// Normal stats
		// int track_count(int ch)
		if(argument1 >= 0 && argument1 < DV_MODELS) {
		  // If over MAX_CH_BLOBS clamp it
		  if(m_bnum[argument1] > MAX_CH_BLOBS) {
		    return(MAX_CH_BLOBS);
		  }
		  else {
		    return(m_bnum[argument1]);
		  }
		}
		else {
			return(0);
		}
#else
		return 0;
#endif 
	case 1007: 
#ifndef NO_VISION
		{
		// long track_get_frame()
		//   argument1 is a pointer to a local long, fill it in with the
		//   frame number
		unsigned int fnum = m_blobAssm.GetFrameNum();
		// 	  PrintToBuffer("  Returning frameNum %d in 0x%x\n", 
		// 			fnum, ((unsigned short)argument1));
#ifndef USE_OWN_CLASS
		SET_MEM4(((unsigned short)argument1), fnum);
#else
		*(unsigned short *)pointer=fnum;
#endif /* #ifndef USE_OWN_CLASS */
		return(0);
			   }
#else
		return 0;
#endif 
	case 1020:
#ifndef NO_VISION
		//void track_enable_orientation()
		//void track_disable_orientation()
		m_options.doBlobAxes = argument1;
		SMoments::computeAxes= argument1;
		// 	  PrintToBuffer("Set doBlobAxes to %d\n", argument1);
#endif
		break;
	case 1021:
#ifndef NO_VISION
		//int track_orientation_enabled();
		return((short)SMoments::computeAxes);
#endif
		break;
	case 1022:
#ifndef NO_VISION
		//void track_set_minarea(int minarea)
		m_options.minBlobArea=argument1;
		// 	  PrintToBuffer("Set minBlobArea to %d\n", argument1);
#endif
		break;
	case 1023:
#ifndef NO_VISION
		{
			//int _track_get_pref_value(int argtype)
            int argtype = argument1;
            if(argtype == TRACK_ENABLE_ORIENT_INDEX) {
		return((short)SMoments::computeAxes);
            }
            else if(argtype >= TRACK_RENDER_COLOR_CH0_INDEX &&
                    argtype >= TRACK_RENDER_COLOR_CH2_INDEX) {
                int ch = argtype - TRACK_RENDER_COLOR_CH0_INDEX;
                return(m_vision->GetRenderColor(ch));
            }
            else {
                // Must be a display option
		return(m_options.GetValue(argtype));
            }
        }
#else
            return 0;
#endif
	case 1024:
#ifndef NO_VISION
		//int track_is_new_data_available()
		return(m_vision->NewSegmentData());
#else
		return 0;
#endif
	case 1031:
#ifndef NO_VISION
            //void _color_reset_prep_model(int ch)
            color_reset_prep_model(argument1);
#endif
            return 0;

	case 1032:
#ifndef NO_VISION
            //int _color_check_prep_model(int ch)
            return(color_check_prep_model(argument1));
#else
            return 0;
#endif
	case 1033:
#ifndef NO_VISION
            //int _color_view_prep_model(int ch)
            color_view_prep_model(argument1);
#endif
            return 0;
	case 1034:
#ifndef NO_VISION
            //void _color_wrapup_model_update(int ch)
            color_wrapup_model_update(argument1);
#endif
            return 0;
	case 1035:
#ifndef NO_VISION
            //void _color_set_prep_model_from_flash(int ch)
            if(color_set_prep_model_from_flash(argument1)) {
                return(0);
            }
            else {
                return(-1);
            }
#else
            return 0;
#endif
        ///////////////////////////////////////////////////////////////////
	// Camera parameter support
	case 1040:
#ifndef NO_VISION
            //int _camera_get_param_value(int argtype)
            {
                CCameraSettings settings;
                if(m_vision->m_camera.ReadSettings(settings)==0) {
                    return(settings.GetValue(argument1));
                }
                else {
                    return(-1);
                }
            }
#else
            return 0;
#endif
	case 1042:
#ifndef NO_VISION
            //void _camera_view_params()
            {
                CCameraSettings settings;
                if(m_vision->m_camera.ReadSettings(settings)==-1) {
                    g_printBuffer->PrintfIC("Can't read camera settings!\n");
                    return(-1);
                }
                g_printBuffer->PrintfIC("Camera settings:\n");
                g_printBuffer->PrintfIC("  %8s\tValue\n", "Param");
                for(int i=0; i<=CAM_MAXINDEX; i++) {
                    const char *iname = CCameraSettings::GetIndexName(i);
                    g_printBuffer->PrintfIC("  %8s\t%3d\n", iname, settings.GetValue(i)); 
                }
            }
#endif
            return 0;

	///////////////////////////////////////////////////////////////////
	// Servo Status Commands	  
	case 1050:
		//enable servos
		//void enable_servos()
		m_servo.Enable();
		break;
	case 1051:
		//disable servos
		//void disable_servos()
		m_servo.Disable();
		break;
	case 1052:
		//int get_servo_position(int index)
		return m_servo.GetPosition(argument1);
	///////////////////////////////////////////////////////////////////
	// Graphics Primitives
        case 1100:
	#ifndef NO_VISION
                //draw_blank_frame
                if(!m_vision)
                        return 0;
                if(BFGET(GBA_REG_DISPCNT, videoMode) == GBA_BG_MODE_3) {
                        GbaScreenImage.fill(0xffff);
                        GbaScreenImage.write_to_gba_screen(0,0,239,159); //this is just called to reset the scaling
                        return 0;
                }
                m_options.mode=RM_RAW;
                m_vision->SetRenderFrames(0);
                CBlobContext::Push(m_vision->GetInterruptCont(),
                             *m_vision, m_options);
                GbaScreenImage.fill(0xffff);
                GbaScreenImage.write_to_gba_screen(0,0,239,159); //again, for scaling purposes
                return 0;
	#endif
        case 1105:
                //draw_clear
                if(BFGET(GBA_REG_DISPCNT, videoMode) != GBA_BG_MODE_3)
                        return 0;
                GbaScreenImage.fill(0xffff);
                return 0;
        case 1106:
                //return_display
                if(BFGET(GBA_REG_DISPCNT, videoMode) != GBA_BG_MODE_3)
                        return 0;
                DispContextStackSingleton.PopContext();
                return 0;


#ifndef NO_ICXVIEW
	//ICXView Stuff:
	case 1500:
		m_xview->SetViewOn();
		break;
	case 1501:
		m_xview->SetViewOff();
		SetupDisplay();
		//ICRobotPBufContext::Push(*g_printBuffer, *this);
		break;
	case 1502:
		m_xview->Blank(argument1);
		break;
	case 1503:
		m_xview->StartPaint();
		break;
#endif

	// printf double-buffer stuff
	// By Jeremy Rand
	case 1510:
		g_printBuffer->CurrPrintBufToScreen();
		break;
	case 1511:
		g_printBuffer->ScreenToCurrPrintBuf();
		break;

	// Serial non-byte types
	// By Jeremy Rand
	// These are used to send ints or longs over the serial port.
	// Not tested since I don't have access to two XBC's right now.
	case 1520:
		// IC: void serial_write_long(long value)
		// callml(1520, (int)&value);

		/*int value_1520;
		value_1520 = MEM4(argument1);
		m_commDevice->Write((char *)&value_1520,4);
		break;*/

		serial_write_long(argument1);
	case 1521:
		// IC: void serial_write_int(int value)
		// callml(1521, value);

		/*m_commDevice->Write((char *)&argument1,2);*/

		serial_write_int(argument1);
		break;
	case 1522:
		// IC: long serial_read_long()
		// callml(1522, (int)&value);

		/*if(m_commDevice->QueryReadCount()) 
		{
			int data_1522;
			m_commDevice->Read((char *)&data_1522,4); // get serial data
			//SET_MEM4(((unsigned short)argument1), *(long int *)(&data_1522)); // write long to IC var
			SET_MEM4(((unsigned short)argument1), data_1522); 
			// write long to IC var; removed float pointer stuff
		}
		SET_MEM4(((unsigned short)argument1), -1); // write -1 to IC var if no data available*/

		serial_read_long(argument1);
		break;
	case 1523:
		// IC int serial_read_int()
		// callml(1523, 0);

		/*if(m_commDevice->QueryReadCount()) 
		{
			short data_1523;
			m_commDevice->Read((char *)&data_1523,2); // get serial data
			return(data_1523);
		}
		return(-1); // Return -1 if no data available*/

		return(serial_read_int());
		break;
	case 1524:
		// High byte of int16
		return(argument1 >> 8);
	case 1525:
		// Low byte of int16
		return(argument1 & 0xFF);
	case 1526:
		// High int16 of int32
#ifndef USE_OWN_CLASS
		return(MEM4(argument1) >> 16);
#else
		return (*(long *)pointer)>>16;
#endif /* #ifndef USE_OWN_CLASS */
	case 1527:
		// Low int16 of int32
#ifndef USE_OWN_CLASS
		return(MEM4(argument1) & 0xFFFF);
#else
		return (*(long *)pointer) & 0xFFFF;
#endif /* #ifndef USE_OWN_CLASS */

	// Direct Sound Playing
	case 1540:
		// Play sound
		switch(argument1)
		{
			#ifdef AUDIO_AHH
			case 1:
				m_tone.Play((char *)ahh, sizeof(ahh));
				break;
			#endif
			#ifdef AUDIO_BLUE
			case 2:
				m_tone.Play((char *)blue, sizeof(blue));
				break;
			#endif
			#ifdef AUDIO_HOKEY
			case 3:
				m_tone.Play((char *)hokey, sizeof(hokey));
				break;
			#endif
			#ifdef AUDIO_MUCHACHA
			case 4:
				m_tone.Play((char *)muchacha, sizeof(muchacha));
				break;
			#endif
			#ifdef AUDIO_OUCH
			case 5:
				m_tone.Play((char *)ouch, sizeof(ouch));
				break;
			#endif
			#ifdef AUDIO_PRADO
			case 6:
				m_tone.Play((char *)prado, sizeof(prado));
				break;
			#endif
			#ifdef AUDIO_FOURBYTESOUND
			case 7:
				m_tone.Play((char *)fourbytesound, sizeof(fourbytesound));
				break;
			#endif
			#ifdef AUDIO_DEATH
			case 8:
				m_tone.Play((char *)death2, sizeof(death2));
				break;
			#endif
			#ifdef AUDIO_INTRO
			case 9:
				m_tone.Play((char *)intro, sizeof(intro),false,true);
				break;
			#endif
			#ifdef AUDIO_MUNCH
			case 10:
				m_tone.Play((char *)munch, sizeof(munch),true,true);
				break;
			#endif
			#ifdef AUDIO_SIREN
			case 11:
				m_tone.Play((char *)siren, sizeof(siren),true,false);
				break;
			#endif
			#ifdef AUDIO_WARNING
			case 12:
				m_tone.Play((char *)warning, sizeof(warning));
				break;
			#endif
			#ifdef AUDIO_READY
			case 13:
				m_tone.Play((char *)ready, sizeof(ready));
				break;
			#endif
			#ifdef AUDIO_GO
			case 14:
				m_tone.Play((char *)go, sizeof(go));
				break;
			#endif
		}
		break;
	case 1541:
		// Stop sound.
		m_tone.Stop();
		break;
	case 1542:
		// Is sound playing?
		return(m_tone.Playing());
	case 1543:
		// Just here to test if we have access to m_tone.
		m_tone.SetEnable(1);
		break;
	case 1544:
		// Make sure that sonar jamming is disabled; it interferes with Timer 3
		if(m_gpio.GetSonarJamming() > -1)
			m_gpio.SetSonarJamming(-1);
		
		// Play sound
		switch(argument1)
		{
			#ifdef AUDIO_AHH
			case 1:
				m_tone.PlayB((char *)ahh, sizeof(ahh));
				break;
			#endif
			#ifdef AUDIO_BLUE
			case 2:
				m_tone.PlayB((char *)blue, sizeof(blue));
				break;
			#endif
			#ifdef AUDIO_HOKEY
			case 3:
				m_tone.PlayB((char *)hokey, sizeof(hokey));
				break;
			#endif
			#ifdef AUDIO_MUCHACHA
			case 4:
				m_tone.PlayB((char *)muchacha, sizeof(muchacha));
				break;
			#endif
			#ifdef AUDIO_OUCH
			case 5:
				m_tone.PlayB((char *)ouch, sizeof(ouch));
				break;
			#endif
			#ifdef AUDIO_PRADO
			case 6:
				m_tone.PlayB((char *)prado, sizeof(prado));
				break;
			#endif
			#ifdef AUDIO_FOURBYTESOUND
			case 7:
				m_tone.PlayB((char *)fourbytesound, sizeof(fourbytesound));
				break;
			#endif
			#ifdef AUDIO_DEATH
			case 8:
				m_tone.PlayB((char *)death2, sizeof(death2));
				break;
			#endif
			#ifdef AUDIO_INTRO
			case 9:
				m_tone.PlayB((char *)intro, sizeof(intro),false,true);
				break;
			#endif
			#ifdef AUDIO_MUNCH
			case 10:
				m_tone.PlayB((char *)munch, sizeof(munch),true,true);
				break;
			#endif
			#ifdef AUDIO_SIREN
			case 11:
				m_tone.PlayB((char *)siren, sizeof(siren),true,false);
				break;
			#endif
			#ifdef AUDIO_WARNING
			case 12:
				m_tone.PlayB((char *)warning, sizeof(warning));
				break;
			#endif
			#ifdef AUDIO_READY
			case 13:
				m_tone.PlayB((char *)ready, sizeof(ready));
				break;
			#endif
			#ifdef AUDIO_GO
			case 14:
				m_tone.PlayB((char *)go, sizeof(go));
				break;
			#endif
		}
		break;
	case 1545:
		// Stop sound.
		m_tone.StopB();
		break;
	case 1546:
		// Is sound playing?
		return(m_tone.PlayingB());
	case 1547:
		m_tone.SetVolumeA(argument1);
		break;
	case 1548:
		m_tone.SetVolumeB(argument1);
		break;


	// Audio Amplitude Resolution Commands
	case 1550:
		// Set amplitude resolution
		m_tone.SetAmplitudeResolution((unsigned short)argument1);
		break;
	case 1551:
		// Get amplitude resolution
		return(m_tone.GetAmplitudeResolution());
	case 1552:
		// IC: void set_beeper_pitch_long(long freq)
		// callml(1552, (int)&freq)
#ifndef USE_OWN_CLASS
		m_tone.SetFreq(MEM4((unsigned short)argument1));
#else
		m_tone.SetFreq(*(long *)pointer);
#endif /* #ifndef USE_OWN_CLASS */
		break;

	case 1560:
	{
		// int button_state_bg()
		// Returns the current button state, even if IC does not own the buttons.
		// Intended for Graphics Mode 3, but will probably also work at the main menu.
		
		// Button State
	        static bool firstBtnState=true;
		m_buttonState.PollKeys();	
		
		// The first time anyone calls this, make sure we go
		// into the right state for the IC console.  
		//if(firstBtnState) {
		//  g_printBuffer->ShowICPrintBuffer();
		//  ICRobotPBufContext::Push(*g_printBuffer, *this);
		//  firstBtnState=false;
		//}
		
		return m_buttonState.CurrState();
	}

	case 1570:
	{
		// void set_sonar_jamming(short port)
		// Activates or deactivates sonar jamming
		
		if(argument1 > -1 && m_tone.PlayingB()) // if we're activating jamming and a sound is playing on Chan. B
			m_tone.StopB(); // Stop sound on Chan. B
		
		m_gpio.SetSonarJamming(argument1);
		
		break;
	}
	case 1571:
	{
		// short get_sonar_jamming()
		// Returns the port on which sonar jamming is enabled, or -1 if sonar jamming is disabled
		
		return(m_gpio.GetSonarJamming());
	}
	
	// New motor functions
	case 1580:
	{
		// short getpwm8(short m)
		// Returns the current PWM level for motor m, range = (-255,255)
		
		return(GetPWM(argument1));
	}
	case 1581:
	{
		// short get_current(short m)
		// Returns the current current for motor m
		
		return(GetCurrent(argument1));
	}
	case 1582:
	{
		// short get_velocity(short m)
		// Returns the current velocity for motor m
		// Deprecated; use the long version instead of the short version
		
		return(GetVelocity(argument1));
	}
	case 1583:
	{
		if(argument1 > 0)
			m_BemfDiv = argument1;
		break;
	}
	case 1584:
	{
		return(m_BemfDiv);
	}
	case 1585:
	{
		// long get_x_coord()
		SET_MEM4( ((unsigned short)argument1), (long) GetXCoord() );
		break;
	}
	case 1586:
	{
		// long get_y_coord()
		SET_MEM4( ((unsigned short)argument1), (long) GetYCoord() );
		break;
	}
	case 1587:
	{
		// void set_base_enabled(short enable)
		SetBaseEnabled(argument1);
		break;
	}
	case 1588:
	{
		// void set_translation_scale(long scale)
		SetTranslationScale(MEM4((unsigned short)argument1));
		break;
	}
	case 1589:
	{
		// void set_rotation_scale(long scale)
		SetRotationScale(MEM4((unsigned short)argument1));
		break;
	}
	
	case 1590:
	{
		// short set_bgcolor(short color)
		// Sets the background color of the text in the palette
		
		m_palette->SetColor(254, argument1);
		m_palette->WriteToGBA();
		return(argument1);
	}
	case 1591:
	{
		// short set_fgcolor(short color)
		// Sets the foreground color of the text in the palette
		
		m_palette->SetColor(253, argument1);
		m_palette->WriteToGBA();
		return(argument1);
	}
	case 1592:
	{
		// void invert_colors()
		// Inverts all colors in the palette
		
		m_palette->Invert();
		m_palette->WriteToGBA();
		return(argument1);
	}
	case 1593:
	{
		// void invert_color_v()
		// Inverts HSV value of all colors in the palette
		
		m_palette->InvertValue();
		m_palette->WriteToGBA();
		return(argument1);
	}
	
	// Camera pursue functions -- THESE DO NOT WORK AND WILL CRASH YOUR XBC!!! DO NOT TRY THEM!!!
	case 1600:
	{
		// short find_blocking(short model)
		
		return(m_pursue.Find(argument1));
	}
	case 1601:
	{
		// short persue_blocking(short velocity)
		
		SetBaseEnabled(1);
		return(m_pursue.Pursue(argument1));
	}
	case 1602:
	{
		// short find_slice(short model)
		
		//return(m_pursue.FindSlice(argument1));
		m_binfo[argument1][0].DoStats();
		return(m_pursue.FindSliceIC(m_binfo[argument1][0].blob, &(m_binfo[argument1][0].stats)));
	}
	case 1603:
	{
		// short pursue_slice(short velocity)
		
		SetBaseEnabled(1);
		return(m_pursue.PursueSlice(argument1));
	}
	
	// DiffBase Functions
	case 1610:
	{
		// void set_x_coord(long coord)
		SetXCoord(MEM4((unsigned short)argument1));
		break;
	}
	case 1611:
	{
		// void set_y_coord(long coord)
		SetYCoord(MEM4((unsigned short)argument1));
		break;
	}

	// ServoTraj Functions
	/*
	case 1620:
	{
		m_servo.Stop(argument1);
		break;
	}
	case 1621:
	{
		return(m_servo.Done(argument1));
	}
	case 1622:
	{
		// int servo_get_start_position(short servo)
		return(m_servo.GetStartPosition(argument1));
	}
	case 1623:
	{
		// int servo_get_end_position(short servo)
		return(m_servo.GetEndPosition(argument1));
	}*/

	// Nonstandard motor stats
	case 1630:
	{
		// BackEMF value
		// short get_backemf(short port);
		unsigned char * base = (unsigned char *)BEMF4_BASE;
		volatile short * m_p_bemf = (short *)(base + 512);
		return(m_p_bemf[argument1]);
	}
	case 1631:
	{
		// Analog motor port sum value
		// short get_motor_analog_sum(short port); // port = 0-3
		unsigned char * base = (unsigned char *)BEMF4_BASE;
		volatile short * m_p_analog      = (short *)(base + 520);
		
		// We sum 16 analog values
		short sum = 0;
		for (unsigned char j = 0; j < 16; j++) 
		{
			sum += m_p_analog[argument1 * 16 + j];
		}
		
		return(sum);
	}
	
	// GetPWM averages
	case 1640:
	{
		// short getpwm_sum32(short axis);
		short sum = 0;
		
		for(int index = 0; index < 0x20; index++)
		{
			sum += getpwm_logs[argument1][index];
		}
		
		return(sum);
	}
	
	#ifndef NO_IR_TRACK
	// IRTrack
	case 1650:
	{
		// void ir_track_set_motor(short port);
		
		IRTrackSetMotor(argument1);
		break;
	}
	case 1651:
	{
		// void ir_track_set_sensor(short port);
		
		IRTrackSetSensor(argument1);
		break;
	}
	case 1652:
	{
		// short ir_range(short port);
		
		return(IRRange(argument1));
	}
	case 1653:
	{
		// void ir_track_dump();
		
		IRTrackDump();
		break;
	}
	case 1654:
	{
		return(ir_track_index);
	}
	case 1655:
	{
		return(ir_track_counter);
	}
	case 1656:
	{
		IRTrackStop();
		break;
	}
	case 1657:
	{
		for(int index = 0; index < ir_track_index; index++)
		{
			SET_MEM4( (unsigned short) argument1 + index * sizeof(long), ir_track_positions[index]);
		}
		break;
		
		//memcpy(mem + (unsigned short)argument1, ir_track_positions, ir_track_index * sizeof(long));
		//break;
	}
	case 1658:
	{
		for(int index = 0; index < ir_track_index; index++)
		{
			SET_MEM2( (unsigned short) argument1 + index * sizeof(short), ir_track_ranges[index]);
		}
		break;
		
		//memcpy(mem + (unsigned short)argument1, ir_track_ranges, ir_track_index * sizeof(short));
		//break;
	}
	case 1659:
	{
		// short ir_range_fast(short port);
		
		return(IRRangeFast(argument1));
	}
	
	#endif

	// RecAxes
	case 1660:
	{
		Record();
		break;
	}
	case 1661:
	{
		Play();
		break;
	}
	case 1662:
	{
		RStop();
		break;
	}
	case 1663:
	{
		return(Playing());
	}
	case 1664:
	{
		return(Recording());
	}

	#ifndef NO_IR_TRACK
	case 1670:
	{
		// short ir_range_benchmark(short port);
		
		IRRangeBenchmark(argument1);
		break;
	}
	case 1671:
	{
		// short ir_track_set_radius_offset(short offset);
		
		ir_track_radius_offset = argument1;
		break;
	}
	case 1672:
	{
		IRTrackProcess();
		break;
	}
	case 1673:
	{
		ir_track_max_horizon = argument1;
		break;
	}
	case 1674:
	{
		ir_track_max_slope = argument1;
		break;
	}
	case 1675:
	{
		ir_track_max_depth = argument1;
		break;
	}
	case 1676:
	{
		// even though the value is internally 32-bit, a 16-bit callml works fine... 
		// the value is usually going to be under 100 (decimal)
		ir_track_min_angular_width = argument1;
		break;
	}
	case 1677:
	{
		// short ir_track_blob_count();
		
		return(ir_track_current_blob);
	}
	#endif

	default:
		return ICRobot::CallML1Translator(functionIndex, argument1, pointer);
	}
	return(0);
}

short XBCRobot::CallML2Translator(const short& functionIndex, const short& argument1, const short& argument2, void * pointer, void * pointer2)
{
#ifdef DEBUG
	PrintToBuffer("callml(%d, %d, %d);\n", 
		(int)functionIndex, 
		(int)argument1, (int)argument2);
#endif
	switch(functionIndex) {
		///////////////////////////////////////////////////////////////////
		// Tracking stats	  
	case 2003:
#ifndef NO_VISION
		//int track_size(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
		   argument2 >= 0 && argument2 < m_bnum[argument1])  {
		  // Get the statistics for the blob into the
		  // appropriate SBlobInfo structure.  If it hasn't
		  // been calculated yet this will calculate it.  If
		  // it has this will just return;
		  m_binfo[argument1][argument2].DoStats();
	
		  // If over 15-bit it's going to show up in IC as negative, 
		  // so clamp it
		  int max_size = 32767;
		  int size = m_binfo[argument1][argument2].stats.area;
		  if(size > max_size) {
		    DBG(printf("Blob size overflow, returning %d (0x%x)\n", 
			       max_size, max_size));
		    return(max_size);
		  }
		  else {
		    return(size);
		  }
			}
		else {
			return(0);
		}
#else 
		return 0;
#endif
	case 2004:
#ifndef NO_VISION
		//int track_x(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
			argument2 >= 0 && argument2 < m_bnum[argument1])  {
				m_binfo[argument1][argument2].DoStats();
				return((short)(m_binfo[argument1][argument2].stats.centroidX));
			}
		else {
			return(0);
		}
#else 
		return 0;
#endif
	case 2005:
#ifndef NO_VISION
		//int track_y(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
			argument2 >= 0 && argument2 < m_bnum[argument1])  {
				m_binfo[argument1][argument2].DoStats();
				return((short)(m_binfo[argument1][argument2].stats.centroidY));
			}
		else {
			return(0);
		}
#else 
		return 0;
#endif
	case 2006:
#ifndef NO_VISION
		//int track_confidence(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
			argument2 >= 0 && argument2 < m_bnum[argument1])  {
				m_binfo[argument1][argument2].DoStats();
				int area = m_binfo[argument1][argument2].stats.area;
				short left, top, right, bottom;
				m_binfo[argument1][argument2].blob->getBBox(left,top,
					right, bottom);
				int boxArea = (bottom+1-top)*(right+1-left);
				int confidence = (area * 100)/(boxArea);
				// 	    printf("Ch %d, blob %d:\n  conf %d, area %d, boxarea %d\n", 
				// 		   argument1, argument2, confidence, area, boxArea);
				return(confidence);
			}
		else {
			return(0);
		}
#else 
		return 0;
#endif
	case 2007:
#ifndef NO_VISION
		//int track_bbox_top(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
                   argument2 >= 0 && argument2 < m_bnum[argument1])  {
                    return((short)(m_binfo[argument1][argument2].blob->top));
                }
		else {
                    return(0);
		}
#else 
		return 0;
#endif
	case 2008:
#ifndef NO_VISION
		//int track_bbox_bottom(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
                   argument2 >= 0 && argument2 < m_bnum[argument1])  {
                    return((short)(m_binfo[argument1][argument2].blob->lastBottom.row));
                }
		else {
                    return(0);
		}
#else 
		return 0;
#endif
	case 2009:
#ifndef NO_VISION
		//int track_bbox_left(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
                   argument2 >= 0 && argument2 < m_bnum[argument1])  {
                    return((short)(m_binfo[argument1][argument2].blob->left));
                }
		else {
                    return(0);
		}
#else 
		return 0;
#endif
	case 2010:
#ifndef NO_VISION
		//int track_bbox_right(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
                   argument2 >= 0 && argument2 < m_bnum[argument1])  {
                    return((short)(m_binfo[argument1][argument2].blob->right));
                }
		else {
                    return(0);
		}
#else 
		return 0;
#endif
	case 2011:
#ifndef NO_VISION
		//int track_bbox_width(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
                   argument2 >= 0 && argument2 < m_bnum[argument1])  {
                    return((short)(m_binfo[argument1][argument2].blob->right-
                                   m_binfo[argument1][argument2].blob->left));
                }
		else {
                    return(0);
		}
#else 
		return 0;
#endif
	case 2012:
#ifndef NO_VISION
		//int track_bbox_height(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
                   argument2 >= 0 && argument2 < m_bnum[argument1])  {
                    return((short)(m_binfo[argument1][argument2].blob->lastBottom.row-
                                   m_binfo[argument1][argument2].blob->top));
                }
		else {
                    return(0);
		}
#else 
		return 0;
#endif
	case 2013:
#ifndef NO_VISION
		//int track_bbox_area(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
                   argument2 >= 0 && argument2 < m_bnum[argument1])  {
                    int height = 
                        m_binfo[argument1][argument2].blob->lastBottom.row-
                        m_binfo[argument1][argument2].blob->top;
                    int width = 
                        m_binfo[argument1][argument2].blob->right-
                        m_binfo[argument1][argument2].blob->left;

                    // If over 15-bit it's going to show up in IC as
                    // negative, so clamp it
                    int max_size = 32767;
                    int size = height*width;
                    if(size > max_size) {
                        DBG(printf("Blob size overflow, returning %d (0x%x)\n", 
                                   max_size, max_size));
                        return((short)max_size);
                    }
                    
                    return((short)(size));
                }
		else {
                    return(0);
		}
#else 
		return 0;
#endif

        case 2024: 
#ifndef NO_VISION
            // int _track_set_pref_value(int argtype, int val)
            {
                int argtype = argument1;
                int val = argument2;

                if(argtype == TRACK_ENABLE_ORIENT_INDEX) {
                    m_options.doBlobAxes = val;
                    SMoments::computeAxes= val;
                    DBG(printf("Set orient enable to %d\n", 
                               val));
                }
                else if(argtype >= TRACK_RENDER_COLOR_CH0_INDEX &&
                        argtype >= TRACK_RENDER_COLOR_CH2_INDEX) {
                    int ch = argtype - TRACK_RENDER_COLOR_CH0_INDEX;
                    m_vision->SetRenderColor(ch, val);
                    DBG(printf("Set render color for %d to 0x%x\n", 
                               ch, val));
                }
                else {
                    // Must be a display option
                    if(!m_options.SetValue(argtype, val)) {
                        DBG(printf("Failed to set track pref %d to %d\n", 
                                   argtype, val));
                        return(-1);
                    }
#ifdef DEBUG
                    const char *iname = CVisionDispOptions::GetIndexName(argtype);
                    printf("Set track pref %s (%d) to %d\n", 
                           iname, argtype, val);
#endif

                    // If changing enable inform the blob assembler
                    // of the new channel mask
                    if(argtype >= TRACK_ENABLE_CH0_INDEX && 
                       argtype <= TRACK_ENABLE_CH2_INDEX) {
                        int newmask = m_options.GetEnableMask();
                        m_blobAssm.SetEnableMask(newmask);
                        DBG(printf("Set enable mask to 0x%x\n", newmask));
                    }
                }
                return(0);
            }
#else 
		return 0;
#endif
        ///////////////////////////////////////////////////////////////////
	// Vision model support
	case 2030:
#ifndef NO_VISION
            //void _color_get_ram_value(int ch, int argtype)
            {
                CColorModelHSV model;
                if(m_vision->GetCurrentModel(argument1, model)) {
                    return(model.GetValue(argument2));
                }
                else {
                    return(-1);
                }
            }
#else
            return 0;
#endif
	case 2031:
#ifndef NO_VISION
            //void _color_get_flash_value(int ch, int argtype)
            {
                CColorModelHSV model;
                if(m_vMenu==NULL) {
                    return(-1);
                }

                if(m_vMenu->m_flashHandler.GetSavedModel(argument1, 
                                                         model)) {
                    return(model.GetValue(argument2));
                }
                else {
                    return(-1);
                }
            }
#else
            return 0;
#endif
        ///////////////////////////////////////////////////////////////////
	// Camera parameter support
	case 2041:
#ifndef NO_VISION
            //int _camera_set_param_value(int argtype, int val)
            {
                int argtype=argument1, val=argument2;
                return(m_vision->m_camera.SetValue(argtype, val));
            }
#endif
            return 0;
	case 2043:
#ifndef NO_VISION
            // int _camera_set_wb_color_temp(int red, int blue)
            {
                int red=argument1, blue=argument2;

                m_vision->m_camera.SetAWB(false);
                return(m_vision->m_camera.SetWhiteBalance(blue, red));
            }
#else
            return 0;
#endif

	///////////////////////////////////////////////////////////////////
	// Servo Status Commands	  
	case 2050:
		m_servo.SetPosition(argument1, argument2);
		break;
	
	///////////////////////////////////////////////////////////////////
        // Graphics Primitives
        case 2101:
	#ifndef NO_VISION
                //draw_camera_frame
                if(!m_vision)
                        return 0;
                if(argument1)
                        m_options.mode = RM_COMBINED;
                else
                        m_options.mode = RM_RAW;
                m_options.SetEnableMask(argument2);
                m_vision->SetRenderFrames(1);
                CBlobContext::Push(m_vision->GetInterruptCont(),
                             *m_vision, m_options);
                return 0;
	#endif

	case 2107:
                if(BFGET(GBA_REG_DISPCNT, videoMode) != GBA_BG_MODE_3)
                        return 0;
                return GbaScreenImage.pixel((int)argument1, (int)argument2);
#ifndef NO_ICXVIEW
	//ICXView 2 params
	case 2500:
		//XVGetColor(x,y)
		m_xview->GetColor(argument1,argument2);
		break;
#endif

	// Complex Serial Types
	// Not Tested As Of Oct. 10, 2006
	case 2520:
		// IC int byte_array_to_int(int bytes[2])
		// callml(2520, bytes[0], bytes[1])
		return( (argument1 << 8) | argument2);
	
	// Motor stats
	case 2530:
		SET_MEM4( ((unsigned short)argument2), (long) GetVelocity(argument1) );
		break;
	
	// DiffBase
	case 2540:
		// mav_ticks
		CHK_MNUM(argument1);
		IcMoveVelocityTicks(argument1, argument2, 
			       m_motionAccel[argument1]);
		break;
	
	// ServoTraj Functions
	/*
	case 2550:
	{
		m_servo.MoveVelocity(argument1, argument2);
		break;
	}
	case 2551:
	{
		//long servo_get_velocity(short servo)
		SET_MEM4( ((unsigned short)argument2), m_servo.GetVelocity(argument1));
		break;
	}
	case 2552:
	{
		// long servo_get_start_mseconds(short servo)
		SET_MEM4( ((unsigned short)argument2), (int) (m_servo.GetStartTime(argument1)/1000));
		break;
	}*/
	
	// Nonstandard motor stats
	case 2560:
	{
		// Analog motor port value
		// short motor_analog(short port, short channel); // port = 0-3, channel = 0-15
		unsigned char * base = (unsigned char *)BEMF4_BASE;
		volatile short * m_p_analog      = (short *)(base + 520);
		return(m_p_analog[argument1 * 16 + argument2]);
	}
	
	#ifndef NO_IR_TRACK
	
	// IRTrack
	case 2570:
	{
		// void ir_track_start(short velocity, long delta_pos);
		
		IRTrackStart(argument1, (signed int)MEM4((unsigned short)argument2) + IcGetPosition(ir_track_motor_port));
		break;
	}
	case 2571:
	{
		// long ir_track_blob_center(short blob);
		
		SET_MEM4( (unsigned short)argument1, (ir_track_positions[ir_track_blob_start[argument2]]+ir_track_positions[ir_track_blob_end[argument2]]) >> 1 );
		break;
	}
	case 2572:
	{
		// long ir_track_blob_low(short blob);
		
		SET_MEM4( (unsigned short)argument1, ir_track_positions[ir_track_blob_start[argument2]] );
		break;
	}
	case 2573:
	{
		// long ir_track_blob_high(short blob);
		
		SET_MEM4( (unsigned short)argument1, ir_track_positions[ir_track_blob_end[argument2]] );
		break;
	}
	
	#endif
	
	default:
		return ICRobot::CallML2Translator(functionIndex, argument1, argument2, pointer, pointer2);
	}
	return(0);
}

short XBCRobot::CallML3Translator(const short& functionIndex, const short& argument1, const short& argument2, const short& argument3, void * pointer)
{
#ifdef DEBUG
	PrintToBuffer("callml(%d, %d, %d, %d);\n", 
		(int)functionIndex, 
		(int)argument1, (int)argument2, (int)argument3);
#endif
	switch(functionIndex) {
		///////////////////////////////////////////////////////////////////
		// Orientation stats
		//   argument3 is a pointer to a local float, fill it in with the
		//   frame number
	case 3010: 
#ifndef NO_VISION
		{
		// float track_angle(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
			argument2 >= 0 && argument2 < m_bnum[argument1])  {
				m_binfo[argument1][argument2].DoStats();
				float32 angle = m_binfo[argument1][argument2].stats.angle;
				// 	    PrintToBuffer("  Returning angle %f in 0x%x\n", 
				// 			  angle, ((unsigned short)argument3));
#ifndef USE_OWN_CLASS
				SET_MEM4(((unsigned short)argument3), *(long int *)(&angle));
#else
				*(float32 *)pointer=*(float32 *)(&angle);
#endif /* #ifndef USE_OWN_CLASS */
				return(0);
			}
		else {
			return(-1);
		}
			   }
#else 
		return 0;
#endif

	case 3011: 
#ifndef NO_VISION
		{
		// float track_major_axis(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
			argument2 >= 0 && argument2 < m_bnum[argument1])  {
				m_binfo[argument1][argument2].DoStats();
				float32 majAxis = m_binfo[argument1][argument2].stats.majorDiameter;
				// 	    PrintToBuffer("  Returning majAxis %f in 0x%x\n", 
				// 			  majAxis, ((unsigned short)argument3));
#ifndef USE_OWN_CLASS
				SET_MEM4(((unsigned short)argument3), *(long int *)(&majAxis));
#else
				*(long *)pointer = *(long *)(&majAxis);
#endif /* #ifndef USE_OWN_CLASS */
				return(0);
			}
		else {
			return(-1);
		}

			   }
#else 
		return 0;
#endif

	case 3012: 
#ifndef NO_VISION
		{
		// float track_minor_axis(int ch, int i)
		if(argument1 >= 0 && argument1 < DV_MODELS && 
			argument2 >= 0 && argument2 < m_bnum[argument1])  {
				m_binfo[argument1][argument2].DoStats();
				float32 minAxis = m_binfo[argument1][argument2].stats.minorDiameter;
				// 	    PrintToBuffer("  Returning minAxis %f in 0x%x\n", 
				// 			  minAxis, ((unsigned short)argument3));
#ifndef USE_OWN_CLASS
				SET_MEM4(((unsigned short)argument3), *(long int *)(&minAxis));
#else
				*(long *)pointer = (long)minAxis;
#endif /* #ifndef USE_OWN_CLASS */

				return(0);
			}
		else {
			return(-1);
		}
			   }
#else 
		return 0;
#endif

 ///////////////////////////////////////////////////////////////////
 // Color setting

	case 3020: 
#ifndef NO_VISION
        {
            // int _color_prep_model_value(int ch, int argtype, int val)
            if(color_prep_model_value(argument1, argument2, argument3)) {
                return(0);
            }
            else {
                return(-1);
            }
        }
#else 
		return 0;
#endif
	case 3021: 
#ifndef NO_VISION
        {
            // void _color_apply_model_partial(int ch, int partBegin, int partEnd)
            if(argument1 < 0 || argument1 >= DV_MODELS) {
                return(-1);
            }
            // TODO: Check to make sure all the values are set
            return(m_vision->UploadModelPartial(argument1, 
                                                m_prep_model[argument1], 
                                                argument2, argument3));
        }
#else 
		return 0;
#endif
			   ///////////////////////////////////////////////////////////////////
			   // Tracking Display
	case 3030:
#ifndef NO_VISION
		// void track_show_display(int show_processed, int frameskip, 
		//                         int channel_mask)
	  if(!m_vision) {
	    return(-1);
	  }
	  if(argument1) {
	    m_options.mode = RM_COMBINED; 
	  }
	  else {
	    m_options.mode = RM_RAW;
	  }
	  // TODO: implement frameskip
	  m_options.SetEnableMask(argument3);
	  m_vision->SetRenderFrames(DV_RENDER_CONTINUOUS);
	  CBlobContext::Push(m_vision->GetInterruptCont(), 
			     *m_vision, m_options);
	  return(0);
#endif		
		break;

	////////////////////////////////////////////////////////////////////////////
        //  Graphics Primitives
        case 3102:
                //draw_pixel
                if(BFGET(GBA_REG_DISPCNT, videoMode) != GBA_BG_MODE_3)
                        return 0;
                GbaScreenImage.pixel((int)argument1, (int)argument2) = (TPix)argument3;
                return 0;
        case 3103:
                //draw_line
                if(BFGET(GBA_REG_DISPCNT, videoMode) != GBA_BG_MODE_3)
                        return 0;
                /* Just for reference
                x1=argument1&0xff;
                y1=(argument1>>8)&0xff;
                x2=argument2&0xff;
                y2=(argument2>>8)&0xff; */
                GbaScreenImage.draw_line(argument1&0xff,(argument1>>8)&0xff,
                                                                argument2&0xff,(argument2>>8)&0xff, argument3);
                return 0;
        case 3104:
                //draw_rect
                if(BFGET(GBA_REG_DISPCNT, videoMode) != GBA_BG_MODE_3)
                        return 0;
                GbaScreenImage.draw_box(argument1&0xff,(argument1>>8)&0xff,
                                                                argument2&0xff,(argument2>>8)&0xff, argument3);
                return 0;

#ifndef NO_ICXVIEW
	case 3500:
		//XVSetColor(x,y,color)
		m_xview->SetColor(argument1,argument2,argument3);
		break;
	case 3501:
		// ICXView: DrawRectPixIDs(int topleft, int dimensions, unsigned short color)
		// IC: DrawRect(int topleft, int dimensions, int color);
		m_xview->DrawRectPixIDs(argument1, argument2, argument3);
		break;
#endif
	// Complex Serial Types
	// NOT TESTED as of Oct. 10, 2006
	case 3520:
		// IC long int_array_to_long(int ints[2])
		// callml(3520, ints[0], ints[1], (int)&result)
#ifndef USE_OWN_CLASS
		SET_MEM4( ((unsigned short)argument3), ( (int)argument1) << 16 | ( (int)argument2)  );
#else
		*(long *)pointer=(int)argument1<<16 | (int)argument2;
#endif /* #ifndef USE_OWN_CLASS */
		break;

	case 3530:
		CHK_MNUM(argument1);

		#ifndef USE_OWN_CLASS
	    IcMoveTicks(argument1, 
		   (signed int)MEM4((unsigned short)argument3),
		   argument2,
		   m_motionAccel[argument1]);
		#else
		IcMoveTicks(argument1,
			*(signed int *)(pointer),
			argument2,
			m_motionAccel[argument1]);
		#endif /* #ifndef USE_OWN_CLASS */
	    break;

	case 3531:
	{
		CHK_MNUM(argument1);

	    if(Done(argument1)) SyncTrajectory(argument1);
	    AxisPosition spos = IcGetPosition(argument1);
#ifndef USE_OWN_CLASS
	    AxisPosition goal = (signed int)MEM4((unsigned short)argument3) + spos;
#else
		AxisPosition goal = (*((long *)pointer)) + spos;
#endif /* #ifndef USE_OWN_CLASS */
	    IcMoveTicks(argument1, goal, 
		   argument2, 
		   m_motionAccel[argument1]);
	    break;
	}
	
	// ServoTraj Functions
	/*
	case 3540:
	{
		m_servo.Move(argument1, argument2, argument3);
		break;
	}*/

	default:
		return ICRobot::CallML3Translator(functionIndex, argument1, argument2, argument3, pointer);
	}
	return(0);
}


#ifndef NO_VISION
/////////////////////////////////////////////////////////////////////////
// Support for setting models from IC
void
XBCRobot::color_reset_prep_model(int ch) 
{
    if(ch < 0 || ch >= DV_MODELS) {
        return;
    }
    for(int i=MODEL_MININDEX; i<=MODEL_MAXINDEX; i++) {
        m_prep_set[ch][i]=false;
    }
    m_prep_status[ch]=COLOR_CHECK_UNKNOWN;
}

void
XBCRobot::color_view_prep_model(int ch) 
{
    if(ch < 0 || ch >= DV_MODELS) {
        g_printBuffer->PrintfIC("Model index %d out of range!\n", 
                                ch);
        return;
    }

    CColorModelHSV old_model;
    m_vision->GetCurrentModel(ch, old_model);

    g_printBuffer->PrintfIC("Model ch %d, status %s:\n", ch, 
                            (m_prep_status[ch]==COLOR_CHECK_OK)?"OK":
                             ((m_prep_status[ch]==COLOR_CHECK_BAD)?"BAD":
                              "UNKNOWN"));
    g_printBuffer->PrintfIC("  Type\tNow\tPrep\n");
    for(int i=MODEL_MININDEX; i<=MODEL_MAXINDEX; i++) {
        const char *iname = CColorModelHSV::GetIndexName(i);
        g_printBuffer->PrintfIC("  %s\t%3d\t", iname, old_model.GetValue(i)); 
        if(m_prep_set[ch][i]) {
            g_printBuffer->PrintfIC("%3d", m_prep_model[ch].GetValue(i)); 
        }
        g_printBuffer->PrintfIC("\n");
    }
}

bool 
XBCRobot::color_prep_model_value(int ch, int argtype, int val) 
{
    if(ch < 0 || ch >= DV_MODELS || 
       argtype < MODEL_MININDEX || argtype > MODEL_MAXINDEX) {
        // Argument out of bounds, return false
        return(false);
    }
    if(m_prep_model[ch].SetValue(argtype, val)) {
        m_prep_set[ch][argtype]=true; 
        return(true);
    }
    else {
        return(false);
    }
}

bool 
XBCRobot::color_set_prep_model_from_flash(int ch) 
{
    if(m_vMenu==NULL) {
        return(false);
    }

    CColorModelHSV model;
    if(m_vMenu->m_flashHandler.GetSavedModel(ch, 
                                             model)) {
        m_prep_model[ch]=model;
        for(int i=MODEL_MININDEX; i<=MODEL_MAXINDEX; i++) {
            m_prep_set[ch][i]=true;
        }
    }
    return(true);
}
int 
XBCRobot::color_check_prep_model(int ch) 
{
    CColorModelHSV &m = m_prep_model[ch];
    
    CColorModelHSV old_model;
    m_vision->GetCurrentModel(ch, old_model);
    
    for(int i=MODEL_MININDEX; i<=MODEL_MAXINDEX; i++) {
        if(!m_prep_set[ch][i]) {
            // This value isn't set, if it's VMAX or SMAX, 
            // use MAX_V or MAX_S
            if(i == SMAX_INDEX) {
                m.sMax = MAX_S;
                m_prep_set[ch][i]=true;
            }
            else if(i == VMAX_INDEX) {
                m.vMax = MAX_V; 
                m_prep_set[ch][i]=true;
            }
            else {
                // Keep the value the same as before
                m.SetValue(i, 
                           old_model.GetValue(i));
            }
        }
    }
    // Check to make sure the values are ok
    if(m.GetHueRange()>MAX_HRANGE || 
       m.sMin>m.sMax || m.sMin<0 || m.sMax>MAX_S ||
       m.vMin>m.vMax || m.vMin<0 || m.vMax>MAX_V) {
        m_prep_status[ch]=COLOR_CHECK_BAD;
    }
    m_prep_status[ch]=COLOR_CHECK_OK;
    return(m_prep_status[ch]);
}

int
XBCRobot::color_apply_model_partial(int ch, int partBegin, int partEnd) 
{

    if(ch < 0 || ch >= DV_MODELS || 
       partBegin<0 || partBegin>32/*CVision::MODEL_PARTS*/ || 
       partEnd<0 || partEnd>32/*CVision::MODEL_PARTS*/+1) {
        // Argument out of bounds, return -1
        return(-1);
    }

    // Check to make sure all the values are set
    if(m_prep_status[ch] == COLOR_CHECK_UNKNOWN) {
        color_check_prep_model(ch);
    }

    // At this point we know that m_prep_status[ch] is either good or bad
    // since color_check_prep_model(ch) will set it to one or the other
    if(m_prep_status[ch]==COLOR_CHECK_BAD) {
        // It's bad, return -1
        return(-1);
    }               

    // It's good, apply the model
    return(m_vision->UploadModelPartial(ch, m_prep_model[ch], 
                                        partBegin, partEnd));
}
void
XBCRobot::color_wrapup_model_update(int ch)
{
    //g_printBuffer->PrintfIC("Checking channel stats ok\n");
    if(ch >= 0 && ch < DV_MODELS && 
       m_prep_status[ch] == COLOR_CHECK_OK) {
        //g_printBuffer->PrintfIC("Checking for model context\n");
        // Args are in bounds and the prep color model for that channel 
        // was good.  If currently showing a model context update it
        IDispContext *dcon = DispContextStackSingleton.GetCurrentContext();
        CModelContext *mcon = dynamic_cast<CModelContext*>(dcon);
        if(mcon!=NULL) {
            // Currently showing model context, update it
            //g_printBuffer->PrintfIC("  Yes, syncing\n");
            mcon->SyncModelFromVision();
        }
        else {
            //g_printBuffer->PrintfIC("  No, ignoring\n");
        }
    }
    m_prep_status[ch] = COLOR_CHECK_UNKNOWN;
}

#endif


#ifdef ICLANGAL_EXPERIMENTAL

void XBCRobot::StartTimer(unsigned char timer_id, long ms)
{
	// 16387 incs/s
	// 65535 incs
	// For 0.001s, 16387incs/s

	// If ms <= 4000, use a single timer
	// If 4000 < ms < 65536, use a double timer with 1ms precision
	// If 65535 < ms < 131072, use a double timer with 2ms precision
	// If 131070 < ms < 262144, use a double timer with 4ms precision
	// Past that... not hard to add, but what's the point?

	unsigned short timer_a_init;
	unsigned short timer_a_
	unsigned short timer_b_

	if(ms <= 4000)
	{
		GBA_REG_TM0D = 0x10000 - (ms * 16384 / 1000);
	}
	else if(ms <= 65536)
	{
		GBA_REG_TM0D = 0x10000 - (16384 / 1000);
		GBA_REG_TM1D = 0x10000 - ms;
	}
	else if(ms <= 131072)
	{
		GBA_REG_TM0D = 0x10000 - (2 * 16384 / 1000);
		GBA_REG_TM1D = 0x10000 - (ms >> 1);
	}
	else if(ms <= 262144)
	{
		GBA_REG_TM0D = 0x10000 - (4 * 16384 / 1000);
		GBA_REG_TM1D = 0x10000 - (ms >> 2);
	}



	//g_Timers[1].clockCount = 0xFC18;
	GBA_REG_TM1D = 0xFC18;
	//g_Timers[0].clockSpan = TIMER_FREQ_1;
	BFSET(GBA_REG_TM0CNT, clockSpan, TIMER_FREQ_1);
	//g_Timers[0].intOnOverflow = 0x0;
	BFSET(GBA_REG_TM0CNT, intOnOverflow, 0x0);
	//g_Timers[0].incOnOverflow = 0x0;
	BFSET(GBA_REG_TM0CNT, incOnOverflow, 0x0);
	//g_Timers[0].enableTimer = 0x1;
	BFSET(GBA_REG_TM0CNT, enableTimer, 0x1);
	//g_Timers[1].intOnOverflow = 0x0;
	BFSET(GBA_REG_TM1CNT, intOnOverflow, 0x0);
	//g_Timers[1].incOnOverflow = 0x1;
	BFSET(GBA_REG_TM1CNT, incOnOverflow, 0x1);
	//g_Timers[1].enableTimer = 0x1;
	BFSET(GBA_REG_TM1CNT, enableTimer, 0x1);
	//g_Timers[2].intOnOverflow = 0x0;
	BFSET(GBA_REG_TM2CNT, intOnOverflow, 0x0);
	//g_Timers[2].incOnOverflow = 0x1;
	BFSET(GBA_REG_TM2CNT, incOnOverflow, 0x1);
	//g_Timers[2].enableTimer = 0x1;
	BFSET(GBA_REG_TM2CNT, enableTimer, 0x1);
	//g_Timers[2].clockCount = 0x0;
	GBA_REG_TM2D = 0x0;
	//g_Timers[0].clockCount = 0xBE76;
	GBA_REG_TM0D = 0xBE76;
}

void XBCRobot::ICRegister(unsigned char vector)
{
	m_ic_vectors[vector] = 1;
	
	m_orig_vectors[vector] = m_pIntCont->m_vectors[vector];
	
	m_pintCont->Register(this, vector);
	m_pintCont->Unmask(this, vector);
}

void XBCRobot::ICUnRegister(unsigned char vector)
{
	m_ic_vectors[vector] = 0;
	
	m_pIntCont->Register(m_orig_vectors[vector], vector);
}

void XBCRobot::Interrupt(unsigned char vector)
{
	// Do the standard stuff first
	ICRobot::Interrupt(vector);
	
	if(m_ic_vectors[vector])
	{
		m_ic_vector_triggered = 1;
	}
}

void XBCRobot::ResetInterrupt(unsigned char vector)
{
	m_ic_vector_triggered = 0;
}

#endif


//#ifndef NO_IR_TRACK
	void XBCRobot::Periodic()
//#else
//	inline void XBCRobot::Periodic()
//#endif
{
	#ifndef NO_IR_TRACK
		
		//static short ir_track_counter;
		
	#endif

	// Do everything normal first
	ICRobot::Periodic();
	
	// GetPWM logs
	// 0x20 values are stored, this is 160ms of data
	for(int axis_count = 0; axis_count < 4; axis_count++)
	{
		getpwm_logs[axis_count][getpwm_index] = GetPWM(axis_count);
	}
	getpwm_index = (getpwm_index + 1) & 0x1F; // equivalent to % 0x20, but faster
	
	#ifndef NO_IR_TRACK
		#if 0
		// The IR Rangefinder only updates every 38.3ms; 
		// no point wasting processing time / RAM logging at a higher frequency.
		if(ir_track_counter == 0)
		{
			// The user has to enable IR Tracking, since it uses a fair bit of CPU time
			if(ir_track_active)
			{
				// We've overflowed the array!
				if(ir_track_index >= IR_TRACK_ARRAY_SIZE)
				{
					PrintToICBuffer("IRTrack Overflow!\n");
				}
				
				// If we've finished the sweep or run out of storage, stop
				if(ir_track_index >= IR_TRACK_ARRAY_SIZE /*|| Done(ir_track_motor_port)*/)
				{
					// debug
					//SadBeep();
					//if(Done(ir_track_motor_port))
					//{	
					//	HappyBeep();SadBeep();
					//}
					// end debug
					
					//Stop(ir_track_motor_port);
					//Execute();
					
					if(ir_track_index >= IR_TRACK_ARRAY_SIZE) // this if is just for debug
					{
					//	HappyBeep(); HappyBeep(); HappyBeep();
						ir_track_active = false;
					}
				}
				else
				{
					// Log the position and range data, and increment the array index
					// trying to comment these out to see if there's a problem with masking
					//ir_track_positions[ir_track_index] = GetPosition(ir_track_motor_port);
					ir_track_positions[ir_track_index] = m_position[ir_track_motor_port];
					
					//ir_track_ranges[ir_track_index] = IRRange(ir_track_ir_port);
					//ir_track_ranges[ir_track_index] = GetAnalog(ir_track_ir_port);
					//ir_track_ranges[ir_track_index] = (short)(pow(GetAnalog(ir_track_ir_port)/16.0, -1.078867)*21417.2055);
					ir_track_ranges[ir_track_index] = IRRangeFast(ir_track_ir_port);
					
					ir_track_index++;
				}
			}
		}
		
		// Deals with the IR Tracking Frequency
		ir_track_counter = (ir_track_counter + 1) % IR_TRACK_FREQ;
		#endif
		
	#endif
}

#ifndef NO_IR_TRACK

void XBCRobot::Interrupt(unsigned char vector)
{
	if(vector == 20) // if it's a BackEMF Interrupt
	{
		ICRobot::Interrupt(vector);
	}
	else if(vector == 0) // VBlank Interrupt
	{
		//HappyBeep();
		
		// The IR Rangefinder only updates every 38.3ms; 
		// no point wasting processing time / RAM logging at a higher frequency.
		if(ir_track_counter == 0)
		{
			// The user has to enable IR Tracking, since it uses a fair bit of CPU time
			if(ir_track_active)
			{
				// We've overflowed the array!
				if(ir_track_index >= IR_TRACK_ARRAY_SIZE)
				{
					PrintToICBuffer("IRTrack Overflow!\n");
				}
				
				// If we've finished the sweep or run out of storage, stop
				if(ir_track_index >= IR_TRACK_ARRAY_SIZE /*|| Done(ir_track_motor_port)*/)
				{
					// debug
					//SadBeep();
					//if(Done(ir_track_motor_port))
					//{	
					//	HappyBeep();SadBeep();
					//}
					// end debug
					
					//Stop(ir_track_motor_port);
					//Execute();
					
					if(ir_track_index >= IR_TRACK_ARRAY_SIZE) // this if is just for debug
					{
					//	HappyBeep(); HappyBeep(); HappyBeep();
						ir_track_active = false;
					}
				}
				else
				{
					// Log the position and range data, and increment the array index
					// trying to comment these out to see if there's a problem with masking
					//ir_track_positions[ir_track_index] = GetPosition(ir_track_motor_port);
					ir_track_positions[ir_track_index] = m_position[ir_track_motor_port];
					
					//ir_track_ranges[ir_track_index] = IRRange(ir_track_ir_port);
					//ir_track_ranges[ir_track_index] = GetAnalog(ir_track_ir_port);
					//ir_track_ranges[ir_track_index] = (short)(pow(GetAnalog(ir_track_ir_port)/16.0, -1.078867)*21417.2055);
					ir_track_ranges[ir_track_index] = IRRangeFast(ir_track_ir_port) + ir_track_radius_offset;
					
					ir_track_index++;
				}
			}
		}
		
		// Deals with the IR Tracking Frequency
		ir_track_counter = (ir_track_counter + 1) % IR_TRACK_FREQ;
	}
}

void XBCRobot::IRTrackSetMotor(unsigned char port)
{
	ir_track_motor_port = port;
}

void XBCRobot::IRTrackSetSensor(unsigned char port)
{
	ir_track_ir_port = port;
}

void XBCRobot::IRTrackStart(short velocity, long new_pos)
{
	// Notes... if we want 0.5deg precision, and we're doing a 30deg sweep, 
	// we need 60 measurements * 38.3ms/measurement = 2298ms for full sweep.
	// Since we round to the nearest 5ms, we need 60 measurements * 40ms/measurement = 2400ms for a full sweep.
	// This is reasonably fast, assuming the CPU can keep up.
	
	if(!ir_track_positions)
	{
		ir_track_positions = (long *)malloc(IR_TRACK_ARRAY_SIZE*sizeof(long));
		if(ir_track_positions == NULL)
		{
			PrintToICBuffer("OUT OF MEMORY!\nPOSITION ALLOCATION\n");
			SadBeep();
			return;
		}
	}
	if(!ir_track_ranges)
	{
		ir_track_ranges = (short *)malloc(IR_TRACK_ARRAY_SIZE*sizeof(short));
		if(ir_track_ranges == NULL)
		{
			PrintToICBuffer("OUT OF MEMORY!\nRANGE ALLOCATION\n");
			SadBeep();
			return;
		}
	}


	// If we interrupted while this was in progress, we'd screw things over
	//m_pIntCont->Mask(m_vector);

	ir_track_active = true;
	ir_track_index = 0;
		
	ir_track_process_index = 0;
	ir_track_current_blob = 0;

	for(int count=0; count<IR_TRACK_MAX_BLOBS; count++)
	{
		ir_track_blob_start[count] = ir_track_blob_end[count] = 0;
		ir_track_blob_started[count] = ir_track_blob_closed[count] = false;
	}
	
	// Setup a VBlank interrupt
	m_pIntCont->Register(this, 0);
	m_pIntCont->Unmask(0);
	BFSET(GBA_REG_DISPSTAT, enableVBInt, 1);
	
	// Start the sweep actuator
	//IcMove(ir_track_motor_port, new_pos,
	//	   velocity, m_motionAccel[ir_track_motor_port]);
	
	// Okay, it's safe to continue now...
	//m_pIntCont->Unmask(m_vector);
	
	// for debug
	//while(Done(ir_track_motor_port));
}

void XBCRobot::IRTrackStop()
{
	BFSET(GBA_REG_DISPSTAT, enableVBInt, 0);
	ir_track_active = false;
	m_pIntCont->Mask(0);
}

void XBCRobot::IRTrackProcess()
{
	if(ir_track_process_index >= IR_TRACK_MAX_BLOBS)
	{
		return;
	}

	// all of the data that isn't yet processed
	for(; ir_track_process_index < ir_track_index; ir_track_process_index++)
	{
		// if we're looking to start a blob
		if(! ir_track_blob_started[ir_track_current_blob]) 
		{
			// if we see something closer than the horizon
			if(ir_track_ranges[ir_track_process_index] <= ir_track_max_horizon)
			{
				// mark the blob as started
				ir_track_blob_started[ir_track_current_blob] = true;
				
				// mark the current index as the start and end
				ir_track_blob_start[ir_track_current_blob] = ir_track_blob_end[ir_track_current_blob] = ir_track_process_index;
				
				continue;
			}
		}
		// if we're looking at a blob but we haven't closed it yet
		else if(ir_track_blob_started[ir_track_current_blob] && ! ir_track_blob_closed[ir_track_current_blob])
		{
			// Difference in the current range and the prevously comfirmed range
			// -1 because the depth detection includes outside the blob
			short depth = ir_track_ranges[ir_track_process_index] - ir_track_ranges[ir_track_blob_end[ir_track_current_blob]-1];
			
			// We've found a blob boundary
			// Either the max depth was exceeded or the max horizon was exceeded
			if(depth > ir_track_max_depth || depth < -ir_track_max_depth || ir_track_ranges[ir_track_process_index] > ir_track_max_horizon)
			{
				ir_track_blob_closed[ir_track_current_blob] = true;
				// -1 because we're already 1 reading outside the blob
				ir_track_blob_end[ir_track_current_blob] = ir_track_process_index -1;
				
				// Check stats to verify that a valid blob was found
				// E.g. check width
				// if the blob is too small
				if(ir_track_positions[ir_track_blob_end[ir_track_current_blob]]-ir_track_positions[ir_track_blob_start[ir_track_current_blob]] < ir_track_min_angular_width
				|| ir_track_positions[ir_track_blob_end[ir_track_current_blob]]-ir_track_positions[ir_track_blob_start[ir_track_current_blob]] > -ir_track_min_angular_width)
				{
					ir_track_blob_started[ir_track_current_blob] = ir_track_blob_closed[ir_track_current_blob] = false;
					continue;
				}
				
				ir_track_current_blob++;
				
				if(ir_track_current_blob > IR_TRACK_MAX_BLOBS)
				{
					return;
				}
				
				// pre-check the next value... oops, or not
				if(ir_track_ranges[ir_track_process_index] <= ir_track_max_horizon)
				{
					// mark the blob as started
					ir_track_blob_started[ir_track_current_blob] = true;
					
					// mark the current index as the start and end
					ir_track_blob_start[ir_track_current_blob] = ir_track_blob_end[ir_track_current_blob] = ir_track_process_index;
				}
			}
		}				
	}
}

/*
Notes on IRTrack:
	To divide the map into blobs, have two user-supplied values: depth tolerance and max slope.
	
	When we process a reading:
		Check for the following conditions:
			Slope from previous 0-true reading to current reading is sharper than max slope
			Depth distance from previous 0-true reading to current reading is greater than depth tolerance
		If 2 of the above are true:
			Blob is over, end it at the last reading where 0 were true
		If 1 of the above is true:
			Continue processing, leave existing edge of blob
		If 0 of the above are true:
			Continue processing, extend edge of blob to this reading
	
	Benefits of max slope vs depth tolerance... need to investigate... max slope may be more robust,
	or we may need both.
	
	Depth tolerance on its own is screwed up by intermediate values, max slope on its own probably isn't.
		This is because the delta depth will be much smaller when it sees an intermediate value
	Max slope on its own is screwed up when looking for a tribble stack, as it may instead see 3 tribbles.
		This is because even though the depth distance between the inner tribble and an outer tribble is minimal,
		the slope will be quite high.

	Addendum 2009 01 01: We're going to ignore slope after all, for 3 reasons.
		1. Slope calculation requires division, which is slow
		2. Slope calculation is unreliable at finding blobs in Excel testing
		3. Depth calculation worked more or less fine on its own in Excel testing
		
		Depth will be relative to start of blob, not previous value.
		This eliminates issues with intermediate values.
	
*/

short XBCRobot::IRRange(short port)
{
	// Float error only affects the reading to under a micrometer; 
	// the rangefinder is only accurate to about a centimeter;
	// thus, don't bother with double-precision.

	short et_value = GetAnalog(port);
    float et_value_8_bit = et_value / 16.0;
    float cm = 2141.72055*pow(et_value_8_bit, -1.078867);
    short mm = (short)(cm * 10.0);
    return(mm);
}

short XBCRobot::IRRangeFast(short port)
{
	// Uses a lookup table instead of float operations.
	// Cut down a 1000-operation loop from 17139ms to only 12ms.

	short et_value = GetAnalog(port);
	return(table_ir_range[et_value]);
}

void XBCRobot::IRRangeBenchmark(short port)
{
	CSimpTimer bench_timer;
	unsigned long long init_time;
	unsigned long long final_time;
	
	short range;
	
	bench_timer.GetCount(&init_time);
	
	for(int count = 0; count < 1000; count++)
	{
		range = IRRange(port);
	}
	
	bench_timer.GetCount(&final_time);
	
	PrintToICBuffer("IRRange (float)\n1000 iterations\n%dms\n", (final_time - init_time)/1000);
	
	// We have the float stats, now we do the table stats
	
	bench_timer.GetCount(&init_time);
	
	for(int count = 0; count < 1000; count++)
	{
		range = IRRangeFast(port);
	}
	
	bench_timer.GetCount(&final_time);
	
	PrintToICBuffer("\nIRRangeFast (table)\n1000 iterations\n%dms\n", (final_time - init_time)/1000);
}

void XBCRobot::IRTrack_Display(long ticks_at_0_rad, long ticks_at_pi_rad, long mm_per_screen_height)
{
	// Displays a polar map of the surroundings (quadrants 1 and 2)

	short loop_index;
	long angle1, angle2;
	short r1, r2, x1, x2, y1, y2;

	// Draw blank frame
	CallML1Translator(1100, 0);
	
	// Test data
	// motor position, -1000 to 1000
	// ticks at 0 = -1500, ticks at pi = 1500
	// final angles, 0 to pi
	// so the positions -1000 to 1000 end up as 1/6*pi, 5/6*pi
	
	// Calculate the angle (milliradians) and radius (pixels)
	angle1 = (long) (MapRange((long long)ir_track_positions[0], (long long)ticks_at_0_rad, (long long)ticks_at_pi_rad, (long long)0, (long long)3142));
	r1 = (long)(ir_track_ranges[0]) * 160 / mm_per_screen_height;
	// Do some trig to find the x,y coords of the reading, and have the origin at the bottom of the screen
	x1 = ((r1 * CosLut(angle1)) >> 10) + 120;
	y1 = 160 - ((r1 * SinLut(angle1)) >> 10);
	
	for(loop_index = 1; loop_index < ir_track_index; loop_index++)
	{
		// Calculate the angle (milliradians) and radius (pixels)
		angle2 = (long) (MapRange((long long)ir_track_positions[loop_index], (long long)ticks_at_0_rad, (long long)ticks_at_pi_rad, (long long)0, (long long)3142));
		r2 = (long)(ir_track_ranges[loop_index]) * 160 / mm_per_screen_height;
		// Do some trig to find the x,y coords of the reading, and have the origin at the bottom of the screen
		x2 = ((r2 * CosLut(angle1)) >> 10) + 120;
		y2 = 160 - ((r2 * SinLut(angle1)) >> 10);
		
		// Check if both points are within our field of view
		if(x1>=0 && x1<240 && x2>=0 && x2<240 && y1>=0 && y1<160 && y2>=0 && y2<160)
		{
			// Draw it!
			GbaScreenImage.draw_line(x1, y1, x2, y2, 0x0);
		}
		
		// Prepare for the next loop
		angle1=angle2;
		r1=r2;
		x1=x2;
		y1=y2;
	}
	
	
}

void XBCRobot::IRTrackDump()
{
	m_commDevice->Write((char *)&ir_track_index,sizeof(short));
	
	m_commDevice->Write((char *)ir_track_positions, ir_track_index * sizeof(long));
	m_commDevice->Write((char *)ir_track_ranges, ir_track_index * sizeof(short));
	
}

#endif

double XBCRobot::MapRange(double x, double start1, double end1, double start2, double end2)
{
	// First x's range is start1, end1
	
	x -= start1;
	
	// x range is 0, end1-start1
	
	x *= end2-start2;
	
	// x range is 0, (end1-start1)*(end2-start2)
	
	x /= end1-start1;
	
	// x range is 0, end2-start2
	
	x += start2;
	
	// x range is start2, end2
	
	return(x);
}

long XBCRobot::MapRange(long x, long start1, long end1, long start2, long end2)
{
	// First x's range is start1, end1
	
	x -= start1;
	
	// x range is 0, end1-start1
	
	x *= end2-start2;
	
	// x range is 0, (end1-start1)*(end2-start2)
	
	x /= end1-start1;
	
	// x range is 0, end2-start2
	
	x += start2;
	
	// x range is start2, end2
	
	return(x);
}

long long XBCRobot::MapRange(long long x, long long start1, long long end1, long long start2, long long end2)
{
	// First x's range is start1, end1
	
	x -= start1;
	
	// x range is 0, end1-start1
	
	x *= end2-start2;
	
	// x range is 0, (end1-start1)*(end2-start2)
	
	x /= end1-start1;
	
	// x range is 0, end2-start2
	
	x += start2;
	
	// x range is start2, end2
	
	return(x);
}

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//    indent-tabs-mode: nil
//   End

