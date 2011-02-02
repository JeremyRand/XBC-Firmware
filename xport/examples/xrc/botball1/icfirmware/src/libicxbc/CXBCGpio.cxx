#include "CXBCGpio.h"
#include "CInterruptContSingleton.h"
#include "string.h"
#include "CPrintBuffer.h"
#include "simptimer.h"

CXBCGpio::CXBCGpio() : CGpioInt(InterruptContSingleton.InstancePtr())
{
	// reset data state
	*m_data = 0;
	memset(m_intType, 0, 32);
}

CXBCGpio::~CXBCGpio()
{

}

void CXBCGpio::SetPortDirection(unsigned short sensorPort, unsigned char portDirection)
{
  PrintToBuffer("Setting Port %d to %d\n", sensorPort, portDirection);
  //Check to see if port number is above 15
  //Return if too high
  if(sensorPort > 15) return;

  //Check to see if port is already set to another type (encoder/sonar)
  //Return if true
  if(m_intType[sensorPort] != NO_TYPE) return;

  //portDirection = 0 - Set port to input
  //portDirection = 1 - Set port to output
  if(!portDirection) *m_dataDir &= 0xffff & ~(1 << sensorPort);
  else *m_dataDir |= 0xffff & (1<<sensorPort);
}

void CXBCGpio::SetPortValue(unsigned short sensorPort, unsigned char portValue)
{
  PrintToBuffer("Setting Port %d value to %d\n", sensorPort, portValue);
  //Check to see if port number is above 15
  //Return if too high
  if(sensorPort > 15) return;

  //Check to see if port is set to output
  //Return if set to input (can't set input values)
  //  if(!(*m_dataDir & (0xffff & 1 << sensorPort))) return;
  
  if(!portValue) *m_data &= 0xffff & ~(1<<sensorPort);
  else *m_data |= 0xffff & (1<<sensorPort);
}

unsigned short CXBCGpio::SetEncoderPort(unsigned short sensorPort)
{
	if(sensorPort > 15) return 0;
	if(m_intType[sensorPort] != NO_TYPE && m_intType[sensorPort] != ENCODER_TYPE) return 0;
	m_intType[sensorPort] = ENCODER_TYPE;
	*m_dataDir &= 0xffff & ~(1 << sensorPort);
	*m_intMask |= (1 << sensorPort); 
	return 1;
}

void CXBCGpio::UnsetEncoderPort(unsigned short sensorPort)
{
	if(sensorPort > 15) return;
	if(m_intType[sensorPort] != ENCODER_TYPE) return;
	m_intType[sensorPort] = NO_TYPE;
	*m_intMask &= 0xffff & ~(1 << sensorPort);
}

void CXBCGpio::ResetEncoderCount(unsigned short sensorPort)
{
	if(sensorPort > 15) return;
	m_encoderArray[sensorPort] = 0;
}

unsigned int CXBCGpio::GetEncoderCount(unsigned short sensorPort)
{
	if(sensorPort >= 15) return 0;
	return m_encoderArray[sensorPort];
}

unsigned short CXBCGpio::GetDigital(unsigned short sensorPort)
{
	return (*(m_data))&(1 << sensorPort) ? 0 : 1;
}

unsigned short CXBCGpio::SetSonarPort(unsigned short sensorPort)
{
	if(sensorPort > 15) return 0;
	if((m_intType[sensorPort] != NO_TYPE && m_intType[sensorPort] != SONAR_TYPE) || (sensorPort & 0x1)) return 0;
	//Set lower port to Input, higher port to output
	m_intType[sensorPort] = SONAR_TYPE;
	m_intType[sensorPort+1] = SONAR_TYPE;
	*m_dataDir &= 0xffff & ~(1 << sensorPort);
	*m_data &= 0xffff & ~(1 << sensorPort);

	*m_dataDir |= (1 << (sensorPort+1));
	*m_data &= 0xffff & ~(1 << sensorPort+1);

	*m_intMask |= (1 << sensorPort);
	return 1;
}

unsigned int CXBCGpio::GetSonarBlocking(unsigned short sensorPort)
{
	SetSonarPort(sensorPort);
	SendSonarRing(sensorPort);
	CSimpTimer timer;
	unsigned long long currentTime;
	unsigned long long startTime;
	timer.GetCount(&startTime);
	timer.GetCount(&currentTime);
	while(m_sonarArray[sensorPort] == 0 && (currentTime - startTime) < 50000LL) timer.GetCount(&currentTime);
	return m_sonarArray[sensorPort];
}

unsigned int CXBCGpio::GetSonarTime(unsigned short sensorPort)
{
	if(sensorPort > 15) return 0;
	if(m_intType[sensorPort] != SONAR_TYPE) return 0;
	return m_sonarArray[sensorPort];
}

void CXBCGpio::SendSonarRing(unsigned short sensorPort)
{
	CSimpTimer timer;
	if(sensorPort > 15 || (sensorPort & 0x1)) return;
	if(m_intType[sensorPort] != SONAR_TYPE) return;
	m_sonarArray[sensorPort] = 0;
	*m_data |= (1 << (sensorPort+1));
	unsigned long long currentTime;
	timer.GetCount(&m_sonarTimeArray[sensorPort]);
	timer.GetCount(&currentTime);
	while(currentTime-m_sonarTimeArray[sensorPort] < 10LL) 
	{
		timer.GetCount(&currentTime);
	}
	*m_data &= 0xffff & ~(1 << (sensorPort+1));
}

// Added by Jeremy, 2007 10 07
void CXBCGpio::SetSonarJamming(short sensorPort)
{
	// -1 disables jamming, only one port can be jammed at a time.
	
	m_sonarJammingPort = sensorPort;
	
	// If we're enabling jamming on a port
	if(sensorPort > -1)
	{
		// Send the sonar ring; this will trigger the interrupt cycle for the jammed port
		SendSonarRing(sensorPort);
	}
	
	// If we're disabling jamming, then we don't have to do anything special; m_sonarJammingPort will no longer
	// cause the interrupt cycle to continue, so once an interrupt triggers, the cycle will stop on its own.
}

short CXBCGpio::GetSonarJamming()
{
	// Returns the port on which a sonar is jamming, or -1 if sonar jamming is disabled.

	return(m_sonarJammingPort);
}

void CXBCGpio::Interrupt(unsigned char vector)
{
	int i;
	unsigned short status = *m_intStatus;

	if(vector == 21) // if it's a digital port interrupt
	{
		
		for (i=0; i<16 && status; i++)
		{
			if (status&0x0001 && m_intType[i]&0x0003)
			{
				if(m_intType[i] == ENCODER_TYPE) ++m_encoderArray[i];
				else if(m_intType[i] == SONAR_TYPE)
				{
					CSimpTimer timer;
					unsigned long long currentTime;
					timer.GetCount(&currentTime);
					m_sonarArray[i] = ((currentTime-m_sonarTimeArray[i]));
					//				PrintToBuffer("%d\n", m_sonarArray[i]);
					
					// check if the current port is being jammed
					if(m_sonarJammingPort == i)
					{
						// stop timer 3 in case it's in use
						GBA_REG_TM3CNT = 0;
						
						// Start timer 3
						GBA_REG_TM3D = 62914; // start value; computes to 10.002ms
						GBA_REG_TM3CNT = 0xC1; // prescaler 64, IRQ, enabled
						
						// Register timer 3 interrupt
						m_pIntCont->Register(this, 6);
						m_pIntCont->Unmask(6);
					}
				}
			}
			status >>= 1;
		}
	}
	else if(vector == 6) // timer 3 interrupt
	{
		// Unregister timer 3 interrupt
		m_pIntCont->UnRegister(6);
		m_pIntCont->Mask(6);
		
		// stop timer 3
		GBA_REG_TM3CNT = 0;
		
		// send a sonar ring
		if(m_sonarJammingPort > -1)
		{
			SendSonarRing(m_sonarJammingPort);
		}
	}
}
