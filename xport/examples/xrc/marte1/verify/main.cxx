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




#include "textdisp.h"
#include "uartqueue.h"
#include "intcont.h"
#include "adc.h"
#include "timer.h"
#include "axesclosedquad.h"
#include "../logic/marte1.h"

CTextDisp td(TDM_LCD_AND_CPORT);

int main(void)
	{

	char c;
	unsigned int res;
	int i;
	CInterruptCont intCont;

#if 0
	// A/D converter test

	CAdc adc(ADC_BASE);

	printf("ADC test\n");
	while(1)
		{
		printf("%x %x %x %x %x %x %x %x\n", 
			adc.GetChannel(0), adc.GetChannel(1), adc.GetChannel(2), adc.GetChannel(3),
			adc.GetChannel(4), adc.GetChannel(5), adc.GetChannel(6), adc.GetChannel(7)
			);
		}
#endif
#if 0
	// Quadrature encoder test

	while(1)
		{
		printf("%d %d %d %d %d %d %d %d %d %d\n", 
		QUADRATURE_REG(0), QUADRATURE_REG(1), QUADRATURE_REG(2), QUADRATURE_REG(3), QUADRATURE_REG(4),
		QUADRATURE_REG(5), QUADRATURE_REG(6), QUADRATURE_REG(7), QUADRATURE_REG(8), QUADRATURE_REG(9));
		}
#endif
#if 0
	// PWM test

	volatile int d;

	printf("PWM test\n");

	while(1)
		{
		for (i=0; i<10; i++)
			{	
			printf("forward %d\n", i);
			PWM_REG(i) = 0x80;
			DIR_REG = 0;
			for (d=0; d<800000; d++);
			printf("reverse %d\n", i);
			PWM_REG(i) = 0xc0;
			DIR_REG = 1<<i;
			for (d=0; d<800000; d++);
			PWM_REG(i) = 0x00;
			}
		}

#endif
#if 0
	// GPIO test

	GPIO_DDR(0) = 0x0000; // set D15-D0 as input
	GPIO_DDR(1) = 0x0000; // set D18-D16 as input

	while(1)
		printf("%x %x\n", GPIO_DATA(1), GPIO_DATA(0));
#endif
#if 1
	// PID test

	CAxesClosedQuad ac(&intCont, 400, 10, 10);
	ac.SetPIDGains(500, 0, 0);

	// move all motors back and forth over and over
	while(1)
		{
		for (i=0; i<10; i++)
			{
			ac.Move(i, 15000, 8000, 8000);
			ac.Execute();
			while(!ac.Done(i))
				printf("%d %d\n", i, ac.GetPosition(i));
			
			ac.Move(i, 0, 8000, 8000);
			ac.Execute();
			while(!ac.Done(i))
				printf("%d %d\n", i, ac.GetPosition(i));
			}
		}
#endif
	}
