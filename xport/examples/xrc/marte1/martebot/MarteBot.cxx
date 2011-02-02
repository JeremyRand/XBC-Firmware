#include "MarteBot.h"
#include "Message.h"
#include "textdisp.h"
#include "../logic/marte1.h"
#include "string.h"


CMarteBot::CMarteBot(CInterruptCont* intcont) :
  CAxesClosedQuad(intcont, 300),
  m_adc(ADC_BASE),
  m_serial(intcont)
{
  
  m_serial.SetBaud(DUC_BAUD_38K);
  SetPIDGains(500, 0, 0);

  GPIO_DDR(0) = 0x0000; // set D15-D0 as input
  GPIO_DDR(1) = 0x0000; // set D18-D16 as input

  m_boardIndex = !GetDigital(PORT_BOARD_INDEX);
}

CMarteBot::~CMarteBot()
{
}

int CMarteBot::SendTaskStatusPackage()
{
  char* taskPackage;
  int numTasks = m_taskList.GetTaskCount();
  int checkSum = 0;
  int packageSize = 0;

  //Packet Structure - 5 bytes + NumTasks bytes
  //2 byte Command Relay
  //1 byte for number of tasks in list
  //2 bytes per task (TaskID + TaskPhase)
  //2 bytes for checksum

  packageSize = (2+1+(numTasks*2)+2);
  taskPackage = new char[sizeof(unsigned char) * packageSize];
  taskPackage[0] = 0;
  taskPackage[1] = TASK_SEND_TASK_STATUS;
  taskPackage[2] = numTasks * 2;
  checkSum = TASK_SEND_TASK_STATUS + numTasks;
  int taskIdx = 3;
  if(numTasks > 0)
    {
      CTaskList::CTaskNode* taskItr = m_taskList.GetListPointer();
      while(1)
	{
	  taskPackage[taskIdx] = taskItr->GetTaskID();
	  ++taskIdx;
	  taskPackage[taskIdx] = taskItr->GetTaskPhase();
	  ++taskIdx;
	  checkSum += taskItr->GetTaskID() + taskItr->GetTaskPhase();
	  if(taskItr->GetNextNode() == NULL) break;
	  taskItr = taskItr->GetNextNode();
	}
    }
  taskPackage[taskIdx] = checkSum >> 8;
  ++taskIdx;
  taskPackage[taskIdx] = checkSum & 0x00ff;
  m_serial.Send(taskPackage, packageSize, 50);
  delete taskPackage;
  return -1;
}

int CMarteBot::SendStatusPackage()
{
  /*
    Status Packet Format - 159 bytes:
    - 2 byte Command Relay (Same as command number to get the status packet)
    - 1 byte Board ID
    - 10 motors @ 12 bytes each = 120 bytes
    - 4 byte motor velocity
    - 4 byte motor current position
    - 4 byte motor goal position
    - 19 digital ports @ 1 byte each = 19 bytes
    - 8 analog ports @ 2 bytes each = 16 bytes
    - 2 byte Checksum
  */

  unsigned char package[160];
  memset(package, 0, 160);
  int packageCursor = 3;

  int velVector[DAO_MAX_AXES];
  GetTrajectoryVelocityVector(velVector);

  int curPos, curVel, goalPos;
  package[0] = 0;
  package[1] = TASK_SEND_BOT_STATUS;
  //Get status of port 19 for board identification
  package[2] = m_boardIndex; 

  for(int i = 0; i < 10; ++i)
    {
      //Get Velocity
      curVel = velVector[i];
      package[packageCursor] = (unsigned char)(curVel >> 24);
      ++packageCursor;
      package[packageCursor] = (unsigned char)((curVel & 0x00ff0000) >> 16);
      ++packageCursor;
      package[packageCursor] = (unsigned char)((curVel & 0x0000ff00) >> 8);
      ++packageCursor;
      package[packageCursor] = (unsigned char)(curVel & 0x000000ff);
      ++packageCursor;


      //Get Current Position
      curPos = GetPosition(i);
      package[packageCursor] = (unsigned char)(curPos >> 24);
      ++packageCursor;
      package[packageCursor] = (unsigned char)((curPos & 0x00ff0000) >> 16);
      ++packageCursor;
      package[packageCursor] = (unsigned char)((curPos & 0x0000ff00) >> 8);
      ++packageCursor;
      package[packageCursor] = (unsigned char)(curPos & 0x000000ff);
      ++packageCursor;

      //Get Goal Position
      goalPos = GetGoalPosition(i);
      package[packageCursor] = (unsigned char)(goalPos >> 24);
      ++packageCursor;
      package[packageCursor] = (unsigned char)((goalPos & 0x00ff0000) >> 16);
      ++packageCursor;
      package[packageCursor] = (unsigned char)((goalPos & 0x0000ff00) >> 8);
      ++packageCursor;
      package[packageCursor] = (unsigned char)(goalPos & 0x000000ff);
      ++packageCursor;
    }
  for(int i = 0; i < 19; ++i, ++packageCursor)
    {
      package[packageCursor] = GetDigital(i);
    }
  for(int i = 0; i < 8; ++i)
    {
      package[packageCursor] = (unsigned char)(m_adc.GetChannel(i) >> 8);
      ++packageCursor;
      package[packageCursor] = (unsigned char)(m_adc.GetChannel(i) & 0x00FF);
      ++packageCursor;
    }
  m_serial.Send((char*)package, 160, 500);
  return -1;
}

void CMarteBot::RunRobotLoop()
{
  int readSize = 0;
  printf("Running Robot Loop\n");
  while(1)
    {
      //Check for half finished command
      /*
	if(readSize == 1)
	{
	printf("Half send\n");
	//If we've already read one byte, try to read the next one to form the full task packet
	readSize = m_serial.Receive(m_currentCommand+1, 1, 500);
	if(readSize == 1)
	{

	readSize = 0;
	m_taskList.AddTask(m_currentCommand[0] << 8 | m_currentCommand[1]);
	}
	}
	else if(readSize == 0)
      */
      {
	readSize = m_serial.Receive(m_currentCommand, 2, 1);
	//If we have a full task value, reset and run the task
	if(readSize == 2)
	  {

	    readSize = 0;
	    int taskID = m_currentCommand[0] << 8 | m_currentCommand[1];
	    printf("Adding task %d\n", taskID);
	    char taskParameter[4];
	    char taskChecksum[2];
	    memset(taskParameter, 0, 4);
	    memset(taskChecksum, 0, 2);
	    int i, j;
	    if(taskID > 10)
	      {
		i = m_serial.Receive(taskParameter, 4, 1);
		j = m_serial.Receive(taskChecksum, 2, 1);
	      }
	    if(!m_taskList.IsTaskRunning(taskID)) m_taskList.AddTask(taskID, taskParameter[0] << 24 | taskParameter[1] << 16 | taskParameter[2] << 8 | taskParameter[3]);
	  }
      }
      //Run through task list
      RunTask();
    }
}

int CMarteBot::GetDigital(int port)
{
  if(port < 0 || port > 19) return 0;
  if(port < 16) return (unsigned char)((GPIO_DATA(0) & (1 << port)) != 0 ? 0 : 1);
  else return (unsigned char)((GPIO_DATA(1) & (1 << (port - 16))) != 0 ? 0 : 1);
}


/************* EDITABLE CODE STARTS HERE ****************/

//Task switching function

void CMarteBot::RunTask()
{
  CTaskList::CTaskNode* taskList = m_taskList.GetListPointer();
  if(!taskList) return;
  if(taskList->GetTaskID() == 0) return;
  int taskPhase = 0;
  int returnValue = 0;
  while(1)
    {
      switch (taskList->GetTaskID())
	{
	  /******** DO NOT EDIT ********/
	  //GUI Motor Movement Functions
	case TASK_RUN_MOTOR:
	  returnValue = MoveToPosition();
	  break;
	case TASK_SET_MOTOR_POSITION:
	  returnValue = SetPosition();
	  break;
	case TASK_HOLD_MOTOR_POSITION:
	  returnValue = HoldPosition();
	  break;

	  //Status Retrieval Functions
	case TASK_SEND_BOT_STATUS:
	  returnValue = SendStatusPackage();
	  break;

	  //Task Status Retrieval Function
	case TASK_SEND_TASK_STATUS:
	  returnValue = SendTaskStatusPackage();
	  break;

	case TASK_STOP_ALL:
	  returnValue = StopAll();
	  break;

	  /******** EDIT HERE ********/
	  //Bot Tasks

	case TASK_PLACE_LINER:
	  returnValue = PlaceLiner();
	  break;

	case TASK_ACCEPT_CORE:
	  returnValue = AcceptCore();
	  break;
	case TASK_TIGHTEN_CLAMP:
	  returnValue = TightenClamp();
	  break;
	case TASK_FACE_CORE:
	  returnValue = FaceCore();
	  break;
	case TASK_CRUSH_SUBSAMPLE:
	  returnValue = CrushSubsample();
	  break;
	case TASK_TRANSFER_SAMPLE:
	  returnValue = TransferSample();
	  break;
	case TASK_DISCARD_TUBE:
	  returnValue = DiscardTube();
	  break;
	case TASK_RESET_ACTUATORS_1:
	case TASK_RESET_ACTUATORS_2:
	  printf("Running Reset Actuators Task\n");
	  returnValue = ResetActuators();
	  break;
	case TASK_RELEASE_CLAMP:
	  returnValue = ReleaseClamp();
	  break;
	case TASK_EMPTY_CLAMP:
	  returnValue = EmptyClamp();
	  break;
	case TASK_GRASP_SAMPLE:
	  returnValue = GraspSample();
	  break;
	case TASK_CUT_SUBSAMPLE:
	  returnValue = CutSubsample();
	  break;

	  //Tasks requiring one parameter
	case TASK_MOVE_CLAMP:
	  returnValue = MoveClampToPosition();
	  break;
	case TASK_STORE_CORE:
	  returnValue = StoreCoreAtPosition();
	  break;
	case TASK_RETRIEVE_CORE:
	  returnValue = RetrieveCoreFromPosition();
	  break;


	case TASK_ABORT_TASK:
	  returnValue = CancelTask();
	  break;

	default:
	  printf("Cannot find task %d\n", taskList->GetTaskID());
	  returnValue = -1;
	}
      if(returnValue == -1)
	{
	  //If this is the last node in our list, return after we delete it.
	  if(taskList->GetNextNode() == NULL)
	    {
	      printf("Removing task %d\n", taskList->GetTaskID());
	      m_taskList.RemoveTask(taskList->GetTaskID());
	      return;
	    }
	  int oldTaskID = taskList->GetTaskID();
	  taskList = taskList->GetNextNode();
	  m_taskList.RemoveTask(oldTaskID);
	  printf("Removing task %d\n", oldTaskID);
	}
      else
	{
	  if(taskList->GetNextNode() == NULL)
	    {
	      return;
	    }
	  m_taskList.SetTaskPhase(taskList->GetTaskID(), returnValue);
	  taskList = taskList->GetNextNode();
	}
    }
}

//
//Motor Command Functions for GUI
//

int CMarteBot::MoveToPosition()
{
  char motorParameters[13];
  int iMotor, iVel, iPosition;

  m_serial.Receive(motorParameters, 11, 5);
  iMotor = motorParameters[0];
  iVel = motorParameters[1] << 24 |  motorParameters[2] << 16 |  motorParameters[3] << 8 | motorParameters[4];
  iPosition = motorParameters[5] << 24 |  motorParameters[6] << 16 |  motorParameters[7] << 8 | motorParameters[8];
  //  Stop(iMotor);
  //Execute();
  printf("Moving motor %d\n", iMotor);
  Move(iMotor, iPosition, iVel, 10000);
  Execute();
  return -1;
}

int CMarteBot::HoldPosition()
{
  printf("Holding %d\n", m_taskList.GetTaskParameter(TASK_HOLD_MOTOR_POSITION));
  Stop(m_taskList.GetTaskParameter(TASK_HOLD_MOTOR_POSITION));
  Execute();
  SyncTrajectory(m_taskList.GetTaskParameter(TASK_HOLD_MOTOR_POSITION));
  Hold(m_taskList.GetTaskParameter(TASK_HOLD_MOTOR_POSITION),true);
  Execute();
  return -1;
}

int CMarteBot::SetPosition()
{
  char motorParameters[7];
  int iMotor, iPosition;
  m_serial.Receive(motorParameters, 7, 5);
  iMotor = motorParameters[0];
  iPosition = motorParameters[1] << 24 |  motorParameters[2] << 16 |  motorParameters[3] << 8 | motorParameters[4];
  CAxesOpenQuad::SetPosition(iMotor, iPosition);
  return -1;
}

int CMarteBot::CancelTask()
{
  int taskID = m_taskList.GetTaskParameter(TASK_ABORT_TASK);
  printf("Canceling task %d\n", taskID);

  if(m_taskList.IsTaskRunning(taskID))
    {
      m_taskList.SetTaskPhase(taskID, -1);
    }
  return -1;
}
	
int CMarteBot::StopAll()
{
  for(int i = 0; i < 10; ++i)
    {
      Stop(i);
      CAxesOpenQuad::SetPWM(i,0);
    }
  Execute();
  CTaskList::CTaskNode* taskNode = m_taskList.GetListPointer();
  while(taskNode != NULL)
    {
      taskNode->SetTaskPhase(-1);
      if(taskNode->GetNextNode() == NULL) break;
      taskNode = taskNode->GetNextNode();
    }
  return -1;
}


#include "MarteBotTasks.cpp"
  
 
