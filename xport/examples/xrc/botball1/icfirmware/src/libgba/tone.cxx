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

#include "gba.h"
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
    //SOUNDCNT_H = 2;
	SOUNDCNT_H |= 2; // this should fix the .wavs being muted when a tone plays
  }
  else {
    // Turn off sound circuit
    SOUNDCNT_X = 0x00;
  }    
}

////////////////////////////////////////////////////////////////
// CTone class
CTone::CTone(CInterruptCont *pIntCont) 
{
  // Default to 500 Hz
  m_freq = 0;
  m_enabled=false;

	// DSound stuff
	
	m_playing_a = false;
	m_playing_b = false;
	
	m_pIntCont = pIntCont;
	m_sound_padding = 100000;

	SetVolumeA(1);
	SetVolumeB(1);

	//m_playlist_last_index = -1;
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
//    freq = 131072;
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

 int CTone::Play(char *data, int size, bool munchbool, bool fastbool, int loops, bool in_middle)
 	{

	// register timer 1 (interrupt bit 4) with interrupt controller 
	m_pIntCont->Register(this, 4);
	m_pIntCont->Unmask(4);

 	// Play a mono sound at 8khz in DMA mode Direct Sound
 	// uses timer 0 as sampling rate source
 	// uses timer 1 to count the samples played in order to stop the sound 

 	//GBA_REG_SGCNT0_L = 0;
	// removed by Jeremy, 2007 10 08; it looks like this causes Channels 1-4 to be muted when a .wav starts.
	
 	GBA_REG_SGCNT0_H |= 0x0B02;       // DS A&B + fifo reset + timer0 + max volume to L and R; was 0b0f; changed based on gbatek

 	GBA_REG_SGCNT1 = 0x0080;       // turn sound chip on
	
 	GBA_REG_DMA1SAD = (unsigned long)data;	// dma1 source
 	GBA_REG_DMA1DAD = 0x040000a0;     // write to FIFO A address
 	GBA_REG_DMA1CNT_H = 0xb600;	    // dma control: DMA enabled+ start on FIFO+32bit+repeat

	if(size > 0xffff) // allows playing long sounds
	{
		GBA_REG_TM1D = 0x0;
		m_remaining_addr = data + 0x10000;
		m_remaining_samples = size - 0x10000;
		m_remaining_fast = fastbool;
	}
	else
	{
		GBA_REG_TM1D = 0xffff-size;	// 0xffff-the number of samples to play
		m_remaining_samples = 0;
	}
	
 	GBA_REG_TM1CNT = 0xC4;		    // enable timer1 + irq and cascade from timer 0
	
	if(! m_playing_b)
	{
		//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
		if(!fastbool) GBA_REG_TM0D = 0xF7ff;	// 8khz playback freq
		else GBA_REG_TM0D = 0xFDff;				// 32khz playback
		GBA_REG_TM0CNT = 0x0080; 	    // enable timer at CPU freq 
	}

	if(! in_middle) // allows long sounds to loop
	{
		m_playing_munch=munchbool;
		
		if(munchbool) // moved inside to optimize
		{
			
			m_munch_data=data;
			m_munch_size=size;
			m_munch_fast=fastbool;
			m_munch_loops = loops;
		}
	}

 	m_playing_a = true;
 	}

void CTone::Stop()
{
	if(! m_playing_b)
		GBA_REG_TM0CNT = 0;	 // disable timer 0
	
 	GBA_REG_TM1CNT = 0;	 // disable timer 1
 	GBA_REG_DMA1CNT_H = 0; // stop DMA
 	m_playing_a = false;	

	m_pIntCont->UnRegister(4);
	m_pIntCont->Mask(4); // trying this to deal with one sound end killing the other sound
}

 bool CTone::Playing()
 	{
 	return m_playing_a;
 	}

void CTone::SetVolumeA(short volume)
{
	if(volume > 0)
	{
		GBA_REG_SGCNT0_H |= 0x4;
	}
	else
	{
		GBA_REG_SGCNT0_H &= ~ 0x4;
	}
}

int CTone::PlayB(char *data, int size, bool munchbool, bool fastbool, int loops, bool in_middle)
 	{

	// register timer 3 (interrupt bit 6) with interrupt controller 
	m_pIntCont->Register(this, 6);
	m_pIntCont->Unmask(6);

 	// Play a mono sound at 8khz in DMA mode Direct Sound
 	// uses timer 0 as sampling rate source
	// uses timer 2 as mirror of timer 0
 	// uses timer 3 to count the samples played in order to stop the sound

 	//GBA_REG_SGCNT0_L = 0; 
	// removed by Jeremy, 2007 10 08; it looks like this causes Channels 1-4 to be muted when a .wav starts.
	
 	GBA_REG_SGCNT0_H |= 0xB002;       // DS A&B + fifo reset + timer0 + max volume to L and R; was 0b0f; changed based on gbatek
 	GBA_REG_SGCNT1 = 0x0080;       // turn sound chip on
	
 	GBA_REG_DMA2SAD = (unsigned long)data;	// dma2 source
 	GBA_REG_DMA2DAD = 0x040000a4;     // write to FIFO B address
 	GBA_REG_DMA2CNT_H = 0xb600;	    // dma control: DMA enabled+ start on FIFO+32bit+repeat
	
	
	if(size > 0xffff) // allows playing long sounds
	{
		GBA_REG_TM3D = 0x0;
		m_remaining_addr_b = data + 0x10000;
		m_remaining_samples_b = size - 0x10000;
		m_remaining_fast_b = fastbool;
	}
	else
	{
		GBA_REG_TM3D = 0xffff-size;	// 0xffff-the number of samples to play
		m_remaining_samples_b = 0;
	}
	
 	GBA_REG_TM3CNT = 0xC4;		    // enable timer3 + irq and cascade from timer 2
	
	if(!m_playing_a)
	{
		//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
		if(!fastbool) GBA_REG_TM0D = 0xF7ff;	// 8khz playback freq
		else GBA_REG_TM0D = 0xFDff;				// 32khz playback
		GBA_REG_TM0CNT = 0x0080; 	    // enable timer at CPU freq 
		
		if(!fastbool) GBA_REG_TM2D = 0xF7ff;	// 8khz playback freq
		else GBA_REG_TM2D = 0xFDff;				// 32khz playback
		GBA_REG_TM2CNT = 0x0080; 	    // enable timer at CPU freq 
	}
	else
	{
		GBA_REG_TM2D = GBA_REG_TM0D; // mirror Timer 0
		GBA_REG_TM2CNT = 0x0080; // enable timer at CPU freq
	}

	if(! in_middle) // allows long sounds to loop
	{
		m_playing_munch_b=munchbool;
		
		if(munchbool) // moved inside to optimize
		{
			m_munch_data_b=data;
			m_munch_size_b=size;
			m_munch_fast_b=fastbool;
			m_munch_loops_b = loops;
		}
	}
	
 	m_playing_b = true;
 	}

void CTone::StopB()
{
	// should work
	
	GBA_REG_TM2CNT = 0;	 // disable timer 2
 	GBA_REG_TM3CNT = 0;	 // disable timer 3
	GBA_REG_DMA2CNT_H = 0; // stop DMA
	m_playing_b = false;	

	m_pIntCont->UnRegister(6);
	m_pIntCont->Mask(6); // trying this to deal with one sound end killing the other sound
	
	if(!m_playing_a)
	{
		GBA_REG_TM0CNT = 0;	 // disable timer 0
	}
}

bool CTone::PlayingB()
{
	// should work
 	return m_playing_b;
}

void CTone::SetVolumeB(short volume)
{
	if(volume > 0)
	{
		GBA_REG_SGCNT0_H |= 0x8;
	}
	else
	{
		GBA_REG_SGCNT0_H &= ~ 0x8;
	}
}

/*

int CTone::Play02(char *data, int size, bool munchbool, bool fastbool)
{

	// checked with GBATEK; not tested

	// register timer 2 (interrupt bit 5) with interrupt controller 
	m_pIntCont->Register(this, 5); // changed
	m_pIntCont->Unmask(5); // changed

 	// Play a mono sound at 8khz in DMA mode Direct Sound
 	// uses timer 0 as sampling rate source
 	// uses timer 2 to count the samples played in order to stop the sound 

 	GBA_REG_SGCNT0_L = 0;
 	//GBA_REG_SGCNT0_H = 0x0b0F;       // DS A&B + fifo reset + timer0 + max volume to L and R
	// was    0000101100001111
	// is now 0000101100001110
	GBA_REG_SGCNT0_H |= 0x0B0E;       // Only touches DS A
	
 	GBA_REG_SGCNT1 = 0x0080;       // turn sound chip on
	
 	GBA_REG_DMA1SAD = (unsigned long)data;	// dma1 source
 	GBA_REG_DMA1DAD = 0x040000a0;     // write to FIFO A address
 	GBA_REG_DMA1CNT_H = 0xb600;	    // dma control: DMA enabled+ start on FIFO+32bit+repeat
	
	// changed
 	GBA_REG_TM2D = 0xffff-size;	// 0xffff-the number of samples to play
 	GBA_REG_TM2CNT = 0xC4;		    // enable timer2 + irq and cascade from timer 0
	
 	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
 	if(!fastbool) GBA_REG_TM0D = 0xF7ff;	// 8khz playback freq
	else GBA_REG_TM0D = 0xFDff;				// 32khz playback
 	GBA_REG_TM0CNT = 0x0080; 	    // enable timer at CPU freq 

	m_playing_munch02=munchbool;
	if(munchbool) m_munch_data02=data;
	if(munchbool) m_munch_size02=size;
	if(munchbool) m_munch_fast02=fastbool;

 	m_playing02 = true;
}

void CTone::Stop02()
{
	// checked with GBATEK; not tested

 	GBA_REG_TM0CNT = 0;	 // disable timer 0
 	GBA_REG_TM2CNT = 0;	 // disable timer 2
 	GBA_REG_DMA1CNT_H = 0; // stop DMA
 	m_playing02 = false;	

	m_pIntCont->UnRegister(5);
}

bool CTone::Playing02()
{
	// Checked with GBATek; not tested
	
 	return m_playing02;
}

int CTone::Play13(char *data, int size, bool munchbool, bool fastbool)
{

	// NOT checked with GBATEK; not tested

	// register timer 3 (interrupt bit 6) with interrupt controller 
	m_pIntCont->Register(this, 6); // changed
	m_pIntCont->Unmask(6); // changed

 	// Play a mono sound at 8khz in DMA mode Direct Sound
 	// uses timer 1 as sampling rate source
 	// uses timer 3 to count the samples played in order to stop the sound 

 	GBA_REG_SGCNT0_L = 0;
 	//GBA_REG_SGCNT0_H = 0x0b0F;       // DS A&B + fifo reset + timer0 + max volume to L and R
	// was    0000101100001111
	// is now 1111000000001110
	GBA_REG_SGCNT0_H |= 0xF00E;       // Only touches DS B
	
 	GBA_REG_SGCNT1 = 0x0080;       // turn sound chip on
	
 	GBA_REG_DMA2SAD = (unsigned long)data;	// dma1 source
 	GBA_REG_DMA2DAD = 0x040000a4;     // write to FIFO B address; changed
 	GBA_REG_DMA2CNT_H = 0xb600;	    // dma control: DMA enabled+ start on FIFO+32bit+repeat
	
	// changed
 	GBA_REG_TM3D = 0xffff-size;	// 0xffff-the number of samples to play
 	GBA_REG_TM3CNT = 0xC4;		    // enable timer3 + irq and cascade from timer 1
	
 	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
 	if(!fastbool) GBA_REG_TM0D = 0xF7ff;	// 8khz playback freq
	else GBA_REG_TM0D = 0xFDff;				// 32khz playback
 	GBA_REG_TM0CNT = 0x0080; 	    // enable timer at CPU freq 

	m_playing_munch02=munchbool;
	if(munchbool) m_munch_data02=data;
	if(munchbool) m_munch_size02=size;
	if(munchbool) m_munch_fast02=fastbool;

 	m_playing02 = true;
}

void CTone::Stop13()
{
	// NOT checked with GBATEK; not tested

 	GBA_REG_TM0CNT = 0;	 // disable timer 0
 	GBA_REG_TM2CNT = 0;	 // disable timer 2
 	GBA_REG_DMA1CNT_H = 0; // stop DMA
 	m_playing02 = false;	

	m_pIntCont->UnRegister(5);
}

bool CTone::Playing13()
{
	// NOT Checked with GBATek; not tested
	
 	return m_playing02;
}

*/

void CTone::Interrupt(unsigned char vector)
{
	// this routine is called when timer 1 or 3 overflows...
	
	if(vector==4)
	{	
		if(m_remaining_samples > 0)
		{
			Stop();
			Play(m_remaining_addr, m_remaining_samples, false, m_remaining_fast, 0, true);
		}
		else if(m_playing_munch) 
		{
			if(m_munch_loops > 0)
			{
				m_munch_loops--;
				Stop();
				Play(m_munch_data,m_munch_size,true,m_munch_fast,m_munch_loops);
			}
			else if(m_munch_loops < 0)
			{
				Stop();
				Play(m_munch_data,m_munch_size,true,m_munch_fast,m_munch_loops);
			}
			else
			{
				Stop();
			}
		}
		else 
			Stop();
	}
	else if(vector==6)
	{
		if(m_remaining_samples_b > 0)
		{
			StopB();
			PlayB(m_remaining_addr_b, m_remaining_samples_b, false, m_remaining_fast_b, 0, true);
		}
		else if(m_playing_munch_b) 
		{
			if(m_munch_loops_b > 0)
			{
				m_munch_loops_b--;
				StopB();
				PlayB(m_munch_data_b,m_munch_size_b,true,m_munch_fast_b,m_munch_loops_b);
			}
			else if(m_munch_loops_b < 0)
			{
				StopB();
				PlayB(m_munch_data_b,m_munch_size_b,true,m_munch_fast_b,m_munch_loops_b);
			}
			else
			{
				StopB();
			}
		}
		else 
			StopB();
	}
}

void CTone::SetAmplitudeResolution(unsigned short resolution)
{
	// Sets SOUNDBIAS to the specified amplitude resolution.

	// Gets current value from reg
	unsigned short reg_value = GBA_REG_SGBIAS;

	// Removes old resolution
	reg_value &= 0x3FFF;

	// Sets resolution to specified value
	reg_value |= (resolution << 14);

	// Sets register
	GBA_REG_SGBIAS = reg_value;
	
	m_amp_res = resolution;
}

unsigned short CTone::GetAmplitudeResolution()
{
	return(m_amp_res);
}


int CTone::PlayNoInt(char *data, int size, unsigned long long current_time)
 	{

	// register timer 1 (interrupt bit 4) with interrupt controller 
	//m_pIntCont->Register(this, 4);
	//m_pIntCont->Unmask(4);

 	// Play a mono sound at 8khz in DMA mode Direct Sound
 	// uses timer 0 as sampling rate source
 	// uses timer 1 to count the samples played in order to stop the sound 

 	GBA_REG_SGCNT0_L = 0;
 	GBA_REG_SGCNT0_H = 0x0b0F;       // DS A&B + fifo reset + timer0 + max volume to L and R
 	GBA_REG_SGCNT1 = 0x0080;       // turn sound chip on
	
 	GBA_REG_DMA1SAD = (unsigned long)data;	// dma1 source
 	GBA_REG_DMA1DAD = 0x040000a0;     // write to FIFO A address
 	GBA_REG_DMA1CNT_H = 0xb600;	    // dma control: DMA enabled+ start on FIFO+32bit+repeat
	
 	//GBA_REG_TM1D = 0xffff-size;	// 0xffff-the number of samples to play
 	//GBA_REG_TM1CNT = 0xC4;		    // enable timer1 + irq and cascade from timer 0
	
 	//Formula for playback frequency is: 0xFFFF-round(cpuFreq/playbackFreq)
 	GBA_REG_TM0D = 0xF7ff;	    // 8khz playback freq
 	GBA_REG_TM0CNT = 0x0080; 	    // enable timer at CPU freq 

	m_time_to_stop = current_time + size * 125 - m_sound_padding; // size in samples * 125 microseconds/sample (assuming 8KHz playback)

 	m_playing_no_int = true;
 	}

void CTone::CheckStopNoInt(unsigned long long current_time)
{
	if(m_time_to_stop < current_time)
	{
		StopNoInt();
	}
}

 void CTone::StopNoInt()
 	{
 	GBA_REG_TM0CNT = 0;	 // disable timer 0
 	//GBA_REG_TM1CNT = 0;	 // disable timer 1
 	GBA_REG_DMA1CNT_H = 0; // stop DMA
 	m_playing_no_int = false;	

	//m_pIntCont->UnRegister(4);
 	}

 bool CTone::PlayingNoInt()
 	{
 	return m_playing_no_int;
 	}
