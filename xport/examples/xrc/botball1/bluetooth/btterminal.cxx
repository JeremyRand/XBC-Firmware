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
#include "menu.h"
#include <textdisp.h>
#include <axesc.h>

// Put your Bluetooth Device Address here.  Use hexadecimal, capital letters, NOT separated by colons
//#define HOST "00A0961F2025"

// Ezekiel
#define HOST				"0010C633CFC8"

// Tissuechan
//#define HOST				"000C7647595E"

// Cody's mac
//#define HOST				"000D930F0FCC"

#define MASTER

CTextDisp td(TDM_LCD);

int main(void)
	{
	int result;
	CMenu menu;
	char c;

	// Instantiate interrupt controller
	CInterruptCont intCont;

	// set mux to BT
	UART_SET_BT();

	// Instantiate Bluetooth Queue class
	CBluetoothQueue BtQueue(&intCont, UART_BASE, UART_VECTOR);

	// Connect
#ifdef MASTER
	printf("Connecting to host %s...", HOST);
	result = BtQueue.MasterConnect(HOST, 60000, DBC_BAUD_38K);
#else // Slave connect
	printf("Waiting for connection...");
	result = BtQueue.SlaveConnect();
#endif
	if (result<0)
		printf("error.\n");
	else
		printf("success.\n");

	// Terminal loop
	while(1)
		{
		if ((c=menu.GetInput()))
			{
			BtQueue.WriteData(&c, 1);
			printf("%c", c);
			}
		if (BtQueue.ReadData(&c, 1, 0))
			{
			printf("%c", c);
			// echo
			BtQueue.WriteData(&c, 1);
			}
		}
	}
