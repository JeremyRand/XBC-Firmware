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

#ifndef _RECAXES_H
#define _RECAXES_H

#include "axesopenquad.h"

enum ERBState
	{
	STATE_STANDBY,
	STATE_TRANSITION_RECORD,
	STATE_RECORD,
	STATE_TRANSITION_PLAY,
	STATE_PLAY
	};

class CRecordAxesQuad 
	{
public:
	CRecordAxesQuad(unsigned long size, unsigned char axes, unsigned char scalePos);
	~CRecordAxesQuad();

	void Record();
	void Play();
	void RStop();
	bool Playing();
	bool Recording();

protected:
	bool RecordActive();
	bool RecordServo();
	void RecordUpdate(AxisPosition position[], AxisPosition desiredPosition[]);
	short *SetPlay();
	short *GetRecord();

	AxisPosition m_recIntegral[DAO_MAX_AXES];
	ERBState m_state;
	short *m_pdata;
	unsigned long m_readIndex;
	unsigned long m_writeIndex;
	unsigned long m_size;
	unsigned char m_axes;
	unsigned char m_scalePos;
	bool m_servo;
	};


#endif
