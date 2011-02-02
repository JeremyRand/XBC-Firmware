#include "textdisp.h"
#include "axiscomm.h"
#include "axiscommtab.h"

//#define DEBUG

CAxesClosedQuad *CAxisComm::pAxes = 0;
 
CAxisComm::CAxisComm(IComm *pComm, CInterruptCont *pIntCont, unsigned short freq, unsigned char axes) :
	CRpc(pComm)
	{
	pAxes = new CAxesClosedQuad(pIntCont, freq, axes, axes);

	Register(DAC_MOVE, Move);
	Register(DAC_MOVEVELOCITY, MoveVelocity);
	Register(DAC_EXECUTE, Execute);
	Register(DAC_GETPOSITION, GetPosition);
	Register(DAC_DONE, Done);
	Register(DAC_STOP, Stop);
	}

CAxisComm::~CAxisComm()
	{
	}

void CAxisComm::Move(CRpcArgs *args)
	{
	unsigned char axis;
	int endPosition;
	int velocity;
	int acceleration;

	args->Pop((char *)&axis);
	args->Pop((long *)&endPosition);
	args->Pop((long *)&velocity);
	args->Pop((long *)&acceleration);

#ifdef DEBUG
	printf("Move %d %d %d %d\n", axis, endPosition, velocity, acceleration);
#endif
	pAxes->Move(axis, endPosition, velocity, acceleration);
	}

void CAxisComm::MoveVelocity(CRpcArgs *args)
	{
	unsigned char axis;
	int velocity;
	int acceleration;

	args->Pop((char *)&axis);
	args->Pop((long *)&velocity);
	args->Pop((long *)&acceleration);

#ifdef DEBUG
	printf("MoveVelocity %d %d %d\n", axis, velocity, acceleration);
#endif
	pAxes->MoveVelocity(axis, velocity, acceleration);
	}

void CAxisComm::Execute(CRpcArgs *args)
	{
#ifdef DEBUG
	printf("Execute\n");
#endif
	pAxes->Execute();
	}

void CAxisComm::GetPosition(CRpcArgs *args)
	{
	unsigned char axis;
	int pos;

	args->Pop((char *)&axis);

#ifdef DEBUG
	printf("GetPosition %d\n", axis);
#endif

	pos = pAxes->GetPosition(axis);

	args->Reset();
	args->Push((long)pos);
	}

void CAxisComm::Done(CRpcArgs *args)
	{
	unsigned char axis;
	bool done;

	args->Pop((char *)&axis);

#ifdef DEBUG
	printf("Done %d\n", axis);
#endif

	done = pAxes->Done(axis);

	args->Reset();
	args->Push((char)done);
	}

void CAxisComm::Stop(CRpcArgs *args)
	{
	unsigned char axis;

	args->Pop((char *)&axis);

#ifdef DEBUG
	printf("Stop %d\n", axis);
#endif
	pAxes->Stop(axis);
	}

