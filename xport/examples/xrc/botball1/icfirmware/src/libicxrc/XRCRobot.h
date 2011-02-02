#ifndef XRCROBOT_H
#define XRCROBOT_H

#include "ICRobot.h"
#include "xirgpio.h"

class XRCRobot : public ICRobot
{
protected:
#ifndef NO_GPIO
	CXirGpio m_gpio;
#endif
public:
	XRCRobot();
	~XRCRobot();
	virtual short CallML1Translator(const short& functionIndex, const short& argument1);
	virtual short CallML2Translator(const short& functionIndex, const short& argument1, const short& argument2);
	virtual short CallML3Translator(const short& functionIndex, const short& argument1, const short& argument2, const short& argument3);
	bool HandleMenuEvent(int eventType, CMenuElement &menu);
	virtual void RunRobotLoop();
#ifndef NO_GPIO
	virtual unsigned short GetDigital(const unsigned short sensorPort);
#endif
};

#endif
