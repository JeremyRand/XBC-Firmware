// serialtest.cpp : Defines the entry point for the console application.
//

#include "pcserial.h"
#include "flashxferhost.h"

int main(int argc, char* argv[])
	{
	if (argc<2)
		{
		printf("usage: upload <filename> <portname [optional]>\n");
		return -1;
		}

	CPCSerial serial;
	CFlashXferHost xfer(&serial);
	unsigned short timeout;
	bool bitstream = false;
	char *filename;
	int flen, res;

	filename = argv[1];
	flen = strlen(filename);

	// look for .bit at end of file for bitstream
	if (flen>4 && filename[flen-1]=='t' && filename[flen-2]=='i' && filename[flen-3]=='b' && filename[flen-4]=='.')
		bitstream = true;

	// COM5, 38400 baud
	if(argc > 2)
	{
		serial.Open(argv[2], 38400);
	}
	else
	{
		serial.Open("COM5", 38400);
	}

	printf("Waiting for Game Boy to boot...");
	fflush(stdout);
	// save timeout
	timeout = xfer.GetTimeout();
	// set timeout to low value
	xfer.SetTimeout(200);
	// wait for gameboy to power up
	if (bitstream)
		while(xfer.SetMode(EXM_PRIVILEGED)<0);
	else
		while(xfer.SetMode(EXM_NORMAL)<0);
	xfer.SetTimeout(timeout);
	printf("done\n");

	// upload
	printf("Uploading...\n");
	fflush(stdout);
	if (bitstream)
		res = xfer.WriteBitstream(filename);
	else
		res = xfer.Write(0x0, filename);

	if (res<0)
		{
		printf("Error\n");
		return -1;
		}

	// if program, execute
	printf("Executing Program...\n");
	fflush(stdout);
	if (!bitstream)
		xfer.Execute(0x8020000);

	printf("Done\n");
	fflush(stdout);

	return 0;
	}

