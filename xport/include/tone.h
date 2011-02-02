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

#ifndef _TONE_H
#define _TONE_H

//----------------------------------
// http://www.work.de/nocash/gbatek.htm#soundcontroller

///////////////////////////////////////////////////////////////////////////
// Master Sound Control Registers
//
// 080h - SOUNDCNT_L (SGCNT0_L) (NR50, NR51)-Channel L/R Volume/Enable (R/W)
//   Bit   Expl.
//   0-2   Sound 1-4 Master volume RIGHT (0-7)
//   3     Not used
//   4-6   Sound 1-4 Master Volume LEFT (0-7)
//   7     Not used
//   8-11  Sound 1-4 Enable Flags RIGHT (each Bit 8-11, 0=Disable, 1=Enable)
//   12-15 Sound 1-4 Enable Flags LEFT (each Bit 12-15, 0=Disable, 1=Enable)
//
// 082h - SOUNDCNT_H (SGCNT0_H) (GBA only) - DMA Sound Control/Mixing (R/W)
//   Bit   Expl.
//   0-1   Sound # 1-4 Volume   (0=25%, 1=50%, 2=100%, 3=Prohibited)
//   2     DMA Sound A Volume   (0=50%, 1=100%)
//   3     DMA Sound B Volume   (0=50%, 1=100%)
//   4-7   Not used
//   8     DMA Sound A Enable RIGHT (0=Disable, 1=Enable)
//   9     DMA Sound A Enable LEFT  (0=Disable, 1=Enable)
//   10    DMA Sound A Timer Select (0=Timer 0, 1=Timer 1)
//   11    DMA Sound A Reset FIFO   (1=Reset)
//   12    DMA Sound B Enable RIGHT (0=Disable, 1=Enable)
//   13    DMA Sound B Enable LEFT  (0=Disable, 1=Enable)
//   14    DMA Sound B Timer Select (0=Timer 0, 1=Timer 1)
//   15    DMA Sound B Reset FIFO   (1=Reset)
//
// 084h - SOUNDCNT_X (SGCNT1) (NR52) - Sound on/off (R/W)
// When not using sound output, write 00h to this register to save
// power consumption. While Bit 7 is cleared, all other sound
// registers cannot be accessed, and their content must be
// re-initialized when re-enabling sound.
//
//   Bit   Expl.
//   0     Sound 1 ON flag (Read Only)
//   1     Sound 2 ON flag (Read Only)
//   2     Sound 3 ON flag (Read Only)
//   3     Sound 4 ON flag (Read Only)
//   4-6   Not used
//   7     All sound on/off  (0: stop all sound circuits) (Read/Write)
//   8-15  Not used
//
// Bits 0-3 are automatically set when starting sound output, and are
// automatically cleared when a sound ends. (Ie. when the length
// expires, as far as length is enabled. The bits are NOT reset when
// an volume envelope ends.)
//
// 086h - Not used
//
// 088h - SOUNDBIAS (SG_BIAS) - Sound PWM Control (R/W, see below)
// This register controls the final sound output. The default setting
// is 0200h, it is normally not required to change this value.
//
//   Bit    Expl.
//   0-9    Bias Level(Default=200h, converting signed samples into unsigned)
//   10-13  Not used
//   14-15  Amplitude Resolution/Sampling Cycle (Default=0, see below)
//
// Amplitude Resolution/Sampling Cycle (0-3):
//   0  9bit / 32.768kHz   (Default, best for DMA channels A,B)
//   1  8bit / 65.536kHz
//   2  7bit / 131.072kHz
//   3  6bit / 262.144kHz  (Best for FM channels 1-4)

///////////////////////////////////////////////////////////////////////////
// Sound 1
//
// 060h - SOUND1CNT_L (formerly SG10_L) (NR10)-Channel 1 Sweep register (R/W)
//   Bit        Expl.
//   0-2   R/W  Number of sweep shift      (n=0-7)
//   3     R/W  Sweep Frequency Direction  (0=Increase, 1=Decrease)
//   4-6   R/W  Sweep Time; units of 7.8ms (0-7, min=7.8ms, max=54.7ms)
//   7-15  -    Not used
// 
// Sweep is disabled by setting Sweep Time to zero, if so, the
// direction bit should be set.  The change of frequency (NR13,NR14)
// at each shift is calculated by the following formula where X(0) is
// initial freq & X(t-1) is last freq:
//   X(t) = X(t-1) +/- X(t-1)/2^n
//
// 062h - SOUND1CNT_L (SG10_H) (NR11, NR12)-Channel 1 Duty/Len/Envelope (R/W)
//   Bit        Expl.
//   0-5   W    Sound length; units of (64-n)/256s  (0-63)
//   6-7   R/W  Wave Pattern Duty                   (0-3, see below)
//   8-10  R/W  Envelope Step-Time; units of n/64s  (1-7, 0=No Envelope)
//   11    R/W  Envelope Direction                  (0=Decrease, 1=Increase)
//   12-15 R/W  Initial Volume of envelope          (1-15, 0=No Sound)
//
// Wave Duty:
//   0: 12.5% ( -_______-_______-_______ )
//   1: 25%   ( --______--______--______ )
//   2: 50%   ( ----____----____----____ ) (normal)
//   3: 75%   ( ------__------__------__ )
//
// The Length value is used only if Bit 6 in NR14 is set.
//
// 064h - SOUND1CNT_X (SG11) (NR13, NR14) - Channel 1 Frequency/Control (R/W)
//   Bit        Expl.
//   0-10  W    Frequency; 131072/(2048-n)Hz  (0-2047)
//   11-13 -    Not used
//   14    R/W  Length Flag  (1=Stop output when length in NR11 expires)
//   15    W    Initial      (1=Restart Sound)

#include "iinterrupt.h"
#include "intcont.h"

/////////////////////////////////////////////////////////////////////////
// Class to interface to tone generator Sound 1.
class CTone : public IInterrupt {
public:
  CTone(CInterruptCont *pIntCont);
  virtual ~CTone();

  // Channel specific controls
  void SetFreq(int freq);
  void SetEnable(bool enable);

  bool IsEnabled() {
    return(m_enabled);
  }

  int  GetFreq() {
    return(m_freq);
  }

  // Master controls
  static void EnableMaster(bool enable);

  int Play(char *data, int size, bool munchbool=false, bool fastbool=false);
  void Stop();
  bool Playing();

  int PlayNoInt(char *data, int size, unsigned long long current_time);
  void CheckStopNoInt(unsigned long long current_time);
  void StopNoInt();
  bool PlayingNoInt();

  void SetAmplitudeResolution(unsigned short resolution);
  unsigned short GetAmplitudeResolution();

protected:
  // Internal function to set the frequency register
  //  void _SetFreqReg(bool restart);
  void _SetFreqReg();
  // Internal function to set/clear the enable register
  void _SetEnableReg(bool enable);

protected:
  int m_freq;
  bool m_enabled;
  unsigned short m_amp_res;
  long long m_time_to_stop;
  long long m_sound_padding;
  bool m_playing_no_int;
	
	char * m_munch_data;
	bool m_playing_munch;
	int m_munch_size;
  

private:
	// IInterrupt 
	virtual void Interrupt(unsigned char vector);

	bool m_playing;
	CInterruptCont *m_pIntCont;

};


#endif
