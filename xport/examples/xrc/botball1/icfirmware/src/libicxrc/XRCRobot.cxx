#include "XRCRobot.h"
#include "CBluetoothDevice.h"
#include "CNullCommDevice.h"
#include "CPrintBuffer.h"
#include "textdisp.h"
#include "ICRobotContext.h"
#include "pcodesim.h"

XRCRobot::XRCRobot()  
#ifndef NO_GPIO
: m_gpio()
#endif
{
	//m_commDevice = new CBluetoothDevice();
	printf("Initializing Null Comm\n");
	m_commDevice = new CNullCommDevice();
	setMLTranslator(this);
}

XRCRobot::~XRCRobot()
{
	if(m_commDevice) delete m_commDevice;
}

void XRCRobot::RunRobotLoop()
{
	// Create an ICRobotContext and push onto the top of the
	// context stack
	ICRobotContext::Push(*this);

	PrintToBuffer("Welcome to IC 6.1\nBy KISS Institute for\nPractical Robotics\n");
	while(1)
	{	
		// Process communications and virtual machine
		ICRobot::RunRobotLoop();
		// Process whatever is on the top of the context stack
		DispContextStackSingleton.Process();
	}
}

#ifndef NO_GPIO
unsigned short XRCRobot::GetDigital(const unsigned short sensorPort)
{
	m_gpio.SetMode(0x00);
	m_gpio.SetGpioDataDir(0x00);
	unsigned short data = m_gpio.GetGpioData();
	PrintToBuffer("%x\n", data);
	return data&(1 << ((2*sensorPort)-1)) ? 0 : 1;
}
#endif

bool XRCRobot::HandleMenuEvent(int eventType, CMenuElement &menu)
{
	if(eventType < 100)
	{
		return ICRobot::HandleMenuEvent(eventType, menu);
	}
	return true;
}

short XRCRobot::CallML1Translator(const short& functionIndex, const short& argument1)
{
	return ICRobot::CallML1Translator(functionIndex, argument1);
}

short XRCRobot::CallML2Translator(const short& functionIndex, const short& argument1, const short& argument2)
{
	return ICRobot::CallML2Translator(functionIndex, argument1, argument2);
}

short XRCRobot::CallML3Translator(const short& functionIndex, const short& argument1, const short& argument2, const short& argument3)
{
	return ICRobot::CallML3Translator(functionIndex, argument1, argument2, argument3);
}

