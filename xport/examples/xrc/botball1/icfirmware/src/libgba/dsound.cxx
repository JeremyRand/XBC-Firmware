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

#include "gba.h"
#include "dsound.h"

CDirectSound::CDirectSound(CInterruptCont *pIntCont)
	{
	m_pIntCont = pIntCont;

	// register timer 1 (interrupt bit 4) with interrupt controller 
	m_pIntCont->Register(this, 4);
	m_pIntCont->Unmask(4);
	}

CDirectSound::~CDirectSound()
	{
	m_pIntCont->UnRegister(4);
	}

int CDirectSound::Play(char *data, int size, bool munch)
	{
	
	// Play a mono sound at 8khz in DMA mode Direct Sound
	// uses timer 0 as sampling rate source
	// uses timer 1 to count the samples played in order to stop the sound 

	GBA_REG_SGCNT0_L = 0;
	GBA_REG_SGCNT0_H = 0x0b0F;       // DS A&B + fifo reset + timer0 + max volume to L and R
	GBA_REG_SGCNT1 = 0x0080;       // turn sound chip on
	
	GBA_REG_DMA1SAD = (unsigned long)data;	// dma1 source
	GBA_REG_DMA1DAD = 0x040000a0;     // write to FIFO A address
	GBA_REG_DMA1CNT_H = 0xb600;	    // dma control: DMA enabled+ start on FIFO+32bit+repeat
	
	GBA_REG_TM1D = 0xffff-size;	// 0xffff-the number of samples to play
	GBA_REG_TM1CNT = 0xC4;		    // enable timer1 + irq and cascade from timer 0
	
	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
	GBA_REG_TM0D = 0xF7ff;	    // 8khz playback freq
	GBA_REG_TM0CNT = 0x0080; 	    // enable timer at CPU freq 

	m_playing_munch=munch;
	if(munch) m_munch_data=data;
	if(munch) m_munch_size=size;

	m_playing = true;
	}

void CDirectSound::Stop()
	{
	GBA_REG_TM0CNT = 0;	 // disable timer 0
	GBA_REG_TM1CNT = 0;	 // disable timer 1
	GBA_REG_DMA1CNT_H = 0; // stop DMA
	m_playing = false;	
	}

bool CDirectSound::Playing()
	{
	return m_playing;
	}

void CDirectSound::Interrupt(unsigned char vector)
	{
	// this routine is called when timer 1 overflows...
	if(m_playing_munch) Play(m_munch_data,m_munch_size,true);

	Stop();
	}


