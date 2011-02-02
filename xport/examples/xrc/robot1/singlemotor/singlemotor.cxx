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

#include <textdisp.h>
#include <axesc.h>

CTextDisp td(TDM_LCD);

int main(void)
	{
	CInterruptCont intCont;

	// create closed-loop motor controller, pass in interrupt controller
	CAxesClosed ac(&intCont);

	// set gains
	ac.SetPIDGains(500, 0, 650);

	// Move motor back and forth (wee!)
	while(1)
		{
		// move to position 50000 
		ac.Move(0, 50000, 10000, 10000);
		ac.Execute();
		while(!ac.Done(0))
			printf("%d\n", ac.GetPosition(0));

		// move back to position 0
		ac.Move(0, 0, -10000, 10000);
		ac.Execute();
		while(!ac.Done(0))
			printf("%d\n", ac.GetPosition(0));
		}
	}
