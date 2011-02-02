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

#ifndef _XPORT_H
#define _XPORT_H

// Cport
#define XP_REG_CPORT_STAT		(*(volatile unsigned short *)0x9ffc200)
#define XP_REG_CPORT_DATA		(*(volatile unsigned short *)0x9ffc202)

// Interrupt controller
#define XP_REG_INTERRUPT_STAT	(*(volatile unsigned short *)0x9ffc242)
#define XP_REG_INTERRUPT_MASK	(*(volatile unsigned short *)0x9ffc240)

// LEDs
#define XP_REG_LED 				(*(volatile unsigned short *)0x9ffc220)
#define XP_LED_GREEN			0x01
#define XP_LED_RED				0x02

// Bitstream identifier
#define XP_REG_IDENTIFIER		(*(volatile unsigned short *)0x9ffc3e0)

// start of block RAM 
#define XP_BRAM_START			0x9ffc000

#endif
