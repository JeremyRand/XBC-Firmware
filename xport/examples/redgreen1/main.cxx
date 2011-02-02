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
#include "../../include/gba.h"

CTextDisp td(TDM_LCD);

#define IO_ADDR		*((volatile unsigned short *)0x9ffc400)

int main(void)
	{
	// Turn off LEDs
	IO_ADDR = 0;
	while(1)
		{
		// Write keypad value into IO address.  Use xor operation to invert logic.
		IO_ADDR = GBA_REG_P1^0x03;

		// For illustration purposes, readback what we just wrote and print results.
		if (IO_ADDR==0x01)
			td.Printf("Red\n");
		else if (IO_ADDR==0x02)
			td.Printf("Green\n");
		else if (IO_ADDR==0x03)
			td.Printf("Red and green\n");
		else
			td.Printf("\n");
		}
	}
