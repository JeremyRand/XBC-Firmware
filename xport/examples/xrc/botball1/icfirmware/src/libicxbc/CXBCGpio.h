#include "gpioint.h"

class CXBCGpio : public CGpioInt
{
	enum
	{
		NO_TYPE = 0,
		ENCODER_TYPE,
		SONAR_TYPE
	};
	unsigned int m_encoderArray[16];
	unsigned int m_sonarArray[16];
	unsigned long long m_sonarTimeArray[16];
	unsigned short m_intType[16];
	
	// Added by Jeremy, 2007 10 07
	short m_sonarJammingPort; // -1 indicates no jamming
	
public:
	CXBCGpio();
	~CXBCGpio();
	
	unsigned short GetDigital(unsigned short sensorPort);
	
	unsigned short SetEncoderPort(unsigned short sensorPort);
	void UnsetEncoderPort(unsigned short sensorPort);
	void ResetEncoderCount(unsigned short sensorPort);
	unsigned int GetEncoderCount(unsigned short sensorPort);
	
	unsigned short SetSonarPort(unsigned short sensorPort);
	void SendSonarRing(unsigned short sensorPort);
	unsigned int GetSonarBlocking(unsigned short sensorPort);
	unsigned int GetSonarTime(unsigned short sensorPort);

	// Added by Jeremy, 2007 10 07
	void SetSonarJamming(short sensorPort);
	short GetSonarJamming();

  int GetPortDirection(unsigned short sensorPort){
  if(sensorPort > 15) return -1;
  return ((*m_dataDir & (1<<sensorPort))>0);
}

	void SetPortDirection(unsigned short sensorPort, unsigned char portDirection);
	void SetPortValue(unsigned short sensorPort, unsigned char portValue);
	
	virtual void Interrupt(unsigned char vector);
};
