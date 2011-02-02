#ifndef CSTATUSBAR_H
#define CSTATUSBAR_H

#include "simptimer.h"
#define DG_BASE					0x9ffd200

class ICRobot;
class CTextBuf;


class CStatusBar
{
	volatile unsigned short * m_data;
	volatile unsigned short * m_dataDir;
	short pin;

	unsigned short m_priorState;
	unsigned short m_currentState;
	char m_lineContent[2][60];
	char m_lineMask[2][60];
	CTextBuf *m_infoBuf;
	CTextBuf *m_infoScreenBuf;
	CSimpTimer m_timer;
public:
	enum
	{
		INFO_STATE = 0,
		SENSOR_STATE,
		MOTOR_STATE,
		COMM_STATE,
		BTN_STATE,
		INTRPT_STATE,
		OTHER_STATE,
		STATE_COUNT
	};

	CStatusBar();
	~CStatusBar();
	void SetupDisplay();
	void UpdateStatusBar(ICRobot& robot);
	void IncreaseStatusBarState();
	void DecreaseStatusBarState();
	void SetStatusBarState(unsigned short stateIdx) { m_currentState = stateIdx; }

    // Set the text of the info page of the status bar
	void InfoPrint(char *message);
	void InfoPrintf(char *format, ...);
};

#endif 
