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
//  the Initial Developer. All Rights Reseimrved.
// 
//  Contributor(s): Rich LeGrand (rich@charmedlabs.com)
// 
//  END LICENSE BLOCK 

#include <gba.h>
#include "tone.h"
//#include <textdisp.h>

typedef unsigned int uint;
typedef unsigned int ushort;

//////////////////////////////////////////////////////////
// Definitions to map gba.h defines to the docs in tone.h
#define SOUNDCNT_L GBA_REG_SGCNT0_L
#define SOUNDCNT_H GBA_REG_SGCNT0_H
#define SOUNDCNT_X GBA_REG_SGCNT1

/////////////////////////////////////////////////////////////////////
// Master controls
void 
CTone::EnableMaster(bool enable)
{
  if(enable) {
    // Turn on sound circuit
    SOUNDCNT_X = 0x80;
    // Set master volume to full
    SOUNDCNT_L |= 0x77;
    // Set output ratio for channels 1-4 to full
    SOUNDCNT_H = 2;
  }
  else {
    // Turn off sound circuit
    SOUNDCNT_X = 0x00;
  }    
}

////////////////////////////////////////////////////////////////
// CTone class
CTone::CTone() 
{
  // Default to 500 Hz
  m_freq = 0;
  m_enabled=false;
}

CTone::~CTone() 
{
}

void
CTone::SetFreq(int freq)
{
  // Clip the frequency if necessary to keep it in bounds
  if(freq<64) {
    freq=64;
  }
  else if(freq > 131072) {
    freq = 131072;
  }

  // Store the frequency in m_freq
  m_freq=freq;

  // If enabled, set the register to the appropriate value
  if(m_enabled) {
    _SetFreqReg();
  }
}

void 
CTone::SetEnable(bool enable)
{
  bool was_enabled = m_enabled;

  // Call internal SetEnable
  _SetEnableReg(enable);

  // If enabling, set frequency register to last requested value
  if(enable && m_freq != 0) {
    _SetFreqReg();// _SetFreqReg(!was_enabled);
  }
}

// Internal function to set the frequency register
void 
CTone::_SetFreqReg()
//CTone::_SetFreqReg(bool restart)
{
  // Calculate the value to put in the _X register based 
  // on the formula F(Hz)=131072/(2048-register value). 
  // The minimum frequency is 64Hz and the maximum is 131Khz.
  ushort reg = (2048 - (131072/m_freq)) & 0x7ff;
  bool restart = true;

  //printf("Setting freq field to 0x%x, addr 0x%x\n", reg, &(GBA_REG_SG11));
  if(restart) {
    reg |= 0x8000;
  }

  GBA_REG_SG11 = reg;
}

// Internal function to set/clear the enable register
void 
CTone::_SetEnableReg(bool enable)
{
  // Channel enable bits are bits 8-11 and 12-15 of SOUNDCNT_L
  // Set mask bits 8 and 12, which control Sound 1
  ushort mask = 0x1100;
  if(enable) {
    // Make sure the master enable is on
    EnableMaster(true);
    // Set the enable for this channel
    SOUNDCNT_L |= mask;
    // Set the sound 1 L register to sweep shifts and sweep
    // time to zero and increase/decrease bit to 1 as recommended at
    // http://belogic.com/gba
    GBA_REG_SG10_L = 0x08;

    // Set sound length to max, pattern to 50%, and volume to full
    GBA_REG_SG10_H= 0xf080;
  }
  else {
    SOUNDCNT_L &= ~mask;
  }
  m_enabled = enable;
}

// int CTone::Play(char *data, int size)
// 	{
// 	// Play a mono sound at 8khz in DMA mode Direct Sound
// 	// uses timer 0 as sampling rate source
// 	// uses timer 1 to count the samples played in order to stop the sound 

// 	GBA_REG_SGCNT0_L = 0;
// 	GBA_REG_SGCNT0_H = 0x0b0F;       // DS A&B + fifo reset + timer0 + max volume to L and R
// 	GBA_REG_SGCNT1 = 0x0080;       // turn sound chip on
	
// 	GBA_REG_DMA1SAD = (unsigned long)data;	// dma1 source
// 	GBA_REG_DMA1DAD = 0x040000a0;     // write to FIFO A address
// 	GBA_REG_DMA1CNT_H = 0xb600;	    // dma control: DMA enabled+ start on FIFO+32bit+repeat
	
// 	GBA_REG_TM1D = 0xffff-size;	// 0xffff-the number of samples to play
// 	GBA_REG_TM1CNT = 0xC4;		    // enable timer1 + irq and cascade from timer 0
	
// 	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
// 	GBA_REG_TM0D = 0xF7ff;	    // 8khz playback freq
// 	GBA_REG_TM0CNT = 0x0080; 	    // enable timer at CPU freq 

// 	m_playing = true;
// 	}

// void CTone::Stop()
// 	{
// 	GBA_REG_TM0CNT = 0;	 // disable timer 0
// 	GBA_REG_TM1CNT = 0;	 // disable timer 1
// 	GBA_REG_DMA1CNT_H = 0; // stop DMA
// 	m_playing = false;	
// 	}

// bool CTone::Playing()
// 	{
// 	return m_playing;
// 	}

// void CTone::Interrupt(unsigned char vector)
// 	{
// 	// this routine is called when timer 1 overflows...
// 	Stop();
// 	}


