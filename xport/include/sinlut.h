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

#ifndef _SINLUT_H
#define SINLUT_H

#define DSL_PERIOD			6283 // full period in milliradians
//#define DSL_QUARTER_PERIOD  ((DSL_PERIOD + 2)/4) 
#define DSL_QUARTER_PERIOD  ((DSL_PERIOD + 2)>>2) // GBA doesn't have hardware divide; bitshift is WAY faster
#define DSL_SCALE           10 // results are scaled up by 2^10 = 1024

int SinLut(int angle); // angle in milliradians
int CosLut(int angle); // angle in milliradians

#endif
