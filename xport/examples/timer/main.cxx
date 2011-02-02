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

extern "C" 
	{
	void _gba_interrupt(void);
	}

CTextDisp td(TDM_LCD_AND_CPORT);

volatile unsigned long count = 0;  // important to declare volatile

// interrupt service routine
void _gba_interrupt(void)
	{
	if (GBA_REG_IF & GBA_INT_TIMER0)
		{
		GBA_REG_IF = GBA_INT_TIMER0;  // reset interrupt flag
		XP_REG_LED = count&1;  // LED feedback
		count++;
		}

	// check other flags if necessary

	// ...
	}

int main(void)
	{
	int i = 0;
	// set timer registers
	GBA_REG_TM0D = 0x8000;  
	GBA_REG_TM0CNT = 0x00c1;  // count down, 3.814us prescaling, enable timer interrupt  

	// enable interrupts
	GBA_REG_IF = 0xffff;  // clear all flags
	GBA_REG_IE = GBA_INT_TIMER0;  // enable timer interrupt
	GBA_REG_IME = 1;  // set master interrupt enable

	while(1)
		td.Printf("%d %d\n", i++, count);

	}
