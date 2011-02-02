#include "martecomm.h"
#include "TaskDefs.h"

MarteComm::MarteComm()
{
}

MarteComm::~MarteComm()
{
	DisconnectFromPort();
}

std::vector<PortTest> MarteComm::GetSerialPorts()
{
	return m_serial.findPorts(true);
}

bool MarteComm::ConnectToPort(std::string portName)
{
	m_serial.setFormat(8, 'n', 1);
	m_serial.setSpeed(38400);
	try
	{
		m_serial.connect(portName);
	}
	catch (SerialStreamError e) 
	{
		return false;
	}
	return true;
}

void MarteComm::DisconnectFromPort()
{
	m_serial.disconnect();
}

bool MarteComm::IsPortConnected()
{
	return m_serial.isConnected();
}

bool MarteComm::SendPacket(unsigned char* packet, int length)
{
	if(!m_serial.isConnected()) return false;
	try
	{
		m_serial.write((const unsigned char*)packet, length);
	}
	catch (SerialStreamError e)
	{
		m_serial.disconnect();
		return false;
	}
	return true;
}

std::vector<MarteTaskStatus> MarteComm::GetTaskList()
{

	std::vector<MarteTaskStatus> taskVector;
	if(!m_serial.isConnected()) return taskVector;

	StartTask(TASK_SEND_TASK_STATUS, 0);

	unsigned char statusInfo[3];
	m_serial.readTimeout(statusInfo, 3, 500);

	unsigned int statusSize = statusInfo[2];
	unsigned char* taskList = new unsigned char[statusSize];
	m_serial.readTimeout(taskList, statusSize, 500);

	unsigned char statusSum[2];
	m_serial.readTimeout(statusSum, 2, 500);

	for(int i = 0; i < statusSize; i += 2)
	{
		taskVector.push_back(MarteTaskStatus(taskList[i], taskList[i+1]));
	}

	return taskVector;
}

bool MarteComm::StartTask(int taskID, int taskParameter)
{
	if(!m_serial.isConnected()) return false;
	unsigned char taskCommand[8];
	ConvertShort(taskCommand, taskID);
	ConvertInt(taskCommand+2, taskParameter);
	ConvertShort(taskCommand+6, 0);
	return SendPacket(taskCommand, 8);
}

bool MarteComm::StopTask(int taskID)
{
	return StartTask(TASK_ABORT_TASK, taskID);
}

bool MarteComm::UpdateStatus()
{
	if(!m_serial.isConnected()) return false;
	if(!StartTask(TASK_SEND_BOT_STATUS, 0)) return false;
	unsigned char status[160];
	int readResult = m_serial.readTimeout(status, 160, 500);
	if(readResult < 160)
	{
		DisconnectFromPort();
		return false;
	}
	int statusPos = 3;
	m_boardID = status[2];
	//Motors
	for(int m = 0; m < 10; ++m, statusPos += 12)
	{
		m_motorCurrentVelocity[m] = status[statusPos] << 24 | status[statusPos+1] << 16 | status[statusPos+2] << 8 | status[statusPos+3];
		m_motorCurrentPosition[m] = status[statusPos+4] << 24 | status[statusPos+5] << 16 | status[statusPos+6] << 8 | status[statusPos+7];
		m_motorGoalPosition[m] = status[statusPos+8] << 24 | status[statusPos+9] << 16 | status[statusPos+10] << 8 | status[statusPos+11];
	}
	//Digitals
	for(int i = 0; i < 19; ++i, ++statusPos)
	{
		m_digitalValues[i] = status[statusPos];
	}
	//Analogs
	for(int i = 0; i < 8; ++i, statusPos += 2)
	{
		m_analogValues[i] = (status[statusPos] << 8) | status[statusPos+1];
	}
	return true;
}

int MarteComm::GetMotorCurrentVelocity(int motorIndex)
{
	if(motorIndex < 0 || motorIndex > 19) return -1;
	return m_motorCurrentVelocity[motorIndex];
}

int MarteComm::GetMotorCurrentPosition(int motorIndex)
{
	if(motorIndex < 0 || motorIndex > 19) return -1;
	return m_motorCurrentPosition[motorIndex];
}

int MarteComm::GetMotorGoalPosition(int motorIndex)
{
	if(motorIndex < 0 || motorIndex > 19) return -1;
	return m_motorGoalPosition[motorIndex];
}

int MarteComm::GetAnalogValue(int sensorIndex)
{
	if(sensorIndex < 0 || sensorIndex > 8) return -1;
	return m_analogValues[sensorIndex];
}

int MarteComm::GetDigitalValue(int sensorIndex)
{
	if(sensorIndex < 0 || sensorIndex > 19) return -1;
	return m_digitalValues[sensorIndex];
}

void MarteComm::ConvertInt(unsigned char* str, int n)
{
	for(int i = 0; i < 4; ++i) {
		str[i] = (n & (0xff000000 >> (i*8))) >> ((3-i) * 8);
	} 
}

void MarteComm::ConvertShort(unsigned char* str, short n)
{
	for(int i = 0; i < 2; ++i)
	{
		str[i] = (n & (0xff00 >> (i*8))) >> ((1-i) * 8);
	}    
}