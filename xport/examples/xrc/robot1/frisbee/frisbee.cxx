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

#include <humrrobot1.h>
#include <textdisp.h>

CTextDisp td(TDM_LCD);

int main(void)
	{
	CHumrRobot1 hr(64000, 28300);

	hr.SetFrisbee(true);
	hr.Move(ROTATE_AXIS, 6283*5, 800, 800);
	hr.Move(X_AXIS, 10000, 150, 150);
	hr.Execute();

	while(1)
		printf("%d %d %d\n", hr.GetPosition(X_AXIS), hr.GetPosition(Y_AXIS), hr.GetPosition(ROTATE_AXIS));
	}
