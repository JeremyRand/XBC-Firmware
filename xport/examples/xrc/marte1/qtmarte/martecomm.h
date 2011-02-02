#ifndef MARTECOMM_H
#define MARTECOMM_H

#include <vector>
#include <string>
#include "serialstream.h"

class MarteTaskStatus
{
	int m_taskID;
	int m_taskStatus;
public:
	MarteTaskStatus(int taskID, int taskStatus) : m_taskID(taskID), m_taskStatus(taskStatus) { }
	~MarteTaskStatus() {}
	int GetTaskID() { return m_taskID; }
	int GetTaskStatus() { return m_taskStatus; }
};

class MarteComm
{
	/* Serial connection class */
	SerialStream m_serial;

	/* Arrays for board features */
	unsigned short m_analogValues[8];
	unsigned char m_digitalValues[19];
	int m_motorCurrentPosition[10];
	int m_motorCurrentVelocity[10];
	int m_motorGoalPosition[10];
	int m_boardID;
public:
	MarteComm();
	~MarteComm();
	
	/* Conversion functions for loading data into packets */
	void ConvertShort(unsigned char* packet, short num);
	void ConvertInt(unsigned char* packet, int num);

	/* Serial Functions */
	
	//Get ports ((*itr).name for port name as std::string)
	std::vector<PortTest> GetSerialPorts(); 
	//Connect to specified port, return 
	bool ConnectToPort(std::string portName); 
	//Disconnect from the serial port
	void DisconnectFromPort(); 
	//Returns true is port is connected, false otherwise
	bool IsPortConnected(); 
	
	/* Bare packet sending function, used for GUI Commands */
	bool SendPacket(unsigned char* packet, int length);

	/* Task Management Functions */
	//Retreives a list of currently running functions, and their phases
	std::vector<MarteTaskStatus> GetTaskList(); 
	//Starts the requested task, with the requested parameter (Assuming the task needs one)
	//Returns true if send (but not start) is successful, false otherwise
	bool StartTask(int taskID, int taskParameter = -1); 
	//Stops the requested task. 
	//Returns true if send (but not stop) is successful, false otherwise
	bool StopTask(int taskID);
	
	/* Status Management Functions */
	//Update the status arrays in the class
	//Returns true if update happens, false otherwise
	bool UpdateStatus();
	//Gets the last updated velocity of the requested motor
	int GetMotorCurrentVelocity(int motorIndex);
	//Gets the last updated current position of the requested motor
	int GetMotorCurrentPosition(int motorIndex);
	//Gets the last updated goal position of the requested motor
	int GetMotorGoalPosition(int motorIndex);
	//Gets the last updated value of the requested analog sensor
	int GetAnalogValue(int sensorIndex);
	//Gets the last updated value of the requested digital sensor
	int GetDigitalValue(int sensorIndex);

	int GetBoardID() { return m_boardID; }
};


#endif