#include "ICRobot.h"
#include "CInterruptContSingleton.h"
#include "pcodesim.h"
#include <gba.h>
#include <regbits.h>
#include "CPrintBuffer.h"
#include "ICRobotPBufContext.h"
#include <stdlib.h>
#include <string.h>
#include "BackgroundGraphics.h"
#include <textcontext.h>
#include "packstructs.h"
#include <mathutils.h>
#include "simptimer.h"
#include "CBluetoothDevice.h"
#include "CUartDevice.h"

#ifdef USE_OWN_CLASS
#include "nonpcode.h"
#endif

using namespace gba;

#define FONT_BLOCK 0
#define BKGND_BLOCK 2

///////////////////////////////////////////////////////////////////
// IC memory space support for flash save/load state and code state 
// Magic number is placed at mem[0] and indicates if a given piece of
// memory is actually a memory image rather than just uninitialized
// flash
#define IC_MAGIC_ADDR 0
#define IC_MAGIC      0x1c12f00d

// The code status address is an unused address in IC memory space
// which holds a bit field showing the status of at memory space
// 
#define CODE_STATUS_ADDR  0x0137
// Bit 8: 1=valid mem, load from flash; 0=invalid mem, do not load 
#define CS_LIVE_BIT  0x80		
// Bit 7: Reserved for IC to signal whether or not to write memory image 
//        to flash after download, but ic5-win/src/serialstream/serialstream-unix.cppis never actually set in the IC
//        memory space
#define CS_WRITE_TO_FLASH_BIT  0x40		
// Bit 0: 1=library loaded in memory image; 1=library not loaded
#define CS_LIB_LOADED_BIT 0x1
// Bit 1: 1=prog with main loaded in memory image; 1=no main
#define CS_MAIN_LOADED_BIT 0x2

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x)
#endif


bool ICRobot::m_useDisplay = false;


ICRobot::ICRobot(unsigned short vector) : 
#ifndef NO_MOTOR
//CAxesClosed(InterruptContSingleton.InstancePtr(), DAO_BEMF_BASE, vector),
CDiffBase(1000, 6283, InterruptContSingleton.InstancePtr(), DAO_BEMF_BASE, vector),
#endif
m_menuSystem(this), 
m_flashMem(InterruptContSingleton.InstancePtr()),
m_icProgramItem(0),
m_inCompetitionMode(false),
#ifndef USE_OWN_CLASS 
m_bIsDormant(true),
m_bWasDormant(true),
#endif /* #ifndef USE_OWN_CLASS */
  m_icOwnsBtns(false),
  m_heartCount(0),
  m_heartPhase(0), 
m_codeState(IC_CODE_MISSING),
m_ledStatus(0),
m_tone(InterruptContSingleton.InstancePtr())
//m_tone()
{
	printf("Initializing ICRobot\n");

#ifndef NO_MOTOR
	SetBaseEnabled(false);
	SetPIDGains(600, 0, 750);
	m_BemfDiv = 20;
	for(int i=0; i<NUM_MOTORS; i++) {
		m_motionAccel[i] = DEFAULT_MOTION_ACCEL;
	}
	
#endif
	m_palette = new CPalette(CPalette::BKGND, 253, 254);
	m_font = new CDispFont(FONT_BLOCK, *m_palette);
	m_bgTiles = new CTileSet(BKGND_BLOCK, 64, 
		(unsigned short const *)ictiles_Tiles);

	// Setup palette and tiles for background
	// Copy gbx2gba palette
	m_palette->CopyFromBuffer(ictiles_Palette);

	// Modify default palette if requested
	#ifdef INVERTCOLORS
		m_palette->Invert();
	#endif
	#ifdef INVERTLUMCOLORS
		m_palette->InvertValue();
	#endif

	// Modify last two entries for font drawing
	//m_palette->SetColor(253, 0x0); // black for text
	m_palette->SetColor(253, FGCOLOR); // black for text
	//m_palette->SetColor(254, 0xffff); // white for text bg
	m_palette->SetColor(254, BGCOLOR); // white for text bg

	InitializeIcons();

	SetFlashSlot(0);

	printf("  Initializing pcode\n");
	//Initialize the pcode memory space
#ifndef USE_OWN_CLASS
	pcodesim_init();
	//Reset the process table
	pcodesim_reset();
#endif
	/*
	CPcodeInfo pcodeInfo;

	pcodeInfo.setProcessTableStartAddress(0xC300);
	pcodeInfo.setMaxNumberOfProcesses(6);
	pcodeInfo.setFlags(3);
	pcodeInfo.setSmallestAddressDataByte(8);
	pcodeInfo.setDataPointerBytes(16);
	pcodeInfo.setSmallestAddressProgramBits(8);
	pcodeInfo.setProgramPointerBits(16);
	pcodeInfo.setPcodeVersionMajor(5);
	pcodeInfo.setPcodeVersionMinor(0);
	pcodeInfo.setFeatureMask(0);
	pcodeInfo.setProcessTableInfoPointer (0x00C8);
	pcodeInfo.setZeroArray (0x0000);
	pcodeInfo.setTargetSpecificFeatures(0x0000);
	pcodeInfo.setMemoryStart(0x8000);
	pcodeInfo.setMemorySize(0x10000-2);
	pcodeInfo.setMemoryUsage(0x00000003);
	pcodeInfo.setMemoryType(0);
	pcodeInfo.setMemoryTypeDetail(0);
	pcodeInfo.setMemoryTimeToAccess(1);
	pcodeInfo.setMemoryReserved(0);
	pcodeInfo.setMemoryUIStart(0xC200);
	pcodeInfo.setMemoryUISize(0x0100);
	pcodeInfo.setMemoryUIUsage(0x0000000C);
	pcodeInfo.setMemoryUIType(0);
	pcodeInfo.setMemoryUITypeDetail(0);
	pcodeInfo.setMemoryUITimeToAccess(1);
	pcodeInfo.setMemoryUIReserved(0);
	pcodeInfo.setTargetName("Xport Robot");
	//	pcodeInfo.setPcodeInMemory(mem, 0x64);
	//	pcodeInfo.setProcessInfoInMemory(mem, 0xC8);
	*/

	// Initialize code status address to 0x80 because 
#ifndef USE_OWN_CLASS
	mem[CODE_STATUS_ADDR] = 0x80;
	// Setup magic number in first 4 bytes of memory.  Don't use
	// SET_MEM4 because that would swap the bytes and make it
	// annoying to check the state of the flash
	*((int *)(&(mem[IC_MAGIC_ADDR]))) = IC_MAGIC;
#endif /* #ifndef USE_OWN_CLASS */
	CopyPackStructsToICMem();
	CreateMenuTree();
#ifndef USE_OWN_CLASS
	//HappyBeep();
	CopyMemFromFlash();
	//SadBeep();
#endif /* #ifndef USE_OWN_CLASS */

	// m_tone = new CTone(&intContDSound);

#ifdef USE_SIM
	m_timer.StartSystemTimer();
#endif

	m_UartModeIC = true;
}

ICRobot::~ICRobot()
{
	delete m_bgTiles;
	delete m_font;
	delete m_palette;
}

void 
ICRobot::UpdateProgramMenuName()
{
	if(m_icProgramItem == NULL) return;

	if(!IsMainLoaded()) {
		if(IsLibLoaded()) {
			m_icProgramItem->SetName("IC Library Loaded");
		}
		else {
			m_icProgramItem->SetName("No Program Loaded");
		}
	}
	else {
		char mbuf[32];
		char *buffer;
		const char *pname = GetProgramName();
		if(strlen(pname) == 0)
		{
#ifndef USE_OWN_CLASS
			pcodesim_init();
			ClearMemFromFlash();
#endif /* #ifndef USE_OWN_CLASS */
			m_icProgramItem->SetName("Program has no name");
			return;
		}
		if(m_codeState == IC_CODE_RUNNING) {
			strcpy(mbuf, "Stop ");
		}
		else {
			strcpy(mbuf, "Run ");
		}
// 		printf("UpdateProgramMenuName: '%s' '%s'\n", mbuf, pname);
		buffer = mbuf + strlen(mbuf);
		strncpy(buffer, pname, 27-strlen(mbuf));
		mbuf[27]=0;
// 		printf("    %s\n", mbuf);
		m_icProgramItem->SetName(mbuf);
	}
	m_menuSystem.DrawMenu();
}

void ICRobot::CreateMenuTree()
{
	
	if(XP_REG_IDENTIFIER < 0x8d05)
	    {
		
		m_menuSystem.AddMenuItem("WARNING: Old Bitstream", -1);
		m_menuSystem.AddMenuItem("IC may not work correctly", -1);
		m_menuSystem.AddMenuItem("Use XportUtil to upgrade", -1);
	    }
#ifndef USE_OWN_CLASS
	m_icProgramItem = m_menuSystem.AddMenuItem("", 20);
	UpdateProgramMenuName();
	
	progElem = m_menuSystem.AddMenuLink("More Programs", 
		this);
		for(int slot_count=0; slot_count <= MAX_FLASH_SLOT; slot_count++)
		{
			SetFlashSlot(slot_count);
			if(CheckFlashMemState() && *(GetProgramNameFlash()))
			{
				const char *the_name = GetProgramNameFlash();
				
				char buffer[32];
				
				strncpy(buffer, the_name, 27);
				
				buffer[27]=0;
				
				m_icProgramFlashItem[slot_count] = progElem->AddMenuItem(buffer, 30+m_flashSlot);
				
				//m_icProgramFlashItem[slot_count] = progElem->AddMenuItem("", 30+slot_count);
				//m_icProgramFlashItem[slot_count]->SetName(GetProgramNameFlash());
			}
			else 
				m_icProgramFlashItem[slot_count] = NULL;
		}
		SetFlashSlot(0);
#else
	m_menuSystem.AddMenuItem("Run Non-pcode", 75);
	m_menuSystem.AddMenuItem("Transfer Non-pcode", 76);
#endif /* #ifndef USE_OWN_CLASS */
	
	m_menuSystem.AddMenuItem("IC Console", 0);
	m_menuSystem.AddMenuItem("Clear IC Console", 1);
	m_menuSystem.AddMenuItem("Clear Motor Positions", 2);
	CMenuElement *memElem = m_menuSystem.AddMenuLink("Save Menu", 
		this);
		memElem->AddMenuItem("Load IC State from flash", 10);
		memElem->AddMenuItem("Save IC State to flash", 11);
		memElem->AddMenuItem("Clear IC State off flash", 12);
	m_menuSystem.AddMenuItem("", 999);
	m_menuSystem.AddMenuItem("Try Bluetooth", 21);
#ifndef NO_ICXVIEW
	m_menuSystem.AddMenuItem("ICX View", 250);
#endif

	/*m_menuSystem.AddMenuItem("TEST OPTION #1", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #2", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #3", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #4", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #5", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #6", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #7", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #8", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #9", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #10", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #11", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #12", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #13", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #14", 250);
	m_menuSystem.AddMenuItem("TEST OPTION #15", 250);*/

	//m_menuSystem.AddMenuItem("View Process Table", 3);
}

void ICRobot::SetupDisplay()
{
        // Record that it's ok to modify the display between now 
        // and when TeardownDisplay is called
        m_useDisplay = true;
 
	// Load palette.  TODO: What about existing palette if any?
	CDisplay::SetCurrPalette(m_palette);

	// Load font.  TODO: What about existing palette if any?
	CDisplay::SetCurrDispFont(m_font);

	// Setup menu display
	m_menuSystem.SetupDisplay();

	// Setup background tiles
	m_bgTiles->WriteToGBA();

	// Setup background map in BG0
	unsigned short* mapBaseBlock = (unsigned short*)0x600E000;
	for(int i = 0; i < 1024; ++i) {
		mapBaseBlock[i] = ictiles_Map[i];
	}

	//Turn on Background 0 settings 
	//0 = 256x256, since our background is 240x160, this should be fine
	BFSET(GBA_REG_BG0CNT, backgroundSize, 0x00);
	//Highest priority
	BFSET(GBA_REG_BG0CNT, priority, 0x1);
	BFSET(GBA_REG_BG0CNT, baseTileBlock, BKGND_BLOCK);
	BFSET(GBA_REG_BG0CNT, baseMapBlock, (unsigned char)28);
	//No mosaic
	BFSET(GBA_REG_BG0CNT, mosaic, 0x0);
	//256 color palette
	BFSET(GBA_REG_BG0CNT, palette, 0x1);

	//Turn on Mode 0, background 1, and sprite drawing
	BFSET(GBA_REG_DISPCNT, videoMode, 0);

	//Turn on BG0 and BG2 so we can see the background and menu
	BFSET(GBA_REG_DISPCNT, drawBG0, 1);
	BFSET(GBA_REG_DISPCNT, drawBG2, 1);

	//Turn off the rest of the backgrounds for the moment
	BFSET(GBA_REG_DISPCNT, drawBG1, 0);
	BFSET(GBA_REG_DISPCNT, drawBG3, 0);

	// Show statusbar
	m_statusBar.SetupDisplay();
	  
	// Show current icons 
	UpdateHeartbeat();
	ShowCurrentCodeState();
	ShowCurrentSerialState();
}

////////////////////////////////////////////////////////////////////
// CICIcon
unsigned short 
CICIcon::GetICMapTile(int row, int col) 
{
  return(ictiles_Map[32*row + col]);
}
void 
CICIcon::SetICBgTile(int row, int col, unsigned short val) 
{
  unsigned short* mapBaseBlock = (unsigned short*)0x600E000;
  mapBaseBlock[32*row + col] = val;
}

void 
CICIcon::ReadFromMap(int srcRow, int srcCol)
{
  int r, c;
  for(r=0; r<2; r++) {
    for(c=0; c<2; c++) {
      tiles[r][c] = GetICMapTile(srcRow+r, 
				 srcCol+c);
    }
  }
}

void 
CICIcon::WriteToBG(int destRow, int destCol)
{
  int r, c;
  if(!ICRobot::UsingDisplay()) return;

  for(r=0; r<2; r++) {
    for(c=0; c<2; c++) {
      SetICBgTile(destRow+r, destCol+c, 
		  tiles[r][c]);
    }
  }
}

////////////////////////////////////////////////////////////////////
// Icons and Heartbeat
#define ICHB_PHASE_LEN 15
// Minimum phase time in microseconds.  Don't beat faster than 10 Hz
#define ICHB_MIN_PHASE_TIME 100000

// Source positions
#define ICONS_SRC_ROW 		20
#define ICHB_SRC_COL 		0 
#define DOWNLOAD_SRC_COL 	4 
#define RUN_SRC_COL 		6 
#define STOP_SRC_COL 		8 
#define CONN_SRC_COL 		10 
#define DISCONN_SRC_COL 	12
#define WAIT_SRC_COL	 	14
#define LOGO_SRC_COL	 	18

#define BLANK_SRC_ROW 		18
#define BLANK_SRC_COL 		0

// Destination positions
#define ICONS_DEST_ROW 		0
#define ICHB_DEST_COL 		28 
#define CODESTATE_DEST_COL 	25 
#define SERIAL_DEST_COL 	19 

void ICRobot::InitializeIcons()
{
  // Blank
  m_blankIcon.ReadFromMap(BLANK_SRC_ROW, BLANK_SRC_COL);
  m_waitIcon.ReadFromMap(ICONS_SRC_ROW, WAIT_SRC_COL);
  m_logoIcon.ReadFromMap(ICONS_SRC_ROW, LOGO_SRC_COL);
  
  // Heartbeat
  for(int phase=0; phase<2; phase++) {
    m_heartIcons[phase].ReadFromMap(ICONS_SRC_ROW, 
				    ICHB_SRC_COL+2*phase);
  }
  // Code state icons
  m_downloadIcon.ReadFromMap(ICONS_SRC_ROW, DOWNLOAD_SRC_COL);
  m_runIcon.ReadFromMap(ICONS_SRC_ROW, RUN_SRC_COL);
  m_stopIcon.ReadFromMap(ICONS_SRC_ROW, STOP_SRC_COL);

  // Serial icons
  m_connIcon.ReadFromMap(ICONS_SRC_ROW, CONN_SRC_COL);
  m_disconnIcon.ReadFromMap(ICONS_SRC_ROW, DISCONN_SRC_COL);
}

void ICRobot::ShowHeartbeatPhase(int phase)
{
  if(phase<0 || phase>2) return;

  m_heartIcons[phase].WriteToBG(ICONS_DEST_ROW, ICHB_DEST_COL);
  m_heartPhase=phase;
}

void ICRobot::UpdateHeartbeat()
{
	// Get current time
	long unsigned currentTime;
	m_timer.GetCount(&currentTime);

	// Increment the count for the heart phase
	m_heartCount++;
	
	// Check if we should update to the next phase
	if(m_heartCount>=ICHB_PHASE_LEN && 
	   currentTime - m_heartChangeTime > ICHB_MIN_PHASE_TIME) {
		ShowHeartbeatPhase(!m_heartPhase); 
		m_heartChangeTime = currentTime;
	}
}

void ICRobot::ShowCodeDownloaded()
{
  m_downloadIcon.WriteToBG(ICONS_DEST_ROW, CODESTATE_DEST_COL);
  m_codeState = IC_CODE_DOWNLOADED;
  UpdateProgramMenuName();
}
void ICRobot::ShowCodeRunning()
{
  m_runIcon.WriteToBG(ICONS_DEST_ROW, CODESTATE_DEST_COL);
  m_codeState = IC_CODE_RUNNING;
  UpdateProgramMenuName();
}
void ICRobot::ShowCodeStopped()
{
  m_stopIcon.WriteToBG(ICONS_DEST_ROW, CODESTATE_DEST_COL);
  m_codeState = IC_CODE_STOPPED;
  UpdateProgramMenuName();
}

void ICRobot::ShowCodeState(EICCodeState state)
{
  switch(state) {
  case IC_CODE_MISSING: 
	  m_blankIcon.WriteToBG(ICONS_DEST_ROW, CODESTATE_DEST_COL);	  
	  break;
  case IC_CODE_DOWNLOADED: ShowCodeDownloaded(); break;
  case IC_CODE_RUNNING: ShowCodeRunning(); break;
  case IC_CODE_STOPPED: ShowCodeStopped(); break;
  }
}

void ICRobot::ShowCurrentCodeState()
{
  ShowCodeState(m_codeState);
}

void ICRobot::ShowSerialState(bool connected, bool showIcon)
{
  if(!showIcon) {
    m_blankIcon.WriteToBG(ICONS_DEST_ROW, SERIAL_DEST_COL);
  }
  else if(connected) {
    m_connIcon.WriteToBG(ICONS_DEST_ROW, SERIAL_DEST_COL);
  }
  else {
    m_disconnIcon.WriteToBG(ICONS_DEST_ROW, SERIAL_DEST_COL);
  }
}

void ICRobot::ShowCurrentSerialState()
{
	//ShowSerialState(m_commDevice->IsConnected(), true);
}

void ICRobot::SetIcOwnsBtns(bool val) 
{
  m_icOwnsBtns = val;
  if(val) {
    m_logoIcon.WriteToBG(0, 22);
  }
  else {
    m_blankIcon.WriteToBG(0, 22);
  }
}

///////////////////////////////////////////////////////////////////
void ICRobot::CheckForConnection()
{
	//Check for communications if we don't have them and 
	//if we arn't in competition mode

	if(!m_commDevice->IsConnected())
	{
		PrintToBuffer("Trying to connect\n");
		//ShowSerialState(false, false);
		m_commDevice->Connect();

		if(!m_commDevice->IsConnected())
		{
                        ShowSerialState(false, true);
			PrintToBuffer("Cannot find connection\n");
			return;
		}

		PrintToBuffer("Connected\n");	
	}
}

#ifndef USE_OWN_CLASS
void ICRobot::CheckIfDormant()
{
	for(int i = 0; i < 32; ++i)
	{
		if(!mem[0xC309+(16*i)])
		{
			if(m_bIsDormant)
			{
				m_bIsDormant = false;
				pcodesim_wakeup();
			}
			return;
		}
	}
	m_bIsDormant = true;
}

void ICRobot::RunICProcess()
{
	static long long unsigned startTime = 0, currentTime = 0;
	static double waitTime = 0;
	CheckIfDormant();
	//If we're running IC code at the moment, run through one process loop.
	if(!m_bIsDormant)
	{
		if(m_bWasDormant)
		{
		    pcodesim_wakeup();
		        waitTime = 0;
			ShowCodeState(IC_CODE_RUNNING); 
			DBG(g_printBuffer->PrintfIC("---START OF EXECUTION---\n"));
			m_bWasDormant = false;
			//m_timer.GetCount(&currentTime);
		}
      		m_timer.GetCount(&startTime);
		//waitTime += ((double)(startTime - currentTime))/1000.0;
		m_timer.GetCount(&currentTime);
		while(currentTime - startTime < 30000)
		{
			pcodesim_execute_some();
			m_timer.GetCount(&currentTime);
		}
	}
	else if (!m_bWasDormant && m_bIsDormant)
	{
	    //g_printBuffer->PrintfIC("TotalWaitTime: %f\n", waitTime);
		DBG(g_printBuffer->PrintfIC("---END OF EXECUTION---\n"));
		ShowCodeState(IC_CODE_STOPPED); 
		m_bWasDormant = true;
	}
}
#endif /* #ifndef USE_OWN_CLASS */

void ICRobot::RunRobotLoop()
{
#ifndef USE_OWN_CLASS
    unsigned long long time_for_sound;
#endif

    UpdateHeartbeat(); 
	
	#ifndef USE_SIM
	if(!m_commDevice->IsConnected()) {
		//PrintToBuffer("CheckForConnection\n");
		if(UART_GET_BT_MODE())
		{
			g_printBuffer->PrintfIC("Lost bluetooth connection, switching back to serial\n");
			delete m_commDevice;
			UART_SET_RS232();
			m_commDevice = new CUartDevice();
			
		}
		CheckForConnection();
	}
	#endif
	
	if(m_commDevice->IsConnected() && m_UartModeIC) {
		//PrintToBuffer("TranslatePacket\n");
#ifndef USE_OWN_CLASS
		TranslatePacket();
#endif /* #ifndef USE_OWN_CLASS */
	}
	/*
	//Profiling code to check IC runtime vs. program runtime
	static long long unsigned inside = 0, outside = 0;
	long long unsigned currentTime;
	CSimpTimer timer;
	timer.GetCount(&currentTime);
	PrintToBuffer("o: %d", currentTime - outside);
	*/
#ifndef USE_OWN_CLASS
	RunICProcess();

	// Stop sound playing if necessary
	if(m_tone.PlayingNoInt())
	{
		m_timer.GetCount(&time_for_sound);
		m_tone.CheckStopNoInt(time_for_sound);
	}


#endif /* #ifndef USE_OWN_CLASS */
	/*
	m_timer.GetCount(&inside);
	m_timer.GetCount(&outside);
	PrintToBuffer("i: %d\n", inside - currentTime);
	*/
	//m_xview->DrawLoop();
}

#ifndef USE_OWN_CLASS
void ICRobot::TranslatePacket()
{
    if(m_commDevice->QueryReadCount() == 0) return;

	// Ok, we're simulating transfer, so therefore we've gotta take apart the packet, figure
	// out where to put the info in the global mem pointer, and put it there

	unsigned char ucPacketSize = 0x00, ucSequence = 0x00, ucType = 0x00;
	//unsigned int iWaitTime = 10;
	//We start off with total packet length
	int iBytesRead = m_commDevice->Read((char*)&ucPacketSize, 1);
	if(iBytesRead == 0) return;

	// Blink serial connected on while we process packet
        ShowSerialState(true, true);
			      
	m_commDevice->Read((char*)&ucSequence, 1);
	m_commDevice->Read((char*)&ucType, 1);
	if(ucType == 0x03)
	{
		//byte 0 - COMMAND_WRITE_BLOCK
		//byte 1 to PointerSize+1 - Address to start writing at
		//byte PointerSize+1 to PacketSize+PointerSize+1 - Packet Length
		//byte PacketSize+PointerSize+1 to End - Data to write
		unsigned char ucAddress[2], ucSize;
		m_commDevice->Read((char*)ucAddress, 2);
		m_commDevice->Read((char*)&ucSize, 1);
		int iAddress = (ucAddress[0] << 8) | (ucAddress[1]);
		//m_commDevice->Read((char*)mem + iAddress, ucSize);
		m_commDevice->Read((char*)(mem+iAddress), ucSize);
		SendReply(ucSequence);
	}
	else if(ucType == 0x04)
	{
		//byte 0 - COMMAND_READ_BLOCK
		//byte 1 to PointerSize+1 - Address to start reading at
		//byte PointerSize+1 to PacketSize+PointerSize+1 - Packet Length
		//byte PacketSize+PointerSize+1 to End - Data address to read from
		unsigned char ucReadAddress[2], ucReadSize;
		m_commDevice->Read((char*)ucReadAddress, 2);
		m_commDevice->Read((char*)&ucReadSize, 1);
		int iReadAddress = (ucReadAddress[0] << 8) | ucReadAddress[1];
		SendReply(ucSequence, (int)ucReadSize, mem + iReadAddress);
	}	
	else if(ucType == 0x05)
	{

		//byte 0 - COMMAND_CLEAR_AND_SET
		//byte 1 to PointerSize+1 - Address to clear and set
		//byte PointerSize+1 to PointerSize+2 - Byte mask to use for clearing (Already complemented)
		//byte PointerSize+2 to PointerSize+3 - Byte to OR the newly masked byte against
		unsigned char ucAddress[2], ucClearBit, ucAssignBit, ucOldByte;
		m_commDevice->Read((char*)ucAddress, 2);
		m_commDevice->Read((char*)&ucClearBit, 1);		
		m_commDevice->Read((char*)&ucAssignBit, 1);		

		unsigned int iAddress = (ucAddress[0] << 8) | ucAddress[1];
		bool saveToFlash = false;

		//Store the old byte, since we have to send it back
		ucOldByte = mem[iAddress];

		// TODO: Change to check ucAssignBit & CS_WRITE_TO_FLASH_BIT
		// to decide whether or not to save to flash
		if(iAddress == CODE_STATUS_ADDR)
		{
// 			printf("CLEAR_AND_SET status: clear 0x%x, assign 0x%x\n", 
// 			       (int)ucClearBit, (int)ucAssignBit);
			
			if(ucAssignBit & CS_WRITE_TO_FLASH_BIT) {
				saveToFlash = true;
				// Don't actually set that bit...
				ucAssignBit &= ~CS_WRITE_TO_FLASH_BIT; 
			}
			ShowCodeDownloaded();
		}

	
		//Clear it, or it, smack it, flip it, rub it down
		//(Oh nooooo...)
		
		mem[iAddress] &= ucClearBit;
		mem[iAddress] |= ucAssignBit;
		
		SendReply(ucSequence, 1, (char*)&ucOldByte);

		if(iAddress == CODE_STATUS_ADDR)
		{
			HappyBeep();
			if(saveToFlash) {
			    pcodesim_reset();
				CopyMemToFlash();
			}
			else {
				m_statusBar.SetStatusBarState(CStatusBar::INFO_STATE);
				m_statusBar.InfoPrintf("Downloaded %s\n", 
						       GetProgramName());
			}
				
			UpdateProgramMenuName();
		}
	}
	else if(ucType == 0x06)
	{
		unsigned char ucMessage[4];
		ucMessage[0] = 0x00;
		ucMessage[1] = 0x64;
		ucMessage[2] = 0x00;
		ucMessage[3] = 0x48;

		SendReply(ucSequence, 4, (char*)ucMessage);
	}	
	else
	{
		PrintToBuffer("Unknown Type: %x\n", ucType);
	}
			    
	// Turn serial connected icont display off now that we're done
	// processing packet
        ShowSerialState(true, false);
}
#endif /* #ifndef USE_OWN_CLASS */
void ICRobot::SendReply(int iSequence, int iLength, char* chData)
{

	unsigned char ucMessage[4];
	ucMessage[0] = iLength + 2;
	ucMessage[1] = iSequence + 1;
	int iResult = m_commDevice->Write((const char*)ucMessage, 2);
	if(iLength > 0)
	{
		iResult += m_commDevice->Write((const char*)chData, iLength);
	}
}

bool ICRobot::HandleMenuEvent(int eventType, CMenuElement &menu)
{
	if(eventType > 99) return false;
	switch(eventType)
	{
		//Print buffer options
	case 0:
		ICRobotPBufContext::Push(*g_printBuffer, *this);
		break;
	case 1:
		g_printBuffer->Clear();
		m_statusBar.InfoPrintf("IC Console Cleared");
		
		break;
	case 2:
#ifndef NO_MOTOR
		// Clear Motor Positions
		for(int i=0; i<NUM_MOTORS; i++) {
			IcSetPosition(i, 0);
		}
#endif
		m_statusBar.InfoPrintf("Motor Positions Cleared");
		break;
		//Case for Process Table display
	case 3:
	    PrintProcessDisplay();
	    break;
#ifndef USE_OWN_CLASS
	//IC memory options
	case 10:
		CopyMemFromFlash();
		break;
	case 11:
		CopyMemToFlash();
		break;
	case 12:
		ClearMemFromFlash();
		break;
#endif /* #ifndef USE_OWN_CLASS */
	case 20:
		ToggleICProgramState();
		break;
	//Bluetooth function:
	case 21:
		ICRobotPBufContext::Push(*g_printBuffer, *this);
		AttemptBluetoothConnect();
		break;
#ifdef USE_OWN_CLASS
	//Runs non-pcode class
	case 75:
		HandleMenuEvent(0, menu);
		static NonPcode * m_nonpcode = new NonPcode();
		m_nonpcode->start(this);
		break;
	//Takes non-pcode into memory.
	case 76: 
		HandleMenuEvent(0, menu);
		m_nonpcode->newNPC();
		m_nonpcode->startOtherNPC();
		break;
#endif

	//ICXView additions:
//	case 40:
//		m_xview->SetViewOn();
//		break;
	}
	
	#ifndef USE_OWN_CLASS
		
		if(eventType>=30 && eventType<60)
		{
			// Load IC program at slot # (eventType-30)
			
			// Set the Flash slot
			SetFlashSlot(eventType-30);
			
			// Load program
			CopyMemFromFlash();
			
			// Run program
			//ToggleICProgramState();
		}
		
	#endif
	
	return true;
}

#ifndef USE_OWN_CLASS
unsigned int 
ICRobot::GetFlashMagic()
{
  unsigned int *fmIntAddr = (unsigned int *)(IC_FLASH_ADDR);
  return(fmIntAddr[IC_MAGIC_ADDR]);
}
unsigned char 
ICRobot::GetFlashCodeStatus()
{
  unsigned char *fmCharAddr = (unsigned char *)(IC_FLASH_ADDR);
  return(fmCharAddr[CODE_STATUS_ADDR]);
}
void
ICRobot::SetFlashCodeStatus(unsigned char status)
{
  unsigned char *fmCharAddr = (unsigned char *)(IC_FLASH_ADDR);
  fmCharAddr[CODE_STATUS_ADDR]=status;
}

bool
ICRobot::CheckFlashMemState() 
{
	unsigned int flashMagic = GetFlashMagic();
	bool magicOK = (flashMagic == IC_MAGIC);

	unsigned char codeStatus = GetFlashCodeStatus();
	bool statusOK = codeStatus & CS_LIVE_BIT;

// 	printf("Checking Magic\n  Expect 0x%x\n  Found  0x%x\n  Result %s\n",
// 	       IC_MAGIC, flashMagic, magicOK?"true":"false");

// 	printf("Checking Flash Status: Found  0x%x\n  Result %s\n",
// 	       (int)codeStatus, statusOK?"true":"false");

	if(!magicOK) {
		m_statusBar.SetStatusBarState(CStatusBar::INFO_STATE);
		m_statusBar.InfoPrintf("No program found in flash #%d\n", m_flashSlot);
	}
	else {
		if(statusOK) {
			m_statusBar.InfoPrintf("Valid program found in flash #%d\n", m_flashSlot);
		}
		else {
			m_statusBar.InfoPrintf("Program cleared from flash #%d\n", m_flashSlot);
		}
	}

	return(magicOK && statusOK);
}

void ICRobot::CopyMemToFlash()
{
	const char *tname = GetProgramName();
	
	int slot_count;
	
	bool slot_found = false;

	unsigned short old_slot = m_flashSlot;

	for(slot_count = 0; slot_count <= MAX_FLASH_SLOT; slot_count++)
	{
		SetFlashSlot(slot_count);
		if(!strcmp(GetProgramNameFlash(), GetProgramName()))
		{
			slot_found = true;
			break;
		}
	}
	
	if(! slot_found)
	{
		for(slot_count = 0; slot_count <= MAX_FLASH_SLOT; slot_count++)
		{
			SetFlashSlot(slot_count);
			if(!strcmp(GetProgramNameFlash(), "\0") || !CheckFlashMemState())
			{
				slot_found = true;
				break;
			}
		}
	}

	if(! slot_found)
	{
		m_statusBar.InfoPrintf("NO FLASH AVAILABLE.\nDELETE A PROGRAM\n");
		SetFlashSlot(old_slot);
		SadBeep();
		return;
	}

	m_statusBar.SetStatusBarState(CStatusBar::INFO_STATE);
	m_statusBar.InfoPrintf("Saving %s to flash #%d...\n", tname, m_flashSlot);
	m_flashMem.Erase(IC_FLASH_ADDR, IC_FLASH_SIZE);
	CopyPackStructsToICMem();
	m_flashMem.Program(IC_FLASH_ADDR,(unsigned short*) mem, IC_FLASH_SIZE);
	m_statusBar.InfoPrintf("Successfully saved %s at #%d\n", tname, m_flashSlot);
	
	if(!m_icProgramFlashItem[m_flashSlot])
	{
		if(CheckFlashMemState() && *(GetProgramNameFlash()))
		{
			const char *the_name = GetProgramNameFlash();
			
			char buffer[32];
			
			strncpy(buffer, the_name, 27);
			
			buffer[27]=0;
			
			m_icProgramFlashItem[slot_count] = progElem->AddMenuItem(buffer, 30+m_flashSlot);
			//m_icProgramFlashItem[slot_count]->SetName(buffer);
			
			SetupDisplay();
		}
	}
	else
	{
		if(CheckFlashMemState() && *(GetProgramNameFlash()))
		{
			const char *the_name = GetProgramNameFlash();
			
			char buffer[32];
			
			strncpy(buffer, the_name, 27);
			
			buffer[27]=0;
			
			//m_icProgramFlashItem[slot_count] = progElem->AddMenuItem(buffer, 30+m_flashSlot);
			m_icProgramFlashItem[slot_count]->SetName(buffer);
			
			SetupDisplay();
		}
	}
	
	
}

void ICRobot::CopyMemFromFlash()
{
	if(!CheckFlashMemState()) {
		return;
	}

	m_statusBar.SetStatusBarState(CStatusBar::INFO_STATE);
	m_statusBar.InfoPrintf("Loading from flash #%d...\n", m_flashSlot);
	memcpy(mem,(unsigned short*)IC_FLASH_ADDR, 65532);
	CopyPackStructsToICMem();
	m_statusBar.InfoPrintf("Successfully loaded %s at #%d\n", 
			       GetProgramName(), m_flashSlot);
	
	if(IsLibLoaded()) {
		ShowCodeDownloaded();
	}
	UpdateProgramMenuName();
}

void ICRobot::ClearMemFromFlash()
{
	m_statusBar.SetStatusBarState(CStatusBar::INFO_STATE);
	m_statusBar.InfoPrint("Clearing flash...\n");

	m_flashMem.Erase(IC_FLASH_ADDR, IC_FLASH_SIZE);
	SetFlashCodeStatus(0);
	m_statusBar.InfoPrint("Cleared flash\n");
	
	if(m_icProgramFlashItem[m_flashSlot])
	{
		m_icProgramFlashItem[m_flashSlot]->SetName("[ERASED]");
		SetupDisplay();
	}
}

void ICRobot::SetFlashSlot(unsigned short slot_num)
{
	if(slot_num <= MAX_FLASH_SLOT)
	{
		m_flashSlot = slot_num;
	}
}

#endif /* #ifndef USE_OWN_CLASS */

#ifndef NO_MOTOR
unsigned short ICRobot::GetScaledAnalog(unsigned short sensorPort)
{
	int result = GetAnalog(sensorPort)/12;
	if(result<256)return result;
	else return 255;
}
#endif

#define CHK_MNUM(x) if(x<0 || x>= NUM_MOTORS) return(-1);

short ICRobot::CallML1Translator(const short& functionIndex, const short& argument1, void * pointer)
{

	switch(functionIndex)
	{
#ifndef NO_GPIO
case 100:
		//Digital Sensor status
		if(GetPortDirection(argument1)) SetPortDirection(argument1, 0);
		return GetDigital(argument1);
#endif
#ifndef NO_MOTOR
	case 101:
		//Analog Sensor status
		return GetAnalog(argument1);
	case 102:
		{
#ifndef USE_OWN_CLASS
			float32 power = GetBatteryVoltage();
			SET_MEM4(((unsigned short)argument1), *(long int *)(&power));
#else
			float32 power = GetBatteryVoltage();
			*(float32 *)pointer=power; //not really the voltage. Multiply by .00214 to get actual voltage
#endif /* #ifndef USE_OWN_CLASS */
			break;
		}
	case 103:
		return GetScaledAnalog(argument1);
#endif
	case 113: {
		// Button State
	        static bool firstBtnState=true;
		m_buttonState.PollKeys();	

		// The first time anyone calls this, make sure we go
		// into the right state for the IC console.  After
		// that, only return real button state if IcOwnsBtns
		// is true.
		if(firstBtnState) {
		  g_printBuffer->ShowICPrintBuffer();
		  ICRobotPBufContext::Push(*g_printBuffer, *this);
		  firstBtnState=false;
		}

		if(m_icOwnsBtns) {
		  return m_buttonState.CurrState();
		}
		else {
		  // Simulate no buttons being pressed
		  return 0x3ff;
		}
	}
	case 120: {
	  // Clear IC display
	  g_printBuffer->Clear(CPrintBuffer::BUF_IC);
	  
	  // Fixes problem with default cursor position when double-buffering.  -Jeremy Rand
	  g_printBuffer->SetPosition(CPrintBuffer::BUF_IC, 0, 0);
	  
	  DBG(PrintToBuffer("Cleared IC display\n"));
	  return(0);
	}
	  ////////////////////////////////////////////////////////
	  // TONE
	case 140:
		// Set Beeper enable
		DBG(PrintToBuffer("Tone enable= %d\n", (int)argument1));
		m_tone.SetEnable(argument1);
		return 0;
	case 141:
		// Set Beeper frequency
		DBG(PrintToBuffer("Tone freq= %d\n", (int)argument1));
		m_tone.SetFreq(argument1);
		return 0;
#ifndef NO_MOTOR
		////////////////////////////////////////////////////////
		// MOTOR
	case 160:
		//Forward/Backward/Alloff
		//  argument1 = pwm value
		{
			CallML2Translator(201, 0, argument1);
			CallML2Translator(201, 1, argument1);
			CallML2Translator(201, 2, argument1);
			CallML2Translator(201, 3, argument1);
		}
		break;
	case 161:
		// Get acceleration currently used by motion commands
		// int get_motion_acceleration(int motor)
		//  argument1 = motor number
		CHK_MNUM(argument1);
		return(m_motionAccel[argument1]);
		break;
	case 162:
		CHK_MNUM(argument1);
		return Done(argument1) ? 1 : 0;
#endif
#ifndef NO_GPIO
	case 170:
		//Set Encoder
		return SetEncoderPort(argument1);
	case 171:
		//Unset Encoder
		UnsetEncoderPort(argument1);
		break;
	case 172:
		//Reset Encoder
		ResetEncoderCount(argument1);
		break;
	case 173:
		//Get Encoder
		return GetEncoderCount(argument1);
	case 174:
		return SetSonarPort(argument1);
	case 175:
		SendSonarRing(argument1);
		break;
	case 176:
		return GetSonarTime(argument1);
	case 177:
		return GetSonarBlocking(argument1);
#endif
#ifndef NO_SERIAL
	case 178:
		//set_serial_mode
		return SetSerialMode(argument1);
	case 179:
		//serial_buffer_count
		return m_commDevice->QueryReadCount();
	case 180:
		//serial_read_byte
		if(m_commDevice->QueryReadCount()) {
			char data;
			m_commDevice->Read((char *)&data,1);
			return data;
		}
		return -1;
	case 181:
		//serial_write_byte
		m_commDevice->Write((char *)&argument1,1);
		break;
	case 182:
                if(argument1)
			{UART_SET_EXTRA();}
                else
			{UART_SET_USB();}
		break;
#endif
#ifndef USE_OWN_CLASS
	case 190:
		//Hog Processor
		//PCODE_PROCESS_TICKS is stored @ 10
		SET_MEM1(10, 250);
		break;
	case 191:
		SET_MEM4(((unsigned short)argument1), GenerateRandom());
		break;
#endif
	case 192:
		return GenerateRandomInRange(argument1);
#ifndef USE_OWN_CLASS
	case 193:
		SeedRandom(MEM4(argument1));
		break;
#endif /* #ifndef USE_OWN_CLASS */
	case 194:
	    {
		if(argument1) m_ledStatus |= XP_LED_RED;
		else m_ledStatus &= ~XP_LED_RED;
		XP_REG_LED = m_ledStatus;
		break;
	    }
	case 195:
	    {
		if(argument1) m_ledStatus |= XP_LED_GREEN;
		else m_ledStatus &= ~XP_LED_GREEN;
		XP_REG_LED = m_ledStatus;
		break;
	    }
	case 196:
	    {
		PrintProcessDisplay();
		break;
	    }
	default:
		return 0;
	}
	return 0;
}

short ICRobot::CallML2Translator(const short& functionIndex, const short& argument1, const short& argument2, void * pointer, void * pointer2)
{
	switch(functionIndex)
	{
#ifndef NO_MOTOR
	case 201:
		//Set PWM for motors
		// void _setpwm(int motor, int pwm)
		//  argument1 = motor number
		//  argument2 = pwm value
		CHK_MNUM(argument1);
		IcMovePWM(argument1, argument2);
		break;
	case 202:
		// long get_motor_position_counter(int motor)
		//  argument1 = motor number
		//  argument2 = address of where to put position value
		CHK_MNUM(argument1);
#ifndef USE_OWN_CLASS
		SET_MEM4((unsigned short)argument2, 
			IcGetPosition(argument1));
#else
		AxisPosition tmp;
		tmp = IcGetPosition(argument1);
		pointer = &tmp;
#endif /* #ifndef USE_OWN_CLASS */

		break;
	case 203:
		// void set_motor_position_counter(int motor, long value)
		//  argument1 = motor number
		//  argument2 = address of new position value
		CHK_MNUM(argument1);
#ifndef USE_OWN_CLASS
		IcSetPosition(argument1, 
			      (signed int)MEM4((unsigned short)argument2));
#else
		IcSetPosition(argument1,*(signed int *)pointer);
#endif /* #ifndef USE_OWN_CLASS */
		break;
// 204 was move_at_velocity but arg2 changed, don't do anything
	case 204: return -1;
// 205 was void set_motion_acceleration(int motor, long acceleration), 
//  arg type changed
	case 205: return -1;
// 206 was long get_motion_acceleration(int motor), but args changed
	case 206: return -1;
// 207 was freeze but arg2 changed, don't do anything
	case 207: return -1;
	case 208: {
		// Move at constant velocity at motion acceleration
		// stored to use for that motor.  This is done by
		// sending it to the maximum position in the direction
		// you want to go based on the sign of the velocity
		// argument.  Motion acceleration defaults to
		// DEFAULT_MOTION_ACCEL and can be set from IC with
		// set_motion_acceleration (callml(206))

		// void move_at_velocity(int motor, long velocity)
		//  argument1 = motor number
		//  argument2 = desired velocity
		CHK_MNUM(argument1);
		IcMoveVelocity(argument1, argument2, 
			       m_motionAccel[argument1]);
		break;
	}
	case 209:
		// Actively hold motor at current position by doing a
		// closed loop move at velocity 0 with a destination
		// it can never get to

		// void freeze(int motor)
		//  argument1 = motor number
		//  argument2 = desired acceleration 
		//              (higher value = more abrupt stop)
		CHK_MNUM(argument1);
		IcMoveFreeze(argument1, 
			     argument2);
 		break;
	case 210:
		// Set acceleration to be used by motion commands
		// void set_motion_acceleration(int motor, long acceleration)
		//  argument1 = motor number
		//  argument2 = new acceleration value
		CHK_MNUM(argument1);
		m_motionAccel[argument1] = argument2;
		break;

	case 211:
	    CHK_MNUM(argument1);
#ifndef USE_OWN_CLASS
	    SET_MEM2((unsigned short)argument2, GetCurrent(argument1));
#endif /* #ifndef USE_OWN_CLASS */
	    break;
	case 212:
	    CHK_MNUM(argument1);
	    SetCalibrate(argument1, argument2);
	    break;
		
#endif
	case 221: {
	  // Set display X,Y
	  CTextBuf *icBuf = g_printBuffer->GetICTextBuf();
	  if(argument1>=0 && argument1 < icBuf->lineNum &&
	     argument2>=0 && argument2 < icBuf->lineWidth) {
	    g_printBuffer->SetPosition(CPrintBuffer::BUF_IC, 
				       argument1, argument2);
	    DBG(PrintToBuffer("Set disp xy to (%d, %d)\n", 
			      icBuf->xPos, icBuf->yPos));
	    return(0);
	  }
	  else {
	    return(-1);
	  }
	}
	case 222: {
	  // Get display X,Y.  argument1 is &x, argument2 is &y
	  unsigned char x, y;
	  g_printBuffer->GetPosition(CPrintBuffer::BUF_IC, 
				     &x, &y);
#ifndef USE_OWN_CLASS
	  SET_MEM2(((unsigned short)argument1), (short)x);
	  SET_MEM2(((unsigned short)argument2), (short)y);
	  return(0);
#else
	  *(unsigned char*)pointer  = x;
	  *(unsigned char*)pointer2 = y;
#endif /* #ifndef USE_OWN_CLASS */
	}
	case 223: {
          // Set display X,Y
          //CTextBuf *icBuf = g_printBuffer->GetICTextBuf();
            g_printBuffer->SetBufferPosition(CPrintBuffer::BUF_IC,
                                       argument1, argument2);
            DBG(PrintToBuffer("Set disp xy to (%d, %d)\n",
                              icBuf->xPos, icBuf->yPos));
            return(0);
          }
          
        case 224: {
          // Get display X,Y.  argument1 is &x, argument2 is &y
          unsigned char x, y;
          g_printBuffer->GetBufferPosition(CPrintBuffer::BUF_IC,
                                     &x, &y);
#ifndef USE_OWN_CLASS
	  SET_MEM2(((unsigned short)argument1), (short)x);
	  SET_MEM2(((unsigned short)argument2), (short)y);
	  return(0);
#else
	  *(unsigned char*)pointer = x;
	  *(unsigned char*)pointer2 = y;
#endif /* #ifndef USE_OWN_CLASS */
        }

#ifndef NO_GPIO	     
	case 230: {
		//Set Port Direction for Digital Sensor In/Out
		SetPortDirection(argument1, 1);
		SetPortValue(argument1, argument2);
		return(0);
	}
#endif
		

	case 290:
#ifndef USE_OWN_CLASS
	  SET_MEM4(((unsigned short)argument2), GenerateRandomInRange((int)MEM4(argument1)));
#endif /* #ifndef USE_OWN_CLASS */
	  break;
	}
	return 0;
}

short ICRobot::CallML3Translator(const short& functionIndex, const short& argument1, const short& argument2, const short& argument3, void * pointer)
{
#ifndef NO_MOTOR
	switch(functionIndex)
	{
	case 301:
	    //Set PID Gains
	    SetPIDGains(argument1, argument2, argument3);
	    break;
	    // 302 was move_to_position, but type of argument 2 changed
	case 302: return -1;
	    // 303 was move_relative_position, but type of argument 2 changed
	case 303: return -1;
	case 304:
	    // void move_to_position(int motor, int velocity, long goal_pos)
	    //  argument1 = motor number
	    //  argument2 = desired velocity
	    //  argument3 = address of goal position
	    CHK_MNUM(argument1);

#ifndef USE_OWN_CLASS
	    IcMove(argument1, 
		   (signed int)MEM4((unsigned short)argument3),
		   argument2,
		   m_motionAccel[argument1]);
#else
		IcMove(argument1,
			*(signed int *)(pointer),
			argument2,
			m_motionAccel[argument1]);
#endif /* #ifndef USE_OWN_CLASS */
	    break;
	case 305: {
	    // void move_relative_position(int motor, int velocity, long goal_pos)
	    //  argument1 = motor number
	    //  argument2 = desired velocity
	    //  argument3 = address of delta position
	    CHK_MNUM(argument1);

	    if(Done(argument1)) SyncTrajectory(argument1);
	    AxisPosition spos = IcGetPosition(argument1);
#ifndef USE_OWN_CLASS
	    AxisPosition goal = (signed int)MEM4((unsigned short)argument3) + spos;
#else
		AxisPosition goal = (*((long *)pointer)) + spos;
#endif /* #ifndef USE_OWN_CLASS */
	    IcMove(argument1, goal, 
		   argument2, 
		   m_motionAccel[argument1]);
	    break;
	}
	case 323: {
		// void display_clear_at(int x, int y, int width)
		CTextBuf *icBuf = g_printBuffer->GetICTextBuf();
		int x = argument1;
		int y = argument2;
		int width = argument3;

		DBG(g_printBuffer->PrintfIC("Clearing disp at (%d, %d) for %d chars\n", 
				x, y, width));

		if(y>=0 && y < icBuf->lineNum &&
		     x>=0 && x < icBuf->lineWidth) {
			g_printBuffer->SetPosition(CPrintBuffer::BUF_IC, 
						x, y);
			// If needed, truncate width to stay on the same line
			if(width+x > icBuf->lineWidth) {
			    width = icBuf->lineWidth - x;
			}
			// Clear the section of the screen specified
			for(int i=0; i<width; i++) {
				g_printBuffer->PrintfIC(" ");
			}
			// Set the position in the buffer to the start of the 
			// cleared area
			g_printBuffer->SetPosition(CPrintBuffer::BUF_IC, 
					   x, y);
			DBG(g_printBuffer->PrintfIC("  DONE, actual width=%d\n", 
					    width));
			return(0);
	    }
	    else
		{
			DBG(g_printBuffer->PrintfIC("  FAILED, lineNum=%d, lineWidth=%d\n", 
					    icBuf->lineNum,
					    icBuf->lineWidth));
			return(-1);
	    }
	    break;
	}
	}
#endif
	return 0;

}

CMenuDisp &ICRobot::GetMenuSystem() 
{
	return(m_menuSystem);
}

void ICRobot::UpdateStatusBar()
{
	m_statusBar.UpdateStatusBar(*this);
}

void ICRobot::ChangeStatusBarPage(int dir)
{
	if(dir>0) {
		m_statusBar.IncreaseStatusBarState();
	}
	else if(dir<0) {
		m_statusBar.DecreaseStatusBarState();
	}
}

////////////////////////////////////////////////////////////////////
// IC Program state
bool 
ICRobot::IsLibLoaded() 
{
#ifndef USE_OWN_CLASS
	char codeStatus = mem[CODE_STATUS_ADDR];
	bool libLoaded =  codeStatus & CS_LIB_LOADED_BIT;
// 	printf("Checking main: Found  0x%x\n  Result %s\n",
// 	       (int)codeStatus, libLoaded?"true":"false");
	return(libLoaded);
#else
	return false;
#endif /* #ifndef USE_OWN_CLASS */
}
bool 
ICRobot::IsMainLoaded()
{
#ifndef USE_OWN_CLASS
	char codeStatus = mem[CODE_STATUS_ADDR];
	bool mainLoaded = codeStatus & CS_MAIN_LOADED_BIT;
// 	printf("Checking main: Found  0x%x\n  Result %s\n",
// 	       (int)codeStatus, mainLoaded?"true":"false");
	return(mainLoaded);
#else
	return false;
#endif /* #ifndef USE_OWN_CLASS */
}

bool 
ICRobot::IsMainLoadedFlash()
{
#ifndef USE_OWN_CLASS
	char codeStatus = ((char *)IC_FLASH_ADDR)[CODE_STATUS_ADDR];
	bool mainLoaded = codeStatus & CS_MAIN_LOADED_BIT;
// 	printf("Checking main: Found  0x%x\n  Result %s\n",
// 	       (int)codeStatus, mainLoaded?"true":"false");
	return(mainLoaded);
#else
	return false;
#endif /* #ifndef USE_OWN_CLASS */
}

const char *
ICRobot::GetProgramName()
{
#ifndef USE_OWN_CLASS
	if(IsMainLoaded()) {
		// The print buffer is at 0xC110, see PRINT_BUFFER in
		// pcode.h and when main is loaded IC writes the
		// program name to the print buffer
		
		if(strlen(mem+0xC110) > 0x50)
		{
			return("\0");
		}
		
		return(mem+0xC110);
	}
	else if(IsLibLoaded()) {
		return("library");
	}
	else {
		return("memory");
	}
#else
	return("Non Pcode in use!");
#endif /* #ifndef USE_OWN_CLASS */
}

const char *
ICRobot::GetProgramNameFlash()
{
#ifndef USE_OWN_CLASS
	if(IsMainLoadedFlash()) {
		// The print buffer is at 0xC110, see PRINT_BUFFER in
		// pcode.h and when main is loaded IC writes the
		// program name to the print buffer
		
		if(strlen((char *)(IC_FLASH_ADDR+0xC110)) > 0x50)
		{
			return("\0");
		}
		
		return((char *)(IC_FLASH_ADDR+0xC110));
	}
	else
	{
		// We don't care about libs, we just want to know if there's a full program to load or not.
		return("\0");
	}
	/*else if(IsLibLoadedFlash()) {
		return("library");
	}
	else {
		return("memory");
	}*/
#else
	return("Non Pcode in use!");
#endif /* #ifndef USE_OWN_CLASS */
}

void ICRobot::StartICProgram()
{
	PrintToBuffer("Reset called\n");
	g_printBuffer->ShowICPrintBuffer();
	ICRobotPBufContext::Push(*g_printBuffer, *this);
#ifndef USE_OWN_CLASS
	pcodesim_reset();
	(unsigned char)mem[0x0200] = 0x0;
	//(unsigned short)*(mem+PROCESS_TABLE+P_PC) = 0x0080;
	//(unsigned char)*(mem+PROCESS_TABLE+P_STATUS) = PSTAT_RUNNING;
	(unsigned short)*(mem+0xc300+0x0) = 0x0002; //remember the little endianness
	(unsigned char)*(mem+0xc300+0x9) = 0x0;
#else
	//TODO: Start nonpcode program here?
#endif /* #ifndef USE_OWN_CLASS */
	ShowCodeState(IC_CODE_RUNNING); 
}

void ICRobot::StopICProgram()
{
#ifndef USE_OWN_CLASS
	// For each entry in the process table, set the status to 0x40
	// to stop it
	for(int i = 0; i < 32; ++i)
	{
		mem[0xC309+(16*i)] = 0x40;
	}
	m_bIsDormant = true;
#else
	//TODO: Stop nonpcode program here?
#endif /* #ifndef USE_OWN_CLASS */
	ShowCodeState(IC_CODE_STOPPED); 
}

void ICRobot::ToggleICProgramState(bool reqAtoStart)
{
#ifndef NO_MOTOR
	for(int i = 0; i < NUM_MOTORS; ++i)	
	{
		Stop(i);
		Execute();
		IcMoveStop(i);
	}
#endif

	if(m_codeState == ICRobot::IC_CODE_RUNNING) {
		// Currently running.  TODO: stop it
		m_statusBar.InfoPrint("Stopping IC Program\n");
		StopICProgram();
		SetSerialMode(0);
	}
	else if(!IsMainLoaded()) {
		// Nothing loaded, don't do anything
		m_statusBar.InfoPrint("No IC Program loaded\n");
		StopICProgram();
	}
	else {
		if(reqAtoStart && (GBA_REG_P1&GBA_KEY_A)) {
			// Requring A to be held down to start, 
			// but isn't pressed
			m_statusBar.InfoPrint("Use A+start to run IC Program\n");
		}
		else {
			// Go ahead and run it
			m_statusBar.InfoPrint("Running IC Program\n");
			StartICProgram();
		}
	}
}

////////////////////////
// Motor wrappers which scale everything by IC_BEMF_DIV on the
// way in and out
#ifndef NO_MOTOR
void 
ICRobot::IcMoveVelocity(unsigned char axis,
			int velocity, unsigned int acceleration)
{
	// sync trajectory to get CAxesClosed to sync up with world
	// outside itself
	if(Done(axis)) SyncTrajectory(axis);
	// setup the desired move command
	DBG(printf("Axis %d: MoveVelocity vel %d, accel %d\n",
		   axis, velocity, acceleration));

	// Setup the trajectory generator for a velocity move
	MoveVelocity(axis, velocity*IC_BEMF_DIV, 
		     acceleration*IC_BEMF_DIV);
	// start the control loop working on move
	Execute();
}

void 
ICRobot::IcMoveVelocityTicks(unsigned char axis,
			int velocity, unsigned int acceleration)
{
	// sync trajectory to get CAxesClosed to sync up with world
	// outside itself
	if(Done(axis)) SyncTrajectory(axis);
	// setup the desired move command
	DBG(printf("Axis %d: MoveVelocity vel %d, accel %d\n",
		   axis, velocity, acceleration));

	// Setup the trajectory generator for a velocity move
	CAxesClosed::MoveVelocity(axis, velocity*IC_BEMF_DIV, 
		     acceleration*IC_BEMF_DIV);
	// start the control loop working on move
	Execute();
}

void 
ICRobot::IcMove(unsigned char axis,
		AxisPosition endPosition, int velocity, 
		unsigned int acceleration)
{
	// Make sure velocity is positive
	if(velocity < 0) velocity *= -1;

	// sync trajectory to get CAxesClosed to sync up with world
	// outside itself
	if(Done(axis)) SyncTrajectory(axis);
	// setup the desired move command
	DBG(printf("Axis %d: Move to pos %d, vel %d, accel %d\n",
		   axis, endPosition, velocity, acceleration));

	// To debug Botball issue, bot moving wrong direction and distance while turning
	//g_printBuffer->PrintfIC("Axis %d: Move to pos %d, vel %d, accel %d\n",
	//	   axis, endPosition, velocity, acceleration);

	Move(axis, endPosition*IC_BEMF_DIV, velocity*IC_BEMF_DIV, 
	     acceleration*IC_BEMF_DIV);
	// start the control loop working on move
	Execute();
}

void 
ICRobot::IcMoveTicks(unsigned char axis,
		AxisPosition endPosition, int velocity, 
		unsigned int acceleration)
{
	// Make sure velocity is positive
	if(velocity < 0) velocity *= -1;

	// sync trajectory to get CAxesClosed to sync up with world
	// outside itself
	if(Done(axis)) SyncTrajectory(axis);
	// setup the desired move command
	DBG(printf("Axis %d: Move to pos %d, vel %d, accel %d\n",
		   axis, endPosition, velocity, acceleration));

	CAxesClosed::Move(axis, endPosition*IC_BEMF_DIV, velocity*IC_BEMF_DIV, 
	     acceleration*IC_BEMF_DIV);
	// start the control loop working on move
	Execute();
}

AxisPosition 
ICRobot::IcGetPosition(unsigned char axis)
{
	int opos = CDiffBase::GetPosition(axis)/IC_BEMF_DIV;
	DBG(int cpos = CAxesClosed::GetPosition(axis)/IC_BEMF_DIV);
	DBG(printf("Axis %d: opos=%d, cpos=%d, done=%s\n", 
		   axis, opos, cpos, Done(axis)?"true":"false"));
	return(opos);
}

AxisPosition 
ICRobot::IcGetPositionTicks(unsigned char axis)
{
	int opos = CAxesClosed::GetPosition(axis)/IC_BEMF_DIV;
	DBG(int cpos = CAxesClosed::GetPosition(axis)/IC_BEMF_DIV);
	DBG(printf("Axis %d: opos=%d, cpos=%d, done=%s\n", 
		   axis, opos, cpos, Done(axis)?"true":"false"));
	return(opos);
}

AxisPosition 
ICRobot::IcGetPositionRaw(unsigned char axis)
{
	int opos = CAxesOpen::GetPosition(axis)/IC_BEMF_DIV;
	DBG(int cpos = CAxesClosed::GetPosition(axis)/IC_BEMF_DIV);
	DBG(printf("Axis %d: opos=%d, cpos=%d, done=%s\n", 
		   axis, opos, cpos, Done(axis)?"true":"false"));
	return(opos);
}

void         
ICRobot::IcSetPosition(unsigned char axis, AxisPosition pos)
{
	// Cancel any trajectory currently in progress, including a
	// freeze command.  Otherwise changing the position will cause
	// the motor to move, possibly violently.
	if(!Done(axis)) {
		Stop(axis);
		Execute();
	}
	CDiffBase::SetPositionClosed(axis, 
		    pos*IC_BEMF_DIV);
	if(Done(axis)) SyncTrajectory(axis);
}

void         
ICRobot::IcSetPositionTicks(unsigned char axis, AxisPosition pos)
{
	// Cancel any trajectory currently in progress, including a
	// freeze command.  Otherwise changing the position will cause
	// the motor to move, possibly violently.
	if(!Done(axis)) {
		Stop(axis);
		Execute();
	}
	CAxesClosed::SetPositionClosed(axis, 
		    pos*IC_BEMF_DIV);
	if(Done(axis)) SyncTrajectory(axis);
}

void         
ICRobot::IcSetPositionRaw(unsigned char axis, AxisPosition pos)
{
	// Cancel any trajectory currently in progress, including a
	// freeze command.  Otherwise changing the position will cause
	// the motor to move, possibly violently.
	if(!Done(axis)) {
		Stop(axis);
		Execute();
	}
	SetPosition(axis, 
		    pos*IC_BEMF_DIV);
	if(Done(axis)) SyncTrajectory(axis);
}

void ICRobot::IcMovePWM(unsigned char axis, short pwm)
{
	if(pwm >= 100) pwm = 100;
	else if(pwm <= -100) pwm = -100;	
	pwm = (short)(pwm*2.55);
	//printf("\nAxis: %d, PWM: %d\n",axis,pwm);
	Stop(axis);
	Execute();
	while(!Done(axis));
	SetPWM(axis, pwm);
}

void ICRobot::IcMoveStop(unsigned char axis)
{
	IcMovePWM(axis, 0);
}

void ICRobot::IcMoveFreeze(unsigned char axis, unsigned int acceleration)
{
	IcMove(axis, 
	       0xF00000, 
	       0, 
	       acceleration);
	Execute();
}
#endif

void ICRobot::HappyBeep()
{
	CSimpTimer timer;
	long long unsigned startTime, currentTime;
	m_tone.SetEnable(true);
	m_tone.SetFreq(1000);
	timer.GetCount(&startTime);
	timer.GetCount(&currentTime);
	while(currentTime - startTime < 50000L) timer.GetCount(&currentTime); 	
	m_tone.SetFreq(1100);
	timer.GetCount(&startTime);
	timer.GetCount(&currentTime);
	while(currentTime - startTime < 50000L) timer.GetCount(&currentTime);
	m_tone.SetFreq(1200);
	timer.GetCount(&startTime);
	timer.GetCount(&currentTime);
	while(currentTime - startTime < 50000L) timer.GetCount(&currentTime);
	m_tone.SetFreq(1300);
	timer.GetCount(&startTime);
	timer.GetCount(&currentTime);
	while(currentTime - startTime < 50000L) timer.GetCount(&currentTime);
	m_tone.SetEnable(false);
}

void ICRobot::SadBeep()
{
	CSimpTimer timer;
	long long unsigned startTime, currentTime;
	m_tone.SetEnable(true);
	for (int i=0; i < 20; i++) {
		m_tone.SetFreq(300);
		timer.GetCount(&startTime);
		timer.GetCount(&currentTime);
		while(currentTime - startTime < 10000L) timer.GetCount(&currentTime); 	
		m_tone.SetFreq(200);
		timer.GetCount(&startTime);
		timer.GetCount(&currentTime);
		while(currentTime - startTime < 10000L) timer.GetCount(&currentTime); 	
	}
	for (int i=0; i < 20; i++) {
		m_tone.SetFreq(150);
		timer.GetCount(&startTime);
		timer.GetCount(&currentTime);
		while(currentTime - startTime < 10000L) timer.GetCount(&currentTime); 	
		m_tone.SetFreq(100);
		timer.GetCount(&startTime);
		timer.GetCount(&currentTime);
		while(currentTime - startTime < 10000L) timer.GetCount(&currentTime); 	
	}
	m_tone.SetEnable(false);
}

// From:
// http://remus.rutgers.edu/~rhoads/Code/mwc.c

/* Choose a value for a from this list
   1791398085 1929682203 1683268614 1965537969 1675393560
   1967773755 1517746329 1447497129 1655692410 1606218150
   2051013963 1075433238 1557985959 1781943330 1893513180
   1631296680 2131995753 2083801278 1873196400 1554115554
*/

// Returns 0 to 2^31-1
int ICRobot::GenerateRandom() {
	#ifndef USE_SIM
        const unsigned int a=1791398085;
        const unsigned int ah=a>>16;
        const unsigned int al=a&65535;
		
        unsigned int xh = m_rand_x>>16, xl = m_rand_x & 65535;
		
        unsigned long microseconds;
        m_timer.GetCount(&microseconds);
        
        m_rand_x = m_rand_x * a + m_rand_c + microseconds;
        m_rand_c = xh*ah + ((xh*al) >> 16) + ((xl*ah) >> 16);
        /* thanks to Don Mitchell for this correction */
        if (xl*al >= ~m_rand_c + 1) m_rand_c++;  
        return 0x7fffffff & m_rand_x;
	#else
		return(rand()); // added by Jeremy Rand, 2007 Sep 28
	#endif
}

int ICRobot::GenerateRandomInRange(int x) {
	return(GenerateRandom()%x);
}
void ICRobot::SeedRandom(int x) {
	#ifndef USE_SIM
		m_rand_x = x;
	#else
		srand(x); // added by Jeremy Rand, 2007 Sep 28
	#endif
}

void ICRobot::PrintProcessDisplay() {
#ifndef USE_OWN_CLASS
    for(int i = 0xc300; i < 0xc350; i+=0x10)
	{
	    g_printBuffer->PrintfIC("%.4x\n", (unsigned short)((*(mem+i)<<8)+*(mem+i+1)));
	}
#endif /* #ifndef USE_OWN_CLASS */
}

void ICRobot::AttemptBluetoothConnect() {
#ifndef USE_SIM
	if(m_commDevice && m_commDevice->IsConnected())
	delete m_commDevice;
	UART_SET_BT();
	m_commDevice = new CBluetoothDevice();
	g_printBuffer->PrintfIC("\nWaiting for a connection...");
	m_commDevice->Connect();
	if (!m_commDevice->IsConnected()) {
		g_printBuffer->PrintfIC("timed out\n");
		delete m_commDevice;
		UART_SET_RS232();
		m_commDevice = new CUartDevice();
	}
	else {
		g_printBuffer->PrintfIC("connected\n");
	}
#endif
}

int ICRobot::SetSerialMode(int baudrate)
{
	if(baudrate == 0) { //Reset the connection
		m_UartModeIC = true;
		while(m_commDevice->QueryReadCount()) { //Empty buffer
			char a;
			m_commDevice->Read(&a,1);
		}
		m_commDevice->Disconnect();
		m_commDevice->SetBaudRate(DUC_BAUD_38K);
		return 1;
	}
	m_commDevice->SetBaudRate(baudrate);
	m_UartModeIC = false;
	return 1;
}

int ICRobot::GetSerialMode()
{
		return m_commDevice->GetBaudRate();
}

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//   End:
