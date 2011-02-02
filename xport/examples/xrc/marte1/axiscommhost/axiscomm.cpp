// axiscomm.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include "pcserial.h"
#include "axiscommhost.h"

int main(int argc, char* argv[])
	{

	CPCSerial serial;
	CAxisCommHost acomm(&serial);

	// COM5, 38400 baud
	serial.Open(5, 38400);

	while(1)
		{
		acomm.Move(0, 50000, 8000, 8000); 
		acomm.Execute();
		while(!acomm.Done(0))
			printf("%d\n", acomm.GetPosition(0));
	
		acomm.Move(0, 0, 8000, 8000); 
		acomm.Execute();
		while(!acomm.Done(0))
			printf("%d\n", acomm.GetPosition(0));
		}

	return 0;
	}

