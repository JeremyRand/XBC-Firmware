#ifndef ICROBOT_H
#define ICROBOT_H

// The following defines allow conditionally enabling (if commented
// out) or disabling (if uncommented) particular sets of functionality
// in ICRobot
//#define NO_GPIO
//#define NO_MOTOR
//#define NO_VISION
#define NO_ICXVIEW
//#define USE_OWN_CLASS
//#define USE_SIM
//#define NO_IR_TRACK

// WARNING: Before enabling audio files, make sure that IC_FLASH_ADDR (below) is after end of firmware binary.
//#define AUDIO_AHH
//#define AUDIO_BLUE
//#define AUDIO_HOKEY
//#define AUDIO_MUCHACHA
//#define AUDIO_OUCH
//#define AUDIO_PRADO
//#define AUDIO_FOURBYTESOUND
//#define AUDIO_DEATH
//#define AUDIO_INTRO
//#define AUDIO_MUNCH
#define AUDIO_SIREN
#define AUDIO_WARNING
//#define AUDIO_READY
//#define AUDIO_GO

//#define LICENSEE "Kirby's Tribble Challenge,\nNorman North High School"
//#define LICENSEE "Team Bandbot,\nNorman North High School"
//#define LICENSEE "La Jolla High School\nBotball 2007"
//#define LICENSEE "Norman North High School\nBotball 2008"
//#define LICENSEE "Norman High School\nBotball 2009"
#define LICENSEE "Public Release\nReleased Feb. 2011"
//#define BGCOLOR 0x0000
#define BGCOLOR 0xFFFF
//#define FGCOLOR 0x021F
#define FGCOLOR 0x0000
//#define INVERTCOLORS
//#define INVERTLUMCOLORS

// Flash memory constants
// An IC program is 0x10000 bytes, but the Flash Sector Size is 0x20000.
// ToDo: Copy current contents of sector to RAM before programming Flash, so that we can use 2 programs per sector.
// ADDR was 0x8080000 in KIPR version
// ADDR was 0x8140000 in version with multiple wavs.
// We've reversed it as of 2008 01 19, due to variable firmware size causing earlier slots to be erased.
//#define IC_FLASH_ADDR ((0x8080000 + (long)m_flashSlot * 0x20000 < 0x8180000) ? (0x8080000 + (long)m_flashSlot * 0x20000) : (0x8080000 + (long)(m_flashSlot+1) * 0x20000) )
#define IC_FLASH_ADDR ((0x8140000 + (long)(MAX_FLASH_SLOT-m_flashSlot) * 0x20000 < 0x8180000) ? (0x8140000 + (long)(MAX_FLASH_SLOT-m_flashSlot) * 0x20000) : (0x8140000 + (long)((MAX_FLASH_SLOT-m_flashSlot)+1) * 0x20000) )
#define IC_FLASH_SIZE 65536
#define MAX_FLASH_SLOT 16
// Can't have a slot at 0x8180000 (where the camera config is); adjust as appropriate to start of flash slots
// Slots are:
// 808, 80A, 80C, 80E, 810, 812, 814, 816, 81A, 81C, 81E, 820, 822, 824, 826, 82A, 82C, 82E, 830, 832, 834, 836, 83A

#include "CCommunicationDevice.h"
#include "CStatusBar.h"
//#include "axesc.h"
#include "diffbase.h"
#include "menudisp.h"
#include "palette.h"
#include "display.h"
#include "flash.h"
#include "btnstate.h"
#include "tone.h"
#include "simptimer.h"

#ifndef NO_ICXVIEW
#include "ICXView.h"
#endif

typedef short          int16;
typedef unsigned short intp;
typedef long           int32;
typedef float          float32;

typedef unsigned short uint16;
typedef unsigned long  uint32;

#ifdef USE_SIM
#define NO_GPIO
#define NO_MOTOR
#define NO_VISION
#endif

//////////////////////////////////////////////////////////////////////
// Motor defaults
#define NUM_MOTORS 4
//#define IC_BEMF_DIV 20
#define IC_BEMF_DIV m_BemfDiv
#define DEFAULT_MOTION_ACCEL 100000/IC_BEMF_DIV
#define MAX_MOTOR_POS 0x7f0000/IC_BEMF_DIV

class ICRobotContext;

//////////////////////////////////////////////////////////////////////
// 16x16 bit icons for IC
class CICIcon {
public:
  CICIcon() {}
  ~CICIcon() {}

  // Given the row and column tile number of the of the top left
  // corner of where the icon is in the source image (tile number is
  // pixels position/8) this method fills in member variables with the
  // correct tile numbers for the icon
  void ReadFromMap(int srcRow, int srcCol);

  // Given the row and column of the destination for the top left
  // corner of the icon this menthod draws it to BG2, which is the IC
  // frame
  void WriteToBG(int destRow, int destCol);

  static unsigned short GetICMapTile(int row, int col);
  static void SetICBgTile(int row, int col, unsigned short val);

public:
  unsigned short tiles[2][2];
};

//////////////////////////////////////////////////////////////////////
class ICRobot : public IMenuHandler
#ifndef NO_MOTOR
//	, public CAxesClosed
	, public CDiffBase
#endif		
	//, public CXirGpio
{
	friend class ICRobotContext;
	friend class ICRobotPBufContext;
#ifdef USE_OWN_CLASS
	friend class NonPcode;
#endif

public:
	enum EICCodeState {
		IC_CODE_MISSING,
		IC_CODE_DOWNLOADED,
		IC_CODE_RUNNING, 
		IC_CODE_STOPPED
	};

protected:
	CSimpTimer m_timer;
	CMenuDisp m_menuSystem;
	CMenuElement *progElem;
	CPalette  *m_palette;
	CDispFont *m_font;
	CTileSet  *m_bgTiles;
	CFlash m_flashMem;
	CBtnState m_buttonState;
	CStatusBar m_statusBar;
	CMenuItem* m_icProgramItem;
	CMenuItem* m_icProgramFlashItem[MAX_FLASH_SLOT+1];
	bool m_inCompetitionMode;
	bool m_bIsDormant;
	bool m_bWasDormant;
	bool m_icOwnsBtns;
	bool m_UartModeIC;

	int m_BemfDiv;

        int m_heartCount;
        int m_heartPhase;
	long m_heartChangeTime;

        CICIcon m_blankIcon, m_waitIcon, m_logoIcon;
        CICIcon m_heartIcons[2];
        CICIcon m_downloadIcon, m_runIcon, m_stopIcon;
        EICCodeState m_codeState;
        CICIcon m_connIcon, m_disconnIcon;
        static bool m_useDisplay;
        unsigned int m_motionAccel[NUM_MOTORS];

	void CheckForConnection();
	void AttemptBluetoothConnect();
	void CheckIfDormant();
	void TranslatePacket();
	void RunICProcess();
	void SendReply(int iSequence, int iLength = 0, char* chData = 0);

	bool CheckFlashMemState();
	void CopyMemToFlash();
	void CopyMemFromFlash();
	void ClearMemFromFlash();
	void SetFlashSlot(unsigned short slot_num);
	
	void PrintProcessDisplay();

	virtual unsigned short 
	getDigitalSensorValue(const unsigned short sensorPort) { return 0; }

	CCommunicationDevice* m_commDevice;

        long unsigned m_rand_x;
        long unsigned m_rand_c;

	unsigned char m_ledStatus;
	//CInterruptCont intContDSound;
	CTone m_tone;
	
	unsigned short m_flashSlot;
public:
	ICRobot(unsigned short vector);
	virtual ~ICRobot();

	////////////////////////
	// This is called once per system loop.  It checks comm,
	// updates the heartbeat, and runs the pcode for a while
	virtual void RunRobotLoop();
	
	////////////////////////
	// Menu
	virtual void CreateMenuTree();
	virtual bool HandleMenuEvent(int eventType, CMenuElement &menu);
	// Support for accessing menu system outside class
	CMenuDisp &GetMenuSystem();

	////////////////////////
	// Callml
	virtual short CallML1Translator(const short& functionIndex, 
					const short& argument1,
					void * pointer = 0);
	virtual short CallML2Translator(const short& functionIndex, 
					const short& argument1, 
					const short& argument2,
					void * pointer = 0,
					void * pointer2 = 0);
	virtual short CallML3Translator(const short& functionIndex, 
					const short& argument1, 
					const short& argument2, 
					const short& argument3,
					void * pointer = 0);
	virtual unsigned short 
		GetDigital(const unsigned short sensorPort) { return 0; }

	////////////////////////
	// IC Program state
	void StartICProgram();
	void StopICProgram();
	void ToggleICProgramState(bool reqAtoStart=false);
	bool IsDormant() { return m_bIsDormant;	}
	EICCodeState GetCodeState() { return(m_codeState); }
	bool IsLibLoaded();
	bool IsMainLoaded();
	bool IsMainLoadedFlash();
	const char *GetProgramName();
	const char *GetProgramNameFlash();
	void UpdateProgramMenuName();
	unsigned int GetFlashMagic();
	unsigned char GetFlashCodeStatus();
	void SetFlashCodeStatus(unsigned char status);

	////////////////////////
	// Buttons->IC Program

	// This controls whether or not buttons should be able to be
	// read by an IC program.  This is set and cleared by
	// ICRobotPBufContext depending on whether the IC Console is
	// showing or hidden
	void SetIcOwnsBtns(bool val);

	bool GetIcOwnsBtns() {
		return(m_icOwnsBtns);
	}

	////////////////////////
	// Context switching support 
	void SetupDisplay();
	void TeardownDisplay() {
		m_useDisplay=false;
	}
	static bool UsingDisplay() { return(m_useDisplay); }
	
	////////////////////////
	// Status Bar
	void UpdateStatusBar();
	void ChangeStatusBarPage(int dir);
	CStatusBar &GetStatusBar() { return(m_statusBar); }

#ifndef NO_MOTOR
	////////////////////////
	// Motor wrappers which scale everything by IC_BEMF_DIV on the
	// way in and out
	void IcMoveVelocity(unsigned char axis,
			    int velocity, unsigned int acceleration);
	void IcMoveVelocityTicks(unsigned char axis,
			int velocity, unsigned int acceleration);
	void IcMove(unsigned char axis,
		    AxisPosition endPosition, int velocity, 
		    unsigned int acceleration);
	void IcMoveTicks(unsigned char axis,
		AxisPosition endPosition, int velocity, 
		unsigned int acceleration);
	AxisPosition IcGetPosition(unsigned char axis);
	AxisPosition IcGetPositionTicks(unsigned char axis);
	AxisPosition IcGetPositionRaw(unsigned char axis);
	void         IcSetPosition(unsigned char axis, AxisPosition pos);
	void IcSetPositionTicks(unsigned char axis, AxisPosition pos);
	void IcSetPositionRaw(unsigned char axis, AxisPosition pos);
	void IcMovePWM(unsigned char axis, short pwm);
	void IcMoveStop(unsigned char axis);
	void IcMoveFreeze(unsigned char axis, unsigned int acceleration);

	unsigned short GetScaledAnalog(unsigned short sensorPort);

#endif


	////////////////////////
	// Icons
	void InitializeIcons();

	// Heartbeat
	void ShowHeartbeatPhase(int phase);
	void UpdateHeartbeat();

	// Code State WARNING: These are also what sets m_codeState
	void ShowCodeDownloaded();
	void ShowCodeRunning();
	void ShowCodeStopped();
	void ShowCodeState(EICCodeState state);
	// This reads m_codeState and updates the icon display accordingly
	void ShowCurrentCodeState();

	// Serial
	void ShowSerialState(bool connected, bool showIcon);
	void ShowCurrentSerialState();

#ifndef NO_GPIO
	//Encoder functions
	virtual unsigned short SetEncoderPort(unsigned short sensorPort) { return 0; }
	virtual void UnsetEncoderPort(unsigned short sensorPort) { return; }
	virtual void ResetEncoderCount(unsigned short sensorPort) { return; }
	virtual unsigned int GetEncoderCount(unsigned short sensortPort) { return 0; }

	//Sonar functions
	virtual unsigned short SetSonarPort(unsigned short sensorPort) { return 0; }
	virtual void SendSonarRing(unsigned short sensorPort) { return; }
	virtual unsigned int GetSonarBlocking(unsigned short sensorPort) { return 0; }
	virtual unsigned int GetSonarTime(unsigned short sensorPort) { return 0; }

	virtual void SetPortDirection(unsigned short sensorPort, unsigned char portDirection) = 0;
	virtual void SetPortValue(unsigned short sensorPort, unsigned char portValue) = 0;
	virtual int GetPortDirection(unsigned short sensorPort) = 0;
#endif
	
#ifndef NO_SERIAL
	int SetSerialMode(int baudrate);
	int GetSerialMode();
#endif

#ifndef NO_MOTOR
	float GetBatteryVoltage() { return (float)GetAnalog(7)*0.00214; }
#endif
	virtual unsigned char GetServoPosition(unsigned char servo) { return 0; }

	void HappyBeep();
	void SadBeep();
	int GenerateRandom();
	int GenerateRandomInRange(int x);
	void SeedRandom(int x);
};

#endif 

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 8
//    c-basic-offset: 8
//   End:
