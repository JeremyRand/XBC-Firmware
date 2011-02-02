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

#include "btqueue.h"
#include "../logic/botball1.h"
#include <xport.h>
#include <string.h>
#include <textdisp.h>
#include <axesc.h>
#include "uartqueue.h"

// put your host's BT device address heere
//#define BT_HOST				"000C41E21DCB"
#define BT_HOST				"0010C633CFC8"

CTextDisp td(TDM_LCD);

int main(void)
	{
	int i = 0;
	int result;
	char buf[0x100];
	volatile unsigned long d;
	volatile unsigned short s;

	CInterruptCont intCont;

	// test RS232
#if 0
	printf("RS232 test\n");

	// set mux to RS232 (power-up default)
	UART_SET_RS232();
	printf("serial test\n");
    strcpy(buf, "hello\n");
	CUartQueue serial(&intCont);

	serial.SetBaud(DUC_BAUD_38K);
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	serial.WriteString("01234567890123456789012345678901234567890123456789012345678901234567890123456789");
	while(1)
		{
		if (serial.ReadData(buf, 1, 0))
			serial.WriteData(buf, 1);
		}

#endif

	// test BT
#if 1 
	printf("Bluetooth test\n");

	// set mux to BT
	UART_SET_BT();
	CBluetoothQueue BtQueue(&intCont, UART_BASE, UART_VECTOR);
	result = BtQueue.MasterConnect(BT_HOST, 60000, DBC_BAUD_38K);
	printf("result %d\n", result);

	// test send
#if 0
	while(1)
		{
		BtQueue.WriteString("0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
		}
	// test streaming receive (use serialtest on host side to test)
#else
	while(1)
		{
		buf[0] = '\0';
		result = BtQueue.ReadData(buf, 100);
		buf[10] = '\0';
		printf("%x %x %s %d %d %d\n", XP_REG_INTERRUPT_STAT, XP_REG_INTERRUPT_MASK, buf, BtQueue.ReceiveCount(), BtQueue.ReceiveFifoCount(), i++);
		// wait, let circular queue fill up
		for (d=0; d<40000; d++);
		}
#endif
#endif
	}
