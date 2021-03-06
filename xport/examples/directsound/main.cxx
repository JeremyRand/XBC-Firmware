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
#include "../../include/dsound.h"
#include "blue.inl"

CTextDisp td(TDM_LCD_AND_CPORT);

int main(void)
	{
	CInterruptCont intCont;

	CDirectSound ds(&intCont);

	td.Printf("Playing...");
	ds.Play((char *)blue, sizeof(blue));
	while(ds.Playing());
	td.Printf("done\n");
	}
