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

#ifndef _BTQUEUE_H
#define _BTQUEUE_H

#include "btcomm.h"
#include "xport.h"
#include "iinterrupt.h"

#define DCQ_SIZE						0x1000
#define DBQ_BT_BASE						0x9ffd000

class CInterruptCont;
class CBluetoothQueue : public CBluetoothComm, public IInterrupt
	{
public:
	CBluetoothQueue(CInterruptCont *pIntCont, unsigned long base=DBQ_BT_BASE,  unsigned char vector=17);
	virtual ~CBluetoothQueue();
	void SetInterruptCont(CInterruptCont *pIntCont);
	void Clear();
	int  ReceiveCount();

	virtual int Reset();
	virtual unsigned int ReadData(char *buf, unsigned int len, unsigned short timeoutMs=500);
	virtual void Interrupt(unsigned char vector);

private:
	void Fill();
	int  Read(char *buf, unsigned int len);

	CInterruptCont *m_pIntCont;
	unsigned char m_vector;
	char m_buf[DCQ_SIZE];
	unsigned int m_indexWrite;
	unsigned int m_indexRead;
	unsigned int m_count;
	};

#endif
