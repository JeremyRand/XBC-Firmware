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
#include "uartserial.h"
#include "intcont.h"
#include "axiscomm.h"
#include "rpcargs.h"

CTextDisp td(TDM_LCD_AND_CPORT);

int main(void)
	{
	CRpcArgs args;
	CInterruptCont intCont;
	CUartSerial serial(&intCont);
	CAxisComm ac(&serial, &intCont, 400, 10);

	serial.SetBaud(DUC_BAUD_38K);

	ac.pAxes->SetPIDGains(500, 0, 0);

	printf("Axiscomm\n");

	while(1)
		ac.Dispatch(&args);
	}
