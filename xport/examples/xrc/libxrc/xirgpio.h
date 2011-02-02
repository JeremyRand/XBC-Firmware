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

#ifndef _XIRGPIO_H
#define _XIRGPIO_H

class CXirGpio
	{
public:
	CXirGpio(unsigned long baseAddr=0x9ffd200, unsigned char mode=0x00);
	~CXirGpio();

	void SetMode(unsigned char mode);

	// Xirimity functions
	void SetXirPower(unsigned char index, unsigned char power);
	unsigned short GetXir(unsigned char index);
	void SetXirAverager(unsigned char window);
	void SetXirRingLength(unsigned char ringLength);
	unsigned short GetXirStatus();

	// GPIO functions
	void SetGpioDataDir(unsigned short dataDir);
	void SetGpioData(unsigned short data);
	unsigned short GetGpioData();

private:
	volatile unsigned short *m_regXir;
	volatile unsigned short *m_regXirStatus;
	volatile unsigned short *m_regXirConfig;
	volatile unsigned short *m_regMode;
	volatile unsigned short *m_regGpioDataDir;
	volatile unsigned short *m_regGpioData;

	char m_divisor;
	};

#endif
