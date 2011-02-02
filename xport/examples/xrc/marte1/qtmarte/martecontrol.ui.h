/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

void MarteControl::init()
{
	std::vector<PortTest> ports = drillComm.GetSerialPorts();
	for(std::vector<PortTest>::iterator itr = ports.begin(); itr != ports.end(); ++itr)
	{
		SerialPortBox->insertItem((*itr).name);
	}
	MotorInfoGroup->setColumns(1);
	motorInfoArray.resize(10);
	for(int i=0; i < 10; ++i)
	{
		motorInfoArray.insert(i, new MotorInfo(MotorInfoGroup));
		motorInfoArray.at(i)->setMotorLabel(i, "Unknown");
		motorInfoArray.at(i)->show();
		connect(motorInfoArray.at(i), SIGNAL(RunMotorCommand(int)), this, SLOT(SendMotorCommand(int)));
		connect(motorInfoArray.at(i), SIGNAL(HoldMotorCommand(int)), this, SLOT(RunHoldMotor(int)));
		connect(motorInfoArray.at(i), SIGNAL(SetPositionCommand(int)), this, SLOT(SendSetPositionCommand(int)));
	}

	SensorInfoGroup->setColumns(4);
	digitalSensorArray.resize(20);
	analogSensorArray.resize(10);
	for(int i=0; i < 19; ++i)
	{

		//digital sensors
		sensorLabelArray.append(new QLabel(SensorInfoGroup));
		sensorLabelArray.getLast()->setText(QString::number(i) + QString("d."));
		sensorLabelArray.getLast()->show();
		digitalSensorArray.insert(i, new QLabel(SensorInfoGroup));
		digitalSensorArray.at(i)->setText(QString::number(0));
		digitalSensorArray.at(i)->show();

		//analog sensors

		if(i < 8)
		{
			sensorLabelArray.append(new QLabel(SensorInfoGroup));
			sensorLabelArray.getLast()->setText(QString::number(i) + QString("a."));
			sensorLabelArray.getLast()->show();
			analogSensorArray.insert(i, new QLabel(SensorInfoGroup));
			analogSensorArray.at(i)->setText(QString::number(0));
			analogSensorArray.at(i)->show();
		}
		else
		{
			sensorLabelArray.append(new QLabel(SensorInfoGroup));
			sensorLabelArray.getLast()->setText(" ");
			sensorLabelArray.getLast()->show();
			sensorLabelArray.append(new QLabel(SensorInfoGroup));
			sensorLabelArray.getLast()->setText(" ");
			sensorLabelArray.getLast()->show();
		}
	}
	TaskListView->setColumnText(0, "MARTE Task List");
	TaskListView->setColumnWidthMode(0, QListView::Maximum);
	connect(RefreshTimerButton, SIGNAL(clicked()), this, SLOT(StartRefreshTimer()));
	connect(&refreshTimer, SIGNAL(timeout()), this, SLOT(GetStatusPacket()));
	repaint();
}

void MarteControl::destroy()
{
	drillComm.DisconnectFromPort();
}

void MarteControl::selectSerialPort()
{
	if(SerialPortBox->currentText().length() == 0) return;
	bool connectResult = drillComm.ConnectToPort(SerialPortBox->currentText().ascii());
	if(connectResult)
	{
		if(!drillComm.UpdateStatus())
		{
			drillComm.DisconnectFromPort();
			return;
		}
		for(QMap<int, QValueList<QString> >::const_iterator itr = stringTbl.GetTaskBeginIterator(); itr != stringTbl.GetTaskEndIterator(); ++itr)
		{
			if( (itr.key() >= 100 && itr.key() < 200 && drillComm.GetBoardID() == 0) ||
				(itr.key() >= 200 && drillComm.GetBoardID() == 1)
				)
			{
				StartTaskBox->insertItem(QString::number(itr.key()) + QString(": ") + itr.data()[0]);
			}
		}
		RefreshTimerButton->setEnabled(true);
		botStatusButton->setEnabled(true);
		StartTaskBox->setEnabled(true);
		ParameterSpinBox->setEnabled(true);
		StartTaskButton->setEnabled(true);
		StopTaskButton->setEnabled(true);
		StopAllTaskButton->setEnabled(true);
		StopAllButton->setEnabled(true);
		for(int i=0; i < 10; ++i)
		{
			motorInfoArray.at(i)->setMotorLabel(i, stringTbl.GetMotorName(drillComm.GetBoardID(), i));
			motorInfoArray.at(i)->enableGo();
		}
		repaint();
	}
}

void MarteControl::PortDisconnect()
{
	StopRefreshTimer();
	RefreshTimerButton->setEnabled(false);
	botStatusButton->setEnabled(false);
	StartTaskBox->setEnabled(false);
	ParameterSpinBox->setEnabled(false);
	StartTaskButton->setEnabled(false);
	StopTaskButton->setEnabled(false);
	StopAllButton->setEnabled(false);
	StopAllTaskButton->setEnabled(false);
	for(int i=0; i < 10; ++i)
	{
		motorInfoArray.at(i)->disableGo();
	}
}

void MarteControl::GetStatusPacket()
{
	if(!drillComm.UpdateStatus())
	{
		PortDisconnect();
		return;
	}
	//Motors
	for(int m = 0; m < 10; ++m)
	{
		motorInfoArray.at(m)->setMotorCurrentPosition(drillComm.GetMotorCurrentPosition(m));
		motorInfoArray.at(m)->setMotorGoalPosition(drillComm.GetMotorGoalPosition(m));
		motorInfoArray.at(m)->setMotorVelocity(drillComm.GetMotorCurrentVelocity(m));
	}
	//Digitals
	for(int i = 0; i < 19; ++i)
	{
		digitalSensorArray.at(i)->setText(QString::number(drillComm.GetDigitalValue(i)));
	}
	//Analogs
	for(int i = 0; i < 8; ++i)
	{
		analogSensorArray.at(i)->setText(QString::number(drillComm.GetAnalogValue(i)));
	}

	GetTaskStatusPacket();
}

void MarteControl::GetTaskStatusPacket()
{
	QString statusList;
	std::vector<MarteTaskStatus> taskList = drillComm.GetTaskList();
	if(taskList.size() == 0) statusList = "None";
	else
	{
		QMap<int, int> tasksRunningMap;
		for(int i = 0; i < StopTaskBox->count(); ++i)
		{
			tasksRunningMap[StopTaskBox->text(i).section(":", 0, 0).toInt()] = i;
		}
		TaskListView->clear();
		for(std::vector<MarteTaskStatus>::iterator itr = taskList.begin(); itr != taskList.end(); ++itr)
		{
			statusList += QString::number((*itr).GetTaskID()) + QString(":") + QString::number((*itr).GetTaskStatus()) + QString("   ");
			if((*itr).GetTaskID() < 100) continue;

			QListViewItem* parent;
			FancyListViewItem* child;
			QValueList<QString> taskNames = stringTbl.GetTaskPhases((*itr).GetTaskID());
			int i = 0;

			if(tasksRunningMap.contains((*itr).GetTaskID()))
			{
				tasksRunningMap.remove((*itr).GetTaskID());
			}
			else
			{
				StopTaskBox->insertItem(QString::number((*itr).GetTaskID()) + QString(": ") + taskNames[0]);
			}

			for(QValueList<QString>::iterator listItr = taskNames.begin(); listItr != taskNames.end(); ++listItr, ++i)
			{
				if(i == 0)
				{
					parent = new QListViewItem(TaskListView, *listItr);
					parent->setOpen(true);
				}
				else if(i == (*itr).GetTaskStatus())
				{
					child = new FancyListViewItem(parent, QString::number(i) + ": " + *listItr);
					child->setBackground(0, QColor("red"));
				}
				else
				{
					child = new FancyListViewItem(parent, QString::number(i) + ": " + *listItr);
				}
			}
			parent->sort();
		}
		for(QMap<int, int>::const_iterator itr = tasksRunningMap.constBegin(); itr != tasksRunningMap.constEnd(); ++itr)
		{
			StopTaskBox->removeItem(itr.data());
		}
	}
	TasksRunningLabel->setText(statusList);
}

void MarteControl::StartRefreshTimer()
{
	refreshTimer.start(100);
	RefreshTimerButton->setText("Stop Refresh Timer");
	disconnect(RefreshTimerButton, SIGNAL(clicked()), this, SLOT(StartRefreshTimer()));
	connect(RefreshTimerButton, SIGNAL(clicked()), this, SLOT(StopRefreshTimer()));
}

void MarteControl::StopRefreshTimer()
{
	refreshTimer.stop();
	RefreshTimerButton->setText("Start Refresh Timer");
	disconnect(RefreshTimerButton, SIGNAL(clicked()), this, SLOT(StopRefreshTimer()));   
	connect(RefreshTimerButton, SIGNAL(clicked()), this, SLOT(StartRefreshTimer()));
}

void MarteControl::SendStopAll()
{
	if(!drillComm.StartTask(TASK_STOP_ALL, 0))
	{
		PortDisconnect();
		return;
	}    
}

void MarteControl::EmitAddTab()
{
	emit AddTab();
}

void MarteControl::EmitRemoveTab()
{
	emit RemoveTab();
}

void MarteControl::SendStartTaskCommand()
{
	int taskIndex = (StartTaskBox->currentText()).section(":", 0, 0).toInt();
	if(!drillComm.StartTask(taskIndex, ParameterSpinBox->value()))
	{
		PortDisconnect();
		return;
	}
}

void MarteControl::SendStopTaskCommand()
{
	int taskIndex = (StopTaskBox->currentText()).section(":", 0, 0).toInt();
	if(!drillComm.StartTask(TASK_ABORT_TASK, taskIndex))
	{
		PortDisconnect();
		return;
	}
}

void MarteControl::SendSetPositionCommand(int motorNum)
{
	unsigned char motorCommand[9];
	drillComm.ConvertShort(motorCommand, TASK_SET_MOTOR_POSITION);
	motorCommand[2] = motorNum;
	drillComm.ConvertInt(motorCommand+3, motorInfoArray.at(motorNum)->getMotorPosition());
	drillComm.ConvertShort(motorCommand+8, 0);
	if(!drillComm.SendPacket(motorCommand, 9))
	{
		PortDisconnect();
		return;
	}
}


void MarteControl::SendMotorCommand(int motorNum)
{
	unsigned char motorCommand[13];
	drillComm.ConvertShort(motorCommand, TASK_RUN_MOTOR);
	motorCommand[2] = motorNum;
	drillComm.ConvertInt(motorCommand+3, motorInfoArray.at(motorNum)->getMotorVelocity());
	drillComm.ConvertInt(motorCommand+7, motorInfoArray.at(motorNum)->getMotorPosition());
	drillComm.ConvertShort(motorCommand+11, 0);
	if(!drillComm.SendPacket(motorCommand, 13))
	{
		PortDisconnect();
		return;
	}
}

void MarteControl::RunHoldMotor(int motorNum)
{
	if(!drillComm.StartTask(TASK_HOLD_MOTOR_POSITION, motorNum))
	{
		PortDisconnect();
		return;
	}
}
