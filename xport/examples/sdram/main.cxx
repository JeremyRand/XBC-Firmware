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
#include "../../include/xport.h"
#include "../../include/gba.h"

CTextDisp td(TDM_LCD_AND_CPORT);

int main(void)
	{
	unsigned short *addr;
	unsigned short read, write;

	// Check to make sure we are using the correct logic configuration
	if (XP_REG_IDENTIFIER!=0x8013)
		{
		td.Printf("Incorrect logic configuration.\n");
		while(1);
		}

	td.Printf("SDRAM test\n");
	td.Printf("Filling memory");
	write = 0;
    for (addr=(unsigned short *)0x9000000; addr<(unsigned short *)0x9ffc000; addr++)
		{
		*addr = write;
		// ensure data is nonperiodic
		if (write>=0xf051)
			write = 0;
		if (write==0)
			td.Printf(".");
		write++;
		}
	td.Printf("done\n\n");

	while(1)
		{
		td.Printf("Testing memory");
		write = 0;
        for (addr=(unsigned short *)0x9000000; addr<(unsigned short *)0x9ffc000; addr++)
			{
			read = *addr;
			if (read!=write)
				{
				td.Printf("error %x %x %x\n", addr, read, write);
				while(1);
				}
			if (write>=0xf051)
				write = 0;
			if (write==0)
				td.Printf(".");
			write++;
			}
		td.Printf("success\n\n");
		}
	}
