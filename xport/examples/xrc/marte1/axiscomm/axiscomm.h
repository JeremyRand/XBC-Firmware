#ifndef _AXISCOMM_H
#define _AXISCOMM_H

#include "../../librpc/rpc.h"
#include "../libmarte/axesclosedquad.h"

class CAxisComm : public CRpc
	{
public:
	CAxisComm(IComm *pComm, CInterruptCont *pIntCont, unsigned short freq, unsigned char axes);
	virtual ~CAxisComm();

	static void Move(CRpcArgs *args);
	static void MoveVelocity(CRpcArgs *args);
	static void Execute(CRpcArgs *args);
	static void GetPosition(CRpcArgs *args);
	static void Done(CRpcArgs *args);
	static void Stop(CRpcArgs *args);

	static CAxesClosedQuad *pAxes;
	};

#endif
