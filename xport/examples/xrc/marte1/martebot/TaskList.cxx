#include "TaskList.h"

CTaskList::CTaskList() : m_taskList(NULL)
{
}

CTaskList::~CTaskList()
{

}

void CTaskList::AddTask(int taskID, int taskParameter)
{

  //Check to see if first node is valid task
  if(!m_taskList)
    {
      m_taskList = new CTaskNode(taskID, taskParameter);
    }
  //Iterate to end and add task
  else
    {
      CTaskNode* taskItr = m_taskList;
      while(taskItr->GetNextNode() != NULL) taskItr = taskItr->GetNextNode();
      CTaskNode* newNode = new CTaskNode(taskID, taskParameter);
      taskItr->SetNextNode(newNode);
    }
}

void CTaskList::RemoveTask(int taskID)
{
  //Check to see if first node is the removed task
  if(m_taskList->GetTaskID() == taskID)
    {
      if(m_taskList->GetNextNode() == NULL) 
	{
	  delete m_taskList; 
	  m_taskList = NULL;
	  return;
	}		
      CTaskNode* oldNode = m_taskList;		
      m_taskList = m_taskList->GetNextNode();
      delete oldNode;
    }
  else
    {
      CTaskNode* taskItr = m_taskList;
      CTaskNode* prevItr;
      while(taskItr->GetTaskID() != taskID)
	{
	  if(taskItr->GetNextNode() == NULL) return;
	  prevItr = taskItr;
	  taskItr = taskItr->GetNextNode();
	}
      prevItr->SetNextNode(taskItr->GetNextNode());
      delete taskItr;
    }
}

bool CTaskList::IsTaskRunning(int taskID)
{
  if(!m_taskList) return false;
  CTaskNode* taskItr = m_taskList;
  while(1)
    {
      if(taskItr->GetTaskID() == taskID)
	{
	  return true;
	}
      if(taskItr->GetNextNode() == NULL) return false;
      taskItr = taskItr->GetNextNode();
    }
  return false;
}

void CTaskList::SetTaskPhase(int taskID, int taskPhase)
{
  if(!m_taskList) return;
  CTaskNode* taskItr = m_taskList;
  printf("Setting task phase\n");
  while(1)
    {
      if(taskItr->GetTaskID() == taskID)
	{
	  printf("Found task, setting\n");
	  taskItr->SetTaskPhase(taskPhase);
	  return;
	}
      if(taskItr->GetNextNode() == NULL) return;
      taskItr = taskItr->GetNextNode();
    }
}

int CTaskList::GetTaskPhase(int taskID)
{
  if(!m_taskList) return -1;
  CTaskNode* taskItr = m_taskList;
  while(1)
    {
      if(taskItr->GetTaskID() == taskID)
	{
	  return taskItr->GetTaskPhase();
	}
      if(taskItr->GetNextNode() == NULL) return -1;
      taskItr = taskItr->GetNextNode();
    }
}

int CTaskList::GetTaskCount()
{
  if(!m_taskList) return 0;
  int taskCount = 0;
  CTaskNode* taskItr = m_taskList;
  while(1)
    {
      ++taskCount;
      if(taskItr->GetNextNode() == NULL) return taskCount;
      taskItr = taskItr->GetNextNode();
    }
}

int CTaskList::GetTaskParameter(int taskID)
{
  if(!m_taskList) return -1;
  CTaskNode* taskItr = m_taskList;
  while(1)
    {
      if(taskItr->GetTaskID() == taskID)
	{
	  return taskItr->GetTaskParameter();
	}
      if(taskItr->GetNextNode() == NULL) return -1;
      taskItr = taskItr->GetNextNode();
    }
}
