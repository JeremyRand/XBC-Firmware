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

#include "../../include/xport.h"
#include "../../include/textdisp.h"
#include "rcservo.h"

CTextDisp td(TDM_LCD_AND_CPORT);

#define RCSERVO_NUM		30
#define RCSERVO_ADDR	0x9ffc400

#define GPIO_NUM		32
#define GPIO_ADDR		0x9ffc600

#define GPIO_REG_NUM	(GPIO_NUM+15)/16
#define GPIO_REG(i)		*((volatile unsigned short *)GPIO_ADDR+i)
#define GPIO_DDR(i)		GPIO_REG(i)
#define GPIO_DATA(i)	GPIO_REG(i+GPIO_REG_NUM)

int main(void)
	{
	// Check to make sure we are using the correct logic configuration
	if (XP_REG_IDENTIFIER!=0x8015)
		{
		td.Printf("Incorrect logic configuration.\n");
		while(1);
		}

#if 1
	volatile unsigned long d;
	CRCServo servo((unsigned char)RCSERVO_NUM, (unsigned short *)RCSERVO_ADDR);

	// set bounds -- this varies from servo to servo
	servo.SetBounds(0, 64, 196);

	td.Printf("Servo demo\n");

	while(1)
		{
		// move maximum counter-clockwise
		servo.SetPosition(0, 0x00);
		td.Printf("Pos: 0x%x\n", servo.GetPosition(0));
		for (d=0; d<1000000; d++);

		// move middle
		servo.SetPosition(0, 0x80);
		td.Printf("Pos: 0x%x\n", servo.GetPosition(0));
		for (d=0; d<1000000; d++);

		// move maximum clockwise
		servo.SetPosition(0, 0xff);
		td.Printf("Pos: 0x%x\n", servo.GetPosition(0));
		for (d=0; d<1000000; d++);
		}
#else
	unsigned short write = 0, read;

	td.Printf("GPIO demo\n");

	// set first 16 GPIO bits to output
	GPIO_DDR(0) = 0xffff; 
	write = 0;
	while(1)
		{
		GPIO_DATA(0) = write;

		// read-back what we just wrote
		read = GPIO_DATA(0);

		if (read!=write)
			{
			td.Printf("ERROR 0x%x 0x%x\n", read, write);
			while(1);
			}
		if (write==0)
			td.Printf(".");
		write++;
		}
#endif
	}
