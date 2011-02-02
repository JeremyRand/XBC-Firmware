// serialtest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>

HANDLE hComm;

void sendbytes(char *data, int len)
	{
	DWORD dwWrite;

	if (!WriteFile(hComm, data, len, &dwWrite, 0)) 
		printf("Error writing\n");
	}

char readbyte()
	{
	char c;
	DWORD dwRead;

	if (!ReadFile(hComm, &c, 1, &dwRead, 0))
		printf("Error reading\n");
	if (dwRead == 1)
		return c;

	return c;
	}


int main(int argc, char* argv[])
	{
	DCB dcb;
	char buf[256];

	hComm = CreateFile("\\\\.\\COM13",  
		GENERIC_READ | GENERIC_WRITE, 
		0, 
		0, 
		OPEN_EXISTING,
		0,
		0);
	if (hComm == INVALID_HANDLE_VALUE)
		{
		printf("Error opening port\n");
		return -1;
		}
	
	ZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength = sizeof(DCB);
	if (!GetCommState(hComm, &dcb))     // get current DCB
		{
		printf("Error getting comm state\n");
		return -1;
		}
	
	// Update DCB rate.
	
	strcpy(buf, "baud=1921600 parity=N data=8 stop=1");
	if (!BuildCommDCB(buf, &dcb)) 
		{   
		// Couldn't build the DCB. Usually a problem
		// with the communications specification string.
		return FALSE;
		}
	// Set new state.
	if (!SetCommState(hComm, &dcb))
		{
		printf("Error setting comm state\n");
		return -1;
		}

#if 1

	char *message = "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
	int i=0;
	while(1)
		{
		sendbytes(message, 100);
		printf("%d\n", i++);
//		if (i==1)
//			break;
		}
	return 0; 
#endif
	}

