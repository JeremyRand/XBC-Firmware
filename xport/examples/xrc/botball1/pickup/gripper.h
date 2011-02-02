#ifndef _GRIPPER_H
#define _GRIPPER_H

#include "axesc.h"

class CGripper
	{
public:
	CGripper(CAxesClosed *pac, unsigned char axis);
	void Open();
	bool Close();
	void Release();

private:
	CAxesClosed *m_pac;
	unsigned char m_axis;
	};

#endif

