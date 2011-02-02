#include "axiscommhost.h"
#include "../axiscomm/axiscommtab.h"

CAxisCommHost::CAxisCommHost(IComm *pComm) : CRpc(pComm)
	{
	}

CAxisCommHost::~CAxisCommHost()
	{
	}

void CAxisCommHost::Move(unsigned char axis,
	int endPosition, int velocity, unsigned int acceleration)
	{
	CRpcArgs args;

	args.Push((char)axis);
	args.Push((long)endPosition);
	args.Push((long)velocity);
	args.Push((long)acceleration);

	Call(DAC_MOVE, &args);
	}

void CAxisCommHost::MoveVelocity(unsigned char axis,
	int velocity, unsigned int acceleration)
	{
	CRpcArgs args;

	args.Push((char)axis);
	args.Push((long)velocity);
	args.Push((long)acceleration);

	Call(DAC_MOVEVELOCITY, &args);
	}

void CAxisCommHost::Execute()
	{
	CRpcArgs args;

	Call(DAC_EXECUTE, &args);
	}

int CAxisCommHost::GetPosition(unsigned char axis)
	{
	CRpcArgs args;
	int pos;

	args.Push((char)axis);
	Call(DAC_GETPOSITION, &args);

	args.Pop((long *)&pos);
	return pos;
	}

bool CAxisCommHost::Done(unsigned char axis)
	{
	CRpcArgs args;
	char done;

	args.Push((char)axis);
	Call(DAC_DONE, &args);
	
	args.Pop((char *)&done);
	return (bool)done;
	}

void CAxisCommHost::Stop(unsigned char axis)
	{
	CRpcArgs args;

	args.Push((char)axis);
	Call(DAC_STOP, &args);
	}

