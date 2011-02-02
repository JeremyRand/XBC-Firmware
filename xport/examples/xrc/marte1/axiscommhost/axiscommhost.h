#ifndef _AXISCOMMHOST_H
#define _AXISCOMMHOST_H

#include "../../librpc/rpc.h"

class CAxisCommHost : public CRpc
	{
public:
	CAxisCommHost(IComm *pComm);
	virtual ~CAxisCommHost();

	void Move(unsigned char axis,
		int endPosition, int velocity, unsigned int acceleration);
	void MoveVelocity(unsigned char axis,
		int velocity, unsigned int acceleration);
	void Execute();
	int GetPosition(unsigned char axis);
	bool Done(unsigned char axis);
	void Stop(unsigned char axis);
	
	};

#endif
