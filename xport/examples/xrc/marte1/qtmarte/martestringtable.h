#ifndef MARTE_STRING_TABLE_H
#define MARTE_STRING_TABLE_H

#include "qstring.h"
#include "qptrvector.h"
#include "qmap.h"
#include "qdom.h"

class MarteStringTable
{
	QMap<int, QValueList<QString> > m_taskInfoMap;
	QMap<int, QString> m_motorNameMap[2];
public:
	MarteStringTable();
	~MarteStringTable();
	void LoadSettingsFile(const QString& fileName);

	QString GetMotorName(int controllerID, int motorID);
	QMap<int, QValueList<QString> >::const_iterator GetTaskBeginIterator() { return m_taskInfoMap.constBegin(); }
	QMap<int, QValueList<QString> >::const_iterator GetTaskEndIterator() { return m_taskInfoMap.constEnd(); }
	QValueList<QString> GetTaskPhases(int taskID) { return m_taskInfoMap[taskID]; }	
};

#endif