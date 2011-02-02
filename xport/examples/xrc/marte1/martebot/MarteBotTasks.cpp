/********************************************************************
Instructions on how to edit this file

Each task is made up of a switch statement. At the begining of every
task, the phase is retrieved, and that is used to figure out what 
task the phase should be executing. Each task phase is documented in
the comments preceding the case statement for the phase.

All motors, speed, accelerations, and positions (except for 
the position 0, in which case the actual number 0 is used) are 
defined in the TaskDefs.h file. All changes to these constants should
be made in that file, to keep this file as free of magic numbers as
possible. 

Here are all of the functions that are used to execute tasks:

Move(<Motor name>, <Position Name>, <Speed Name>, <Accel Name>)
- Puts a command in the queue that will move a motor at to a certain
position, at a certain speed, with a certain velocity. Command does
not actually run until Execute() is called.

Hold(<Motor name>)
- Puts a command in the queue that makes the motor hold its 
current position. Command does not actually run until Execute() is 
called.

Stop(<Motor name>)
- Puts a command in the queue that stops the motor  
moving or holding. Command does not actually run until Execute() 
is called.

Execute()
- Runs all of the commands that have been queued since the last 
Execute() call.

Done(<Motor name>) 
- returns turn if the motor is done moving to where it was told.

CAxesOpenQuad::SetPWM(<Motor Name>, <Speed Name>)
- Sets the specified motor to the specified PWM speed. The motor
will run at this speed until a Hold, Stop, or Move is called, or else
the PWM is reset by another call to SetPWM

CAxesOpenQuad::GetPWM(<Motor Name>)
- Gets the PWM value from the specified motor

GetDigital(port)
- Returns an int with the status of the requested digital port

GetAnalog(port)
- Returns an int with the status of the requested analog port
********************************************************************/

/******************************************************************************/

int CMarteBot::PlaceLiner()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_PLACE_LINER);
  
  switch(taskPhase)
    {
      //Turn motor until paper sensor is true
    case 1:
      CAxesOpenQuad::SetPWM(MOTOR_LINER_FEEDER, SPEED_LINER_FEEDER_PWM);
      
      //Wait for paper sensor
      //Block is handled in the Periodic function
    case 2:
      if(GetDigital(PORT_LINER_FEEDER_BREAK_BEAM)) return 3;
      return 2;

      //Fire solenoid until paper sensor is false
    case 3:
      CAxesOpenQuad::SetPWM(MOTOR_LINER_FEEDER, 0);
      CAxesOpenQuad::SetPWM(MOTOR_LINER_FEEDER_SOLENOID, SPEED_LINER_FEEDER_SOLENOID_PWM);

      //Wait for paper sensor
      //Block is handled in the Periodic function
    case 4:
      if(!GetDigital(PORT_LINER_FEEDER_BREAK_BEAM)) return 5;
      return 4;

      //Release Solenoid and Turn off motors
    case 5:
      CAxesOpenQuad::SetPWM(MOTOR_LINER_FEEDER, 0);
      CAxesOpenQuad::SetPWM(MOTOR_LINER_FEEDER_SOLENOID, 0);
      return -1;
    }

  return -1;
}

/******************************************************************************/

// Move clamp to end of rail and open jaws
int CMarteBot::AcceptCore()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_RAIL_HIGH_SPEED to 0
      //MOTOR_CORE_CLAMP to POSITION_CORE_CLAMP_OPEN
    case 1:
      Move(MOTOR_RAIL_HIGH_SPEED, 0, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Move(MOTOR_CORE_CLAMP, POSITION_CORE_CLAMP_OPEN, SPEED_CORE_CLAMP_MAX, ACCEL_CORE_CLAMP_MAX);
      Execute();

      //Block Until Done
    case 2:
      if(Done(MOTOR_RAIL_HIGH_SPEED) && Done(MOTOR_CORE_CLAMP))
	{
	  return -1;
	}
      return 2;
    }
  return -1;
}

/******************************************************************************/

//Tightens Clamp
int CMarteBot::TightenClamp()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_TIGHTEN_CLAMP);
  switch(taskPhase)
    {
      //MOTOR_CORE_CLAMP moving to position POSITION_CORE_CLAMP_CLOSED
    case 1:
      Move(MOTOR_CORE_CLAMP, POSITION_CORE_CLAMP_CLOSED, SPEED_CORE_CLAMP_MAX, ACCEL_CORE_CLAMP_MAX);
      Execute();

      //Block until done or speed < SPEED_CORE_CLAMP_MIN
    case 2:
      if(Done(MOTOR_CORE_CLAMP) || GetPWM(MOTOR_CORE_CLAMP) < SPEED_CORE_CLAMP_MIN)
	{
	  Stop(MOTOR_CORE_CLAMP);
	  return -1;
	}
      return 2;
    }
  return -1;
}

/******************************************************************************/

//Moves clamp to end of rail, lowers and turns on saw, slowly moves core past saw, raises saw
int CMarteBot::FaceCore()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_FACE_CORE);
  switch(taskPhase)
    {
      //MOTOR_RAIL_HIGH_SPEED to 0
    case 1:
      Move(MOTOR_RAIL_HIGH_SPEED, 0, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Execute();

      //Block until Case 1 is done
    case 2:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return 3;
	}
      return 2;

      //MOTOR_FACE_SAW_TILT To 0
    case 3:
      Move(MOTOR_FACE_SAW_TILT, 0, SPEED_FACE_SAW_TILT_MAX, ACCEL_FACE_SAW_TILT_MAX);
      Execute();

      //Block until Case 3 is done
    case 4:
      if(Done(MOTOR_FACE_SAW_TILT))
	{
	  return 5;
	}
      return 4;

      //Break MOTOR_RAIL_HIGH_SPEED
    case 5:
      Hold(MOTOR_RAIL_HIGH_SPEED, true);
      Execute();

      //MOTOR_FACE_SAW_DRIVE at Speed X & MOTOR_RAIL_LOW_SPEED @ speed Y to position Z
    case 6:
      //TODO: Write Face Saw Drive algorithm

      //Block until case 6 done or Face saw speed < SPEED_FACE_SAW_MIN
    case 7:

      //MOTOR_FACE_SAW_TILT to position POSITION_FACE_SAW_UP
    case 8:
      Move(MOTOR_FACE_SAW_TILT, POSITION_FACE_SAW_TILT_UP, SPEED_FACE_SAW_TILT_MAX, ACCEL_FACE_SAW_TILT_MAX);
      Execute();

      //Block until case 8 is done
    case 9:
      if(Done(MOTOR_FACE_SAW_TILT))
	{
	  return 10;
	}
      return 9;

      //MOTOR_FACE_SAW_DRIVE @ speed 0 & release MOTOR_RAIL_HIGH_SPEED
    case 10:
      SetPWM(MOTOR_FACE_SAW_DRIVE, 0);
      Hold(MOTOR_RAIL_HIGH_SPEED, false);
      Execute();
      return -1;
    }
  return -1;
}

/******************************************************************************/


//Moves clamp to specified preset position
int CMarteBot::MoveClampToPosition()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_MOVE_CLAMP);
  switch(taskPhase)
    {
      //MOTOR_RAIL_HIGH_SPEED to position determined by parameter
    case 1:
      //Block until done
    case 2:
      return -1;
    }
  return -1;
}

/******************************************************************************/

//Store core in storage spot N
int CMarteBot::StoreCoreAtPosition()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_STORE_CORE);
  switch(taskPhase)
    {
      //MOTOR_RAIL_HIGH_SPEED to Position RAIL_NEAR_STORAGE_POSITION
    case 1:
      Move(MOTOR_RAIL_HIGH_SPEED, POSITION_RAIL_HIGH_SPEED_NEAR_STORAGE, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Execute();

      //Block until case 1 is done
    case 2:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return 3;
	}
      return 2;

      //Core Clamp Storage to position N
    case 3:
      //TODO: Write Core Clamp Movement code

      //Block Until case 4 is Done
    case 4:

      //MOTOR_RAIL_HIGH_SPEED to position POSITION_RAIL_HIGH_SPEED_DOCK
    case 5:
      Move(MOTOR_RAIL_HIGH_SPEED, POSITION_RAIL_HIGH_SPEED_DOCK, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Execute();

      //Block Until case 5 is done
    case 6:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return 7;
	}
      return 6;

      //MOTOR_CART_CLAMP_RELEASE to position POSITION_CART_CLAMP_LOOSE
    case 7:
      Move(MOTOR_CART_CLAMP_RELEASE, POSITION_CART_CLAMP_RELEASE_LOOSE, SPEED_CART_CLAMP_RELEASE_MAX, ACCEL_CART_CLAMP_RELEASE_MAX);
      Execute();

      //Block Until case 6 is done
    case 8:
      if(Done(MOTOR_CART_CLAMP_RELEASE))
	{
	  return 9;
	}
      return 8;

      //MOTOR_RAIL_HIGH_SPEED to position POSITION_RAIL_NEAR_STORAGE
    case 9:
      Move(MOTOR_RAIL_HIGH_SPEED, POSITION_RAIL_HIGH_SPEED_NEAR_STORAGE, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Execute();

      //Block until case 9 is done
    case 10:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return -1;
	}
      return 10;
    }
  return -1;
}

/******************************************************************************/

//Moves clamp from position N onto rail
int CMarteBot::RetrieveCoreFromPosition()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_RAIL_HIGH_SPEED to position POSITION_RAIL_NEAR_STORAGE
    case 1:
      Move(MOTOR_RAIL_HIGH_SPEED, POSITION_RAIL_HIGH_SPEED_NEAR_STORAGE, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Execute();

      //Block until Case 1 is done
    case 2:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return 3;
	}
      return 2;

      //MOTOR_CORE_CLAMP to position specified by parameter
    case 3:
      //TODO:Write Core Clamp Storage movement

      //Block until Case 3 is done
    case 4:
      if(Done(MOTOR_CORE_CLAMP))
	{
	  return 5;
	}
      return 4;

      //MOTOR_CART_CLAMP_RELEASE to position POSITION_CART_CLAMP_LOOSE
    case 5:
      Move(MOTOR_CART_CLAMP_RELEASE, POSITION_CART_CLAMP_RELEASE_LOOSE, SPEED_CART_CLAMP_RELEASE_MAX, ACCEL_CART_CLAMP_RELEASE_MAX);
      Execute();

      //Block until case 5 is done
    case 6:
      if(Done(MOTOR_CART_CLAMP_RELEASE))
	{
	  return 7;
	}
      return 6;

      //MOTOR_RAIL_HIGH_SPEED to position POSITION_RAIL_HIGH_SPEED_DOCK
    case 7:
      Move(MOTOR_RAIL_HIGH_SPEED, POSITION_RAIL_HIGH_SPEED_DOCK, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Execute();

      //Block until case 7 is done
    case 8:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return 9;
	}
      return 8;

      //MOTOR_CART_CLAMP_RELEASE to position POSITION_CART_CLAMP_TIGHT
    case 9:
      Move(MOTOR_CART_CLAMP_RELEASE, POSITION_CART_CLAMP_RELEASE_TIGHT, SPEED_CART_CLAMP_RELEASE_MAX, ACCEL_CART_CLAMP_RELEASE_MAX);
      Execute();

      //Block until case 9 is done
    case 10:
      if(Done(MOTOR_CART_CLAMP_RELEASE))
	{
	  return 11;
	}
      return 10;

      //MOTOR_RAIL_HIGH_SPEED to position POSITION_RAIL_HIGH_SPEED_NEAR_STORAGE
    case 11:
      Move(MOTOR_RAIL_HIGH_SPEED, POSITION_RAIL_HIGH_SPEED_NEAR_STORAGE, SPEED_RAIL_HIGH_SPEED_MAX, ACCEL_RAIL_HIGH_SPEED_MAX);
      Execute();

      //Block until case 11 is done
    case 12:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return -1;
	}
      return 12;
    }
  return -1;
}

/******************************************************************************/

//Moves to 1 of 12 subsample positions
int CMarteBot::MoveToSubsamplePosition()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_RAIL_HIGH_SPEED to position specified by parameter
    case 1:
      //TODO: Write Move to Subsample Position code

      //Block until case 1 is done
    case 2:
      if(Done(MOTOR_RAIL_HIGH_SPEED))
	{
	  return -1;
	}
      return 2;
    }
  return -1;
}

/******************************************************************************/

//Stops blades and grasps sample
int CMarteBot::GraspSample()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //Brake MOTOR_SUBSAMPLE_SAW_DRIVE
    case 1:
      SyncTrajectory(MOTOR_SUBSAMPLE_SAW_DRIVE);
      Hold(MOTOR_SUBSAMPLE_SAW_DRIVE, true);
      Execute();
      return 2;
      //Move MOTOR_SUBSAMPLE_SQUEEZE to POSITION_SUBSAMPLE_SQUEEZE_CLOSED
    case 2:
      Move(MOTOR_SUBSAMPLE_SQUEEZE, POSITION_SUBSAMPLE_SQUEEZE_CLOSED, SPEED_SUBSAMPLE_SQUEEZE_MAX, ACCEL_SUBSAMPLE_SQUEEZE_MAX);
      Execute();
      return 3;

      //Block until case 2 done or speed < SPEED_SUBSAMPLE_SQUEEZE_MIN
    case 3:
      if(Done(MOTOR_SUBSAMPLE_SQUEEZE) || GetPWM(MOTOR_SUBSAMPLE_SQUEEZE) < SPEED_SUBSAMPLE_SQUEEZE_MIN)
	{
	  Stop(MOTOR_SUBSAMPLE_SQUEEZE);
	  return 4;
	}
      return 3;

      //Stop hold on MOTOR_SUBSAMPLE_SAW_DRIVE
    case 4:
      Hold(MOTOR_SUBSAMPLE_SAW_DRIVE, false);
      return -1;
    }
  return -1;
}

/******************************************************************************/

//Moves sample tube into SOLID
int CMarteBot::TransferSample()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_TUBE_STORAGE_SOLENOID to ON and return next phase
    case 1:

      //MOTOR_TUBE_STORAGE_SOLENOID to OFF and return next phase
    case 2:

      //MOTOR_SAMPLE_TRANSFER_ROTATE to POSITION_SAMPLE_TRANSFER_ROTATE_SAFE
    case 3:
      Move(MOTOR_SAMPLE_TRANSFER_ROTATE, POSITION_SAMPLE_TRANSFER_ROTATE_SAFE, SPEED_SAMPLE_TRANSFER_ROTATE_MAX, ACCEL_SAMPLE_TRANSFER_ROTATE_MAX);
      Execute();

      //Block until case 3 is done
    case 4:
      if(Done(MOTOR_SAMPLE_TRANSFER_ROTATE))
	{
	  return 5;
	}
      return 4;

      //MOTOR_SAMPLE_TRANSFER_VERTICAL to position POSITION_SAMPLE_TRANSFER_VERTICAL_GET_TUBE
      //MOTOR_SAMPLE_TRANSFER_VIBRATOR to POSITION_SAMPLE_TRANSFER_VIBRATE_UP
    case 5:
      Move(MOTOR_SAMPLE_TRANSFER_VERTICAL, POSITION_SAMPLE_TRANSFER_VERTICAL_GET_TUBE, SPEED_SAMPLE_TRANSFER_VERTICAL_MAX, ACCEL_SAMPLE_TRANSFER_VERTICAL_MAX);
      Move(MOTOR_SAMPLE_TRANSFER_VIBRATOR, POSITION_SAMPLE_TRANSFER_VIBRATOR_UP, SPEED_SAMPLE_TRANSFER_VIBRATOR_MAX, ACCEL_SAMPLE_TRANSFER_VIBRATOR_MAX);
      Execute();

      //Block until cases 5 are done
    case 6:
      if(Done(MOTOR_SAMPLE_TRANSFER_VERTICAL) && Done(MOTOR_SAMPLE_TRANSFER_VIBRATOR))
	{
	  return 7;
	}
      return 6;

      //MOTOR_SAMPLE_TRANSFER_ROTATE to POSITION_SAMPLE_TRANSFER_ROTATE_PICK_UP_TUBE
      //MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER to ON
    case 7:
      Move(MOTOR_SAMPLE_TRANSFER_ROTATE, POSITION_SAMPLE_TRANSFER_ROTATE_PICK_UP_TUBE, SPEED_SAMPLE_TRANSFER_ROTATE_MAX, ACCEL_SAMPLE_TRANSFER_ROTATE_MAX);
      //TODO: Write Gripper ON move
      Execute();

      //Block until case 7 is done
    case 8:
      if(Done(MOTOR_SAMPLE_TRANSFER_ROTATE) && Done(MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER))
	{
	  return 9;
	}
      return 8;

      //MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER to OFF
      //MOTOR_SAMPLE_TRANSFER_VERTICAL to POSITION_SAMPLE_TRANSFER_VERTICAL_SOLID_OVER
    case 9:
      //TODO: Write Gripper OFF move
      Move(MOTOR_SAMPLE_TRANSFER_VERTICAL, POSITION_SAMPLE_TRANSFER_VERTICAL_SOLID_OVER, SPEED_SAMPLE_TRANSFER_VERTICAL_MAX, ACCEL_SAMPLE_TRANSFER_VERTICAL_MAX);
      Execute();

      //Block until case 9 is done
    case 10:
      if(Done(MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER) && Done(MOTOR_SAMPLE_TRANSFER_VERTICAL))
	{
	  return 11;
	}
      return 10;

      //MOTOR_SAMPLE_TRANSFER_ROTATE to POSITION_SAMPLE_TRANSFER_ROTATE_SOLID
    case 11:
      Move(MOTOR_SAMPLE_TRANSFER_ROTATE, POSITION_SAMPLE_TRANSFER_ROTATE_SOLID, SPEED_SAMPLE_TRANSFER_ROTATE_MAX, ACCEL_SAMPLE_TRANSFER_ROTATE_MAX);
      Execute();

      //Block until case 11 done
    case 12:
      if(Done(MOTOR_SAMPLE_TRANSFER_ROTATE))
	{
	  return 13;
	}
      return 12;

      //MOTOR_SAMPLE_TRANSFER_VERTICAL to POSITION_SAMPLE_TRANSFER_VERTICAL_SOLID_INSERT
    case 13:
      Move(MOTOR_SAMPLE_TRANSFER_VERTICAL, POSITION_SAMPLE_TRANSFER_VERTICAL_SOLID_INSERT, SPEED_SAMPLE_TRANSFER_VERTICAL_MAX, ACCEL_SAMPLE_TRANSFER_VERTICAL_MAX);
      Execute();

      //Block until case 13 done
    case 14:
      if(Done(MOTOR_SAMPLE_TRANSFER_VERTICAL))
	{
	  return 15;
	}
      return 14;

      //MOTOR_SAMPLE_TRANSFER_VIBRATION to ON
    case 15:
      //TODO: Write vibration task

      //Block for period VIBRATE_SAMPLE_TIME
    case 16:
      return -1;
    }
  return -1;
}

/******************************************************************************/


//Removes tube from SOLID and discards
int CMarteBot::DiscardTube()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_SAMPLE_TRANSFER_VIBRATE to POSITION_SAMPLE_TRANSFER_VIBRATE_UP
      //MOTOR_SAMPLE_TRANSFER_VERTICAL to POSITION_SAMPLE_TRANSFER_VERTICAL_SOLID_OVER
    case 1:
      Move(MOTOR_SAMPLE_TRANSFER_VIBRATOR, POSITION_SAMPLE_TRANSFER_VIBRATOR_UP, SPEED_SAMPLE_TRANSFER_VIBRATOR_MAX, ACCEL_SAMPLE_TRANSFER_VIBRATOR_MAX);
      Move(MOTOR_SAMPLE_TRANSFER_VERTICAL, POSITION_SAMPLE_TRANSFER_VERTICAL_SOLID_OVER, SPEED_SAMPLE_TRANSFER_VERTICAL_MAX, ACCEL_SAMPLE_TRANSFER_VERTICAL_MAX);
      Execute();

      //Block until case 1 done
    case 2:
      if(Done(MOTOR_SAMPLE_TRANSFER_VIBRATOR) && Done(MOTOR_SAMPLE_TRANSFER_VERTICAL))
	{
	  return 3;
	}
      return 2;

      //MOTOR_SAMPLE_TRANSFER_ROTATE to POSITION_SAMPLE_TRANSFER_ROTATE_TRASH
    case 3:
      Move(MOTOR_SAMPLE_TRANSFER_ROTATE, POSITION_SAMPLE_TRANSFER_ROTATE_TRASH, SPEED_SAMPLE_TRANSFER_ROTATE_MAX, ACCEL_SAMPLE_TRANSFER_ROTATE_MAX);
      Execute();

      //Block until case 3 is done
    case 4:
      if(Done(MOTOR_SAMPLE_TRANSFER_ROTATE))
	{
	  return 5;
	}
      return 4;

      //MOTOR_SAMPLE_TRANSFER_VERTICAL to POSITION_SAMPLE_TRANSFER_VERTICAL_TRASHCAN
    case 5:
      Move(MOTOR_SAMPLE_TRANSFER_VERTICAL, POSITION_SAMPLE_TRANSFER_VERTICAL_TRASHCAN, SPEED_SAMPLE_TRANSFER_VERTICAL_MAX, ACCEL_SAMPLE_TRANSFER_VERTICAL_MAX);
      Execute();

      //Block until case 5 is done
    case 6:
      if(Done(MOTOR_SAMPLE_TRANSFER_VERTICAL))
	{
	  return 7;
	}
      return 6;

      //MOTOR_SAMPLE_TRANSFER_ROTATE to POSITION_SAMPLE_TRANSFER_ROTATE_TRASH_TUBE
      //MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER to ON
    case 7:
      Move(MOTOR_SAMPLE_TRANSFER_ROTATE, POSITION_SAMPLE_TRANSFER_ROTATE_TRASH_TUBE, SPEED_SAMPLE_TRANSFER_ROTATE_MAX, ACCEL_SAMPLE_TRANSFER_ROTATE_MAX);
      //TODO: Write gripper movement code

      //Block until case 7 is done
    case 8:
      if(Done(MOTOR_SAMPLE_TRANSFER_ROTATE) && Done(MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER))
	{
	  return 9;
	}
      return 8;

      //MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER to OFF
      //MOTOR_SAMPLE_TRANSFER_VERTICAL to POSITION_SAMPLE_TRANSFER_VERTICAL_SAFE
    case 9:
      Move(MOTOR_SAMPLE_TRANSFER_VERTICAL, POSITION_SAMPLE_TRANSFER_VERTICAL_TRASHCAN, SPEED_SAMPLE_TRANSFER_VERTICAL_MAX, ACCEL_SAMPLE_TRANSFER_VERTICAL_MAX);

      Execute();
      //TODO: Write gripper movement code

      //Block until case 9 is done
    case 10:
      if(Done(MOTOR_SAMPLE_TRANSFER_VERTICAL) && Done(MOTOR_SAMPLE_TRANSFER_OPEN_GRIPPER))
	{
	  return -1;
	}
      return 10;
    }
  return -1;
}

/******************************************************************************/


//Put Actuators in neutral position
int CMarteBot::ResetActuators()
{

  printf("Resetting Actuators\n");
  //Board 1 reset positions
  if(!m_boardIndex)
    {
      printf("Resetting Actuators for Board 1\n");
      int taskPhase = m_taskList.GetTaskPhase(TASK_RESET_ACTUATORS_1);
      switch(taskPhase)
	{
	  //Get task parameter and jump to proper phase
	case 1:
	  if(m_taskList.GetTaskParameter(TASK_RESET_ACTUATORS_1) == 0) return -1;
	  return m_taskList.GetTaskParameter(TASK_RESET_ACTUATORS_1);

	  //Linear Rail Reset
	case 2:

	  break;
	  
	  //Liner Feeder Reset
	case 3:

	  break;

	  //Face Saw Reset
	case 4:
	  //If we have hit the limit switch, stop the saw and set to zero
	  if(GetDigital(PORT_FACE_SAW_TILT_LIMIT_SWITCH))
	    {
	      Stop(MOTOR_FACE_SAW_TILT);
	      Execute();
	      CAxesOpenQuad::SetPosition(MOTOR_FACE_SAW_TILT, 0);
	      return -1;
	    }
	  //Else, assuming we arn't moving, move back in very small increments
	  if(Done(MOTOR_FACE_SAW_TILT))
	    {
	      Move(MOTOR_FACE_SAW_TILT, GetPosition(MOTOR_FACE_SAW_TILT)-500, SPEED_FACE_SAW_TILT_MAX, ACCEL_FACE_SAW_TILT_MAX);
	      Execute();
	    }
	  return 4;
	  
	  //Subsampler
	case 5: 

	  break;
	  
	  //Transfer Arm
	case 6:
	  
	  break;
	}
    }
  //Board 2 reset positions
  else
    {

      int taskPhase = m_taskList.GetTaskPhase(TASK_RESET_ACTUATORS_2);
      switch(taskPhase)
	{
	  //Get task parameter and jump to proper phase
	case 1:
	  if(m_taskList.GetTaskParameter(TASK_RESET_ACTUATORS_2) == 0) return -1;
	  return m_taskList.GetTaskParameter(TASK_RESET_ACTUATORS_2);

	  //Subsampler Vertical
	case 2:

	  if(GetDigital(PORT_SUBSAMPLE_VERTICAL_LIMIT_HIGH))
	    {
	      Stop(MOTOR_SUBSAMPLE_VERTICAL);
	      Execute();
	      CAxesOpenQuad::SetPosition(MOTOR_SUBSAMPLE_VERTICAL, 0);
	      return 3;
	    }
	  if(Done(MOTOR_SUBSAMPLE_VERTICAL))
	    {
	      Move(MOTOR_SUBSAMPLE_VERTICAL, GetPosition(MOTOR_SUBSAMPLE_VERTICAL) - 10000, SPEED_SUBSAMPLE_VERTICAL_MAX, ACCEL_SUBSAMPLE_VERTICAL_MAX);
	      Execute();
	    }
	  return 2;

	  //Subsampler Turn
	case 3:
	  if(GetDigital(PORT_SUBSAMPLE_ROTATE_INDEX))
	    {
	      Stop(MOTOR_SUBSAMPLE_ROTATE);
	      Execute();
	      CAxesOpenQuad::SetPosition(MOTOR_SUBSAMPLE_ROTATE, 0);
	      return -1;
	    }
	  if(Done(MOTOR_SUBSAMPLE_ROTATE))
	    {
	      Move(MOTOR_SUBSAMPLE_ROTATE, GetPosition(MOTOR_SUBSAMPLE_ROTATE) + 10000, SPEED_SUBSAMPLE_ROTATE_MAX, ACCEL_SUBSAMPLE_ROTATE_MAX);
	      Execute();
	    }
	  return 3;

	  //Transfer Vertical
	case 4:

	  if(GetDigital(PORT_SAMPLE_TRANSFER_VERTICAL_LIMIT_HIGH))
	    {
	      Stop(MOTOR_SAMPLE_TRANSFER_VERTICAL);
	      Execute();
	      CAxesOpenQuad::SetPosition(MOTOR_SAMPLE_TRANSFER_VERTICAL, 0);
	      return 5;
	    }
	  if(Done(MOTOR_SAMPLE_TRANSFER_VERTICAL))
	    {
	      Move(MOTOR_SAMPLE_TRANSFER_VERTICAL, GetPosition(MOTOR_SAMPLE_TRANSFER_VERTICAL) - 10000, SPEED_SAMPLE_TRANSFER_VERTICAL_MAX, ACCEL_SAMPLE_TRANSFER_VERTICAL_MAX);
	      Execute();
	    }
	  return 4;

	  //Transfer Turn
	case 5:
	  if(GetDigital(PORT_SAMPLE_TRANSFER_ROTATE_INDEX))
	    {
	      Stop(MOTOR_SAMPLE_TRANSFER_ROTATE);
	      Execute();
	      CAxesOpenQuad::SetPosition(MOTOR_SAMPLE_TRANSFER_ROTATE, 0);
	      return -1;
	    }
	  if(Done(MOTOR_SAMPLE_TRANSFER_ROTATE))
	    {
	      Move(MOTOR_SAMPLE_TRANSFER_ROTATE, GetPosition(MOTOR_SAMPLE_TRANSFER_ROTATE) + 10000, SPEED_SAMPLE_TRANSFER_ROTATE_MAX, ACCEL_SAMPLE_TRANSFER_ROTATE_MAX);
	      Execute();
	    }
	  return 5;

	}
    }
  return -1;
}

/******************************************************************************/


//Looses core in the clamp
int CMarteBot::ReleaseClamp()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_CORE_CLAMP to position POSITION_CORE_CLAMP_OPEN
    case 1:
      Move(MOTOR_CORE_CLAMP, POSITION_CORE_CLAMP_OPEN, SPEED_CORE_CLAMP_MAX, ACCEL_CORE_CLAMP_MAX);
      Execute();

      //Block until done
    case 2:
      if(Done(MOTOR_CORE_CLAMP))
	{
	  return -1;
	}
      return 2;
    }
  return -1;
}

/******************************************************************************/


//Dumps the sample in the clamp stored in storage cell 9
//(Note that the clamp had to have a release_clamp operation while it was on
//the rail for this to work)
int CMarteBot::EmptyClamp()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_CORE_DUMP to position POSITION_CORE_DUMP_DUMP
    case 1:
      Move(MOTOR_CORE_DUMP, POSITION_CORE_DUMP_DUMP, SPEED_CORE_DUMP_MAX, ACCEL_CORE_DUMP_MAX);
      Execute();

      //Block until case 1 is done
    case 2:
      if(Done(MOTOR_CORE_DUMP))
	{
	  return 3;
	}
      return 2;

      //MOTOR_CORE_DUMP to position POSITION_CORE_DUMP_NORMAL
    case 3:
      Move(MOTOR_CORE_DUMP, POSITION_CORE_DUMP_NORMAL, SPEED_CORE_DUMP_MAX, ACCEL_CORE_DUMP_MAX);
      Execute();

      //Block until case 3 is done
    case 4:
      if(Done(MOTOR_CORE_DUMP))
	{
	  return -1;
	}
      return 4;
    }
  return -1;
}

/******************************************************************************/


//Cuts a subsample
int CMarteBot::CutSubsample()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_SUBSAMPLE_VERTICAL to position POSITION_SUBSAMPLE_VERTICAL_TRANSFER_TURN
    case 1:
      Move(MOTOR_SUBSAMPLE_VERTICAL, POSITION_SUBSAMPLE_VERTICAL_TRANSFER_TURN, SPEED_SUBSAMPLE_VERTICAL_MAX, ACCEL_SUBSAMPLE_VERTICAL_MAX);
      Execute();

      //Block until case 1 is done
    case 2:
      if(Done(MOTOR_SUBSAMPLE_VERTICAL))
	{
	  return 3;
	}
      return 2;

      //MOTOR_SUBSAMPLE_ROTATE to position POSITION_SUBSAMPLE_ROTATE_CUT
      //MOTOR_SUBSAMPLE_SQUEEZE to position POSITION_SUBSAMPLE_SQUEEZE_OPEN
    case 3:
      Move(MOTOR_SUBSAMPLE_ROTATE, POSITION_SUBSAMPLE_ROTATE_CUT, SPEED_SUBSAMPLE_ROTATE_MAX, ACCEL_SUBSAMPLE_ROTATE_MAX);
      Move(MOTOR_SUBSAMPLE_SQUEEZE, POSITION_SUBSAMPLE_SQUEEZE_OPEN, SPEED_SUBSAMPLE_SQUEEZE_MAX, ACCEL_SUBSAMPLE_SQUEEZE_MAX);
      Execute();

      //Block until case 3 is done
    case 4:
      if(Done(MOTOR_SUBSAMPLE_ROTATE) && Done(MOTOR_SUBSAMPLE_SQUEEZE))
	{
	  return 5;
	}
      return 4;

      //MOTOR_SUBSAMPLE_VERTICAL to position POSITION_SUBSAMPLE_VERTICAL_START_CUT
    case 5:
      Move(MOTOR_SUBSAMPLE_VERTICAL, POSITION_SUBSAMPLE_VERTICAL_START_CUT, SPEED_SUBSAMPLE_VERTICAL_MAX, ACCEL_SUBSAMPLE_VERTICAL_MAX);
      Execute();


      //Block until case 5 is done
    case 6:
      if(Done(MOTOR_SUBSAMPLE_VERTICAL))
	{
	  return 7;
	}
      return 6;

      //MOTOR_SUBSAMPLE_SAW_DRIVE @ speed SPEED_SUBSAMPLE_SAW_SAMPLE
      //MOTOR_SUBSAMPLE_VERTICAL @ speed SPEED_SUBSAMPLE_VERITCAL_SAMPLE to position POSITION_SUBSAMPLE_VERTICAL_END_CUT
    case 7:
      //TODO: Write Subsampleing drive task

      //Block until case 7 is done or MOTOR_SUBSAMPLE_SAW_DRIVE @ speed < SPEED_SUBSAMPLE_SAW_SAMPLE_MINIMUM
    case 8:

      //MOTOR_SUBSAMPLE_SAW_DRIVE @ speed 0
    case 9:
      SetPWM(MOTOR_SUBSAMPLE_SAW_DRIVE, 0);
      return -1;
    }
  return -1;
}

/******************************************************************************/

//Crushes a subsample
int CMarteBot::CrushSubsample()
{
  int taskPhase = m_taskList.GetTaskPhase(TASK_ACCEPT_CORE);
  switch(taskPhase)
    {
      //MOTOR_SUBSAMPLE_VERTICAL to position POSITION_SUBSAMPLE_VERTICAL_TRANSFER_TURN
    case 1:
      Move(MOTOR_SUBSAMPLE_VERTICAL, POSITION_SUBSAMPLE_VERTICAL_TRANSFER_TURN, SPEED_SUBSAMPLE_VERTICAL_MAX, ACCEL_SUBSAMPLE_VERTICAL_MAX);
      Execute();

      //Block until case 1 is done
    case 2:
      if(Done(MOTOR_SUBSAMPLE_VERTICAL))
	{
	  return 3;
	}
      return 2;

      //MOTOR_SUBSAMPLE_ROTATE to position POSITION_SUBSAMPLE_ROTATE_CRUSH
    case 3:
      Move(MOTOR_SUBSAMPLE_ROTATE, POSITION_SUBSAMPLE_ROTATE_CRUSH, SPEED_SUBSAMPLE_ROTATE_MAX, ACCEL_SUBSAMPLE_ROTATE_MAX);
      Execute();

      //Block until case 3 is done
    case 4:
      if(Done(MOTOR_SUBSAMPLE_ROTATE))
	{
	  return 5;
	}
      return 4;

      //MOTOR_SUBSAMPLE_SQUEEZE to position POSITION_SUBSAMPLE_SQUEEZE_OPEN
    case 5:
      Move(MOTOR_SUBSAMPLE_SQUEEZE, POSITION_SUBSAMPLE_SQUEEZE_OPEN, SPEED_SUBSAMPLE_SQUEEZE_MAX, ACCEL_SUBSAMPLE_SQUEEZE_MAX);
      Execute();

      //Block until case 5 is done
    case 6:
      if(Done(MOTOR_SUBSAMPLE_SQUEEZE))
	{
	  return -1;
	}
      return 6;
    }
  return -1;
}


	       
	   
