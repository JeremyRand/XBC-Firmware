#include "martestringtable.h"
#include "TaskDefs.h"

MarteStringTable::MarteStringTable()
{
	/* Load all strings statically */
	(m_motorNameMap[0])[0] = "Low Speed Rail";
	(m_motorNameMap[0])[1] = "High Speed Rail";
	(m_motorNameMap[0])[2] = "Liner Feed";
	(m_motorNameMap[0])[3] = "Liner Feeder Solenoid";
	(m_motorNameMap[0])[4] = "Face Saw Drive";
	(m_motorNameMap[0])[5] = "Face Saw Tilt";
	(m_motorNameMap[0])[6] = "Core Clamping";
	(m_motorNameMap[0])[7] = "Cart-Clamp Release";
	(m_motorNameMap[0])[8] = "Core Clamp Storage";
	(m_motorNameMap[0])[9] = "Core Dump";
	(m_motorNameMap[1])[0] = "Sub Sample Turn";
	(m_motorNameMap[1])[1] = "Sub Sample Vertical";
	(m_motorNameMap[1])[2] = "Sub Sample Saw Drive";
	(m_motorNameMap[1])[3] = "Sub Sample Squeeze";
	(m_motorNameMap[1])[4] = "Sample Transfer Turn";
	(m_motorNameMap[1])[5] = "Sample Transfer Vertical";
	(m_motorNameMap[1])[6] = "Sample Transfer Open Gripper";
	(m_motorNameMap[1])[7] = "Sample Transfer Vibrator";
	(m_motorNameMap[1])[8] = "Tube Storage Solenoid";
	(m_motorNameMap[1])[9] = "Crusher";

	(m_taskInfoMap[TASK_RUN_MOTOR]).push_back("Run Motor");
	(m_taskInfoMap[TASK_RUN_MOTOR]).push_back("Running Motor To Position");

	(m_taskInfoMap[TASK_SET_MOTOR_POSITION]).push_back("Set Motor Position");
	(m_taskInfoMap[TASK_SET_MOTOR_POSITION]).push_back("Setting Motor To Position");

	(m_taskInfoMap[TASK_PLACE_LINER]).push_back("Place Liner");
	(m_taskInfoMap[TASK_PLACE_LINER]).push_back("Fire solenoid and turn motor until paper sensor is true");
	(m_taskInfoMap[TASK_PLACE_LINER]).push_back("Wait for paper sensor");
	(m_taskInfoMap[TASK_PLACE_LINER]).push_back("Release solenoid until paper sensor is true");
	(m_taskInfoMap[TASK_PLACE_LINER]).push_back("Wait for paper sensor");
	(m_taskInfoMap[TASK_PLACE_LINER]).push_back("Turn off motor");

	(m_taskInfoMap[TASK_SEND_BOT_STATUS]).push_back("Get Bot Status");
	(m_taskInfoMap[TASK_SEND_BOT_STATUS]).push_back("Getting Bot Status");

	(m_taskInfoMap[TASK_SEND_TASK_STATUS]).push_back("Get Task Status");
	(m_taskInfoMap[TASK_SEND_TASK_STATUS]).push_back("Getting Task Status");

	(m_taskInfoMap[TASK_ACCEPT_CORE]).push_back("Accept Core");
	(m_taskInfoMap[TASK_ACCEPT_CORE]).push_back("Rail-hi-speed to position zero and Core Clamping to position Open");
	(m_taskInfoMap[TASK_ACCEPT_CORE]).push_back("Wait for move");

	(m_taskInfoMap[TASK_TIGHTEN_CLAMP]).push_back("Tighten Clamp");
	(m_taskInfoMap[TASK_TIGHTEN_CLAMP]).push_back("Core Clamping to position Closed");
	(m_taskInfoMap[TASK_TIGHTEN_CLAMP]).push_back("Wait for move or speed < Y");

	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Face Core");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("High Speed Rail to position 0");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Face Saw Tilt to position 0");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Brake High Speed Rail");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Face Saw Drive at speed X & Low-speed rail @ speed Y to position Z");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Wait for move or Face Saw Speed < XX");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Face Saw Tilt to position UP");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_FACE_CORE]).push_back("Face Saw Drive @ speed 0 & release High Speed Rail");

	(m_taskInfoMap[TASK_MOVE_CLAMP]).push_back("Move Clamp");
	(m_taskInfoMap[TASK_MOVE_CLAMP]).push_back("Hi Speed Rail to position X");
	(m_taskInfoMap[TASK_MOVE_CLAMP]).push_back("Wait for move");

	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Store Core");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("High Speed Rail to position Z");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Core Clamp Storage to position N");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("High Speed Rail to position DOCK");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Cart-Clamp Release to position LOOSE");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("High Speed Rail to position Z");
	(m_taskInfoMap[TASK_STORE_CORE]).push_back("Wait for move");

	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Retrieve Core");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("High Speed Rail to position Z");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Core Clamp Storage to position N");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Cart-Clamp Release to position LOOSE");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("High Speed Rail to position DOCK");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Cart-Clamp Release to position TIGHT");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("High Speed Rail to position Z");
	(m_taskInfoMap[TASK_RETRIEVE_CORE]).push_back("Wait for move");

	(m_taskInfoMap[TASK_MOVE_TO_SUBSAMPLE_POSITION]).push_back("Move to Subsample Position");
	(m_taskInfoMap[TASK_MOVE_TO_SUBSAMPLE_POSITION]).push_back("High Speed Rail to position N");
	(m_taskInfoMap[TASK_MOVE_TO_SUBSAMPLE_POSITION]).push_back("Wait for move");

	(m_taskInfoMap[TASK_GRASP_SAMPLE]).push_back("Grasp Sample");
	(m_taskInfoMap[TASK_GRASP_SAMPLE]).push_back("Brake Subsample Saw Drive");
	(m_taskInfoMap[TASK_GRASP_SAMPLE]).push_back("Move Subsample Squeeze to position CLOSED");
	(m_taskInfoMap[TASK_GRASP_SAMPLE]).push_back("Block until finished or speed < XXX");
	(m_taskInfoMap[TASK_GRASP_SAMPLE]).push_back("Stop hold on Subsample Saw Drive");

	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Transfer Sample");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Tube Storage Solenoid ON");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Tube Storage Solenoid OFF");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Sample transfer rotate to SAFE");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Sample Transfer vertical to position GETTUBE and Sample Transfer vibrate to UP");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Sample Transfer rotate to PICKUPTUBE and Sample Transfer open gripper ON");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Sample Transfer open gripper OFF and Sample Transfer vertical to SOLIDOVER");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Sample Transfer rotate to SOLID");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Sample Transfer vertical to SOLIDINSERT");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Sample Transfer vibrate ON");
	(m_taskInfoMap[TASK_TRANSFER_SAMPLE]).push_back("Block for time VIBRATESAMPLE");

	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Discard Tube");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Sample Transfer Vibrate to UP and Sample Transfer vertical to SOLIDOVER");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Sample Transfer rotate to TRASH");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Sample Transfer vertical to TRASHCAN");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Sample Transfer rotate to TRASHTUBE and Sample Transfer open gripper ON");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Sample Transfer open gripper OFF");
	(m_taskInfoMap[TASK_DISCARD_TUBE]).push_back("Sample Transfer vertical to SAFE");

	(m_taskInfoMap[TASK_RELEASE_CLAMP]).push_back("Release Clamp");
	(m_taskInfoMap[TASK_RELEASE_CLAMP]).push_back("Move Core Clamping to position OPEN");
	(m_taskInfoMap[TASK_RELEASE_CLAMP]).push_back("Wait for move");

	(m_taskInfoMap[TASK_EMPTY_CLAMP]).push_back("Empty Clamp");
	(m_taskInfoMap[TASK_EMPTY_CLAMP]).push_back("Move core-dump motor to position DUMP");
	(m_taskInfoMap[TASK_EMPTY_CLAMP]).push_back("Wait for move");
	(m_taskInfoMap[TASK_EMPTY_CLAMP]).push_back("Move core-dump motor to position NORMAL");
	(m_taskInfoMap[TASK_EMPTY_CLAMP]).push_back("Wait for move");

	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Cut Subsample");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Subsample vertical to position TRANSFERTURN");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Subsample rotate to position CUT and Subsample squeeze to position OPEN");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Subsample vertical to position STARTCUT");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Subsample Saw Drive @ speed X and Subsample vertical @ speed Y to position ENDCUT");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Block until done or Subsample saw-speed < XX");
	(m_taskInfoMap[TASK_CUT_SUBSAMPLE]).push_back("Subsample Saw Drive @ speed 0");

	(m_taskInfoMap[TASK_CRUSH_SUBSAMPLE]).push_back("Crush Subsample");
	(m_taskInfoMap[TASK_CRUSH_SUBSAMPLE]).push_back("Subsample vertical to position TRANSFERTURN");
	(m_taskInfoMap[TASK_CRUSH_SUBSAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_CRUSH_SUBSAMPLE]).push_back("Subsample rotate to position CRUSH");
	(m_taskInfoMap[TASK_CRUSH_SUBSAMPLE]).push_back("Wait for move");
	(m_taskInfoMap[TASK_CRUSH_SUBSAMPLE]).push_back("Subsample squeeze to position OPEN");
	(m_taskInfoMap[TASK_CRUSH_SUBSAMPLE]).push_back("Wait for move");

	(m_taskInfoMap[TASK_RESET_ACTUATORS_1]).push_back("Reset Actuators Board 1");
	(m_taskInfoMap[TASK_RESET_ACTUATORS_1]).push_back("Reset Face Saw");

	(m_taskInfoMap[TASK_RESET_ACTUATORS_2]).push_back("Reset Actuators Board 2");
	(m_taskInfoMap[TASK_RESET_ACTUATORS_2]).push_back("Start Reset");
	(m_taskInfoMap[TASK_RESET_ACTUATORS_2]).push_back("Reset Subsample Vertical");
	(m_taskInfoMap[TASK_RESET_ACTUATORS_2]).push_back("Reset Subsample Turn");
	(m_taskInfoMap[TASK_RESET_ACTUATORS_2]).push_back("Reset Transfer Vertical");
	(m_taskInfoMap[TASK_RESET_ACTUATORS_2]).push_back("Reset Transfer Turn");


}

MarteStringTable::~MarteStringTable()
{
}

QString MarteStringTable::GetMotorName(int controllerID, int motorID) 
{
	if(controllerID < 0 || controllerID > 1) return "";
	if(motorID < 0 || motorID > 9) return "";
	return (m_motorNameMap[controllerID])[motorID];
}