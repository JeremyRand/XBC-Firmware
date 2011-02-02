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

#include <stdlib.h>
#include "recaxesquad.h"

CRecordAxesQuad::CRecordAxesQuad(unsigned long size, unsigned char axes, unsigned char scalePos)
	{
	m_size = size;
	m_axes = axes;
	m_scalePos = scalePos;

	m_readIndex = 0;
	m_writeIndex = 0;
	m_pdata = 0;
	m_state = STATE_STANDBY;
	m_servo = false;
	}

CRecordAxesQuad::~CRecordAxesQuad()
	{
	if (m_pdata)
		free(m_pdata);
	}

short *CRecordAxesQuad::GetRecord()
	{
	short *result;

	if (m_writeIndex>=m_size)
		return NULL;

	result = m_pdata + m_writeIndex;
	m_writeIndex += m_axes;

	return result;
	}

short *CRecordAxesQuad::SetPlay()
	{
	short *result;

	if (m_readIndex>=m_size || m_readIndex>=m_writeIndex)
		return NULL;

	result = m_pdata + m_readIndex;
	m_readIndex += m_axes;

	return result;
	}

void CRecordAxesQuad::Record()
	{
	if (!m_pdata)
		m_pdata = (short *)malloc(m_size*sizeof(short));
	m_writeIndex = 0;
	m_state = STATE_TRANSITION_RECORD;
	}

void CRecordAxesQuad::Play()
	{
	m_readIndex = 0;
	m_state = STATE_TRANSITION_PLAY;
	}

void CRecordAxesQuad::RStop()
	{
	m_servo = false;
	m_state = STATE_STANDBY;
	}

bool CRecordAxesQuad::Playing()
	{
	return (m_state==STATE_TRANSITION_PLAY || m_state==STATE_PLAY);
	}

bool CRecordAxesQuad::Recording()
	{
	return (m_state==STATE_TRANSITION_RECORD || m_state==STATE_RECORD);
	}

void CRecordAxesQuad::RecordUpdate(AxisPosition position[], AxisPosition desiredPosition[])
	{
	unsigned char axis;
	short *data;

	if (m_state==STATE_TRANSITION_PLAY)
		{
		for (axis=0; axis<m_axes; axis++)
			{
			m_recIntegral[axis] = position[axis];
			desiredPosition[axis] = m_recIntegral[axis] << m_scalePos;
			}
		m_servo = true;
		m_state = STATE_PLAY;
		}
	else if (m_state==STATE_PLAY)
		{
		if ((data=SetPlay())==0)
			RStop();
	
		for (axis=0; axis<m_axes; axis++)
			{
			m_recIntegral[axis] += (AxisPosition)data[axis];
			desiredPosition[axis] = m_recIntegral[axis] << m_scalePos;
			}
		}
	else if (m_state==STATE_TRANSITION_RECORD) 
		{
		for (axis=0; axis<m_axes; axis++)
			m_recIntegral[axis] = position[axis];
		m_servo = false;
		m_state = STATE_RECORD;
		}
	else if (m_state==STATE_RECORD) 
		{
		if ((data=GetRecord())==0)
			m_state = STATE_STANDBY;
		else
			{
			for (axis=0; axis<m_axes; axis++)
				{
				data[axis] = (short)(position[axis] - m_recIntegral[axis]);
				m_recIntegral[axis] = position[axis];
				}
			}
		}
	}

bool CRecordAxesQuad::RecordActive()
	{
	return m_state != STATE_STANDBY;
	}

bool CRecordAxesQuad::RecordServo()
	{
	return m_servo;
	}

