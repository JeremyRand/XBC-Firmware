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

#include "../../libxrc/vision.h"
#include <intcont.h>
#include <textdisp.h>
#include <gba.h>

class CDerivedVision : public CVision
	{
public:
	CDerivedVision(CInterruptCont *pIntCont);

protected:
	virtual void PostRender();
	};


CDerivedVision::CDerivedVision(CInterruptCont *pIntCont) : CVision(pIntCont)
	{
	}

void CDerivedVision::PostRender()
	{
	if (!(GBA_REG_P1&GBA_KEY_A))
		SetRenderMode(RM_RAW);
	else if (!(GBA_REG_P1&GBA_KEY_B))
		SetRenderMode(RM_PROCESSED);
	}

CTextDisp td(TDM_CPORT);

int main(void)
	{
	printf("Vision\n");
	CInterruptCont intCont;
	CDerivedVision vision(&intCont);

	vision.UploadModel(0, 0, 15, 200, 224, 100, 224);
	vision.SetRenderColor(0, DV_BUILD_COLOR(31, 0, 0));
	vision.SetRenderMode(RM_RAW);
	vision.RenderLoop();
	}

