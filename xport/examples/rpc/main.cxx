//  BEGIN LICENSE BLOCK
// 
//  Version: MPL 1.1
// 
//  The contents of this file are subject to the Mozilla Public License Version
//  1.1 (the "License"); you may not use this file except in compliance with
//  the License. You may obtain a copy of the License at
//  http://www.mozilla.org/MPL/
// 
//  Software distributed under the License is distributed on an "AS IS" basis,
//  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
//  for the specific language governing rights and limitations under the
//  License.
// 
//  The Original Code is The Xport Software Distribution.
// 
//  The Initial Developer of the Original Code is Charmed Labs LLC.
//  Portions created by the Initial Developer are Copyright (C) 2003
//  the Initial Developer. All Rights Reserved.
// 
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
// 
//  END LICENSE BLOCK 

#include "../../include/textdisp.h"
#include "../../include/rpc.h"

#define FILENAME   "rpctest.bin"

int main(void)
	{
	CRPCtarget rpc(RPC_LCD_AND_CPORT);
	unsigned char fd;

	rpc.Printf("RPC test\n");

	fd = rpc.Fopen(FILENAME, "wb");
	if (fd)
		{
		rpc.Printf("Opened " FILENAME ".  Writing... (fd=%d)\n", fd);

		// Dump 128K of flash contents.  Normally Fwrite would be used to dump
		// data from a camera (for example) to a file on the host PC for later analysis.
		rpc.Fwrite((unsigned char *)0x8000000, 0x20000, fd);
		rpc.Fclose(fd);

		rpc.Printf("Done.\n");
		rpc.Printf("You can now look in the local directory for the file \"" FILENAME "\"\n");
		}
	else // fd==0 implies error
		{
		rpc.Printf("Could not open file " FILENAME "\n");
		rpc.Printf("Try running \"xpcomm -rpc\"\n");
		}
	}