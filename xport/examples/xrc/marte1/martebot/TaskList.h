#ifndef TASKLIST_H
#define TASKLIST_H

#include "textdisp.h"

#define NULL 0

class CTaskList
{
public:
  class CTaskNode
  {
    int m_taskID;
    int m_taskPhase;
    int m_taskParameter;
    CTaskNode* m_nextNode;
  public:
    CTaskNode(int taskID, int taskParameter = -1) : m_taskID(taskID), m_taskPhase(1), m_taskParameter(taskParameter), m_nextNode(0) {}
    void SetNextNode(CTaskNode* nextNode) { m_nextNode = nextNode; }
    CTaskNode* GetNextNode() { return m_nextNode; }
    int GetTaskID() { return m_taskID; }
    int GetTaskPhase() { return m_taskPhase; }
    int GetTaskParameter() { return m_taskParameter; }
    void SetTaskPhase(int taskPhase) { m_taskPhase = taskPhase; }
  };
private:
    CTaskNode* m_taskList;
public:
  CTaskList();
  ~CTaskList();
  int GetTaskCount();
  void AddTask(int taskID, int taskParameter = -1);
  int GetTaskParameter(int taskID);
  bool IsTaskRunning(int taskID);
  void RemoveTask(int taskID);
  void SetTaskPhase(int taskID, int taskPhase);
  int GetTaskPhase(int taskID);
  CTaskNode* GetListPointer() { return m_taskList; }
};
#endif
