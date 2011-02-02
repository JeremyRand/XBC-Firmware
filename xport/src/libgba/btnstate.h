#ifndef CBTNSTATE_H
#define CBTNSTATE_H

#include "gba.h"

//#define NORMBUTTONS

class CBtnState
{
public:
	enum
	{
		A_BUTTON		= GBA_KEY_A,
		B_BUTTON		= GBA_KEY_B,
		SELECT_BUTTON	= GBA_KEY_SL,
		START_BUTTON	= GBA_KEY_ST,
		RIGHT_BUTTON	= GBA_KEY_RT,
		LEFT_BUTTON		= GBA_KEY_LFT,
		UP_BUTTON		= GBA_KEY_UP,
		DOWN_BUTTON		= GBA_KEY_DWN,
		R_BUTTON		= GBA_KEY_R,
		L_BUTTON		= GBA_KEY_L
	};
	CBtnState() { m_keyCurrentState = GBA_REG_P1; PollKeys(); }
	~CBtnState() {}
	//Polling and key status functions ripped from TONC
	//http://user.chem.tue.nl/jakvijn/tonc/keys.htm
	void PollKeys() { m_keyPreviousState = m_keyCurrentState; m_keyCurrentState= GBA_REG_P1; }
	void JustUpdateKeys() { m_keyCurrentState= GBA_REG_P1; }
	bool KeyTransit(unsigned short key)		{	return  (m_keyCurrentState ^ m_keyPreviousState) & key; }
	bool KeyHeld(unsigned short key)		{	return ~(m_keyCurrentState | m_keyPreviousState) & key; }
	bool KeyHit(unsigned short key)			{	return (~m_keyCurrentState & m_keyPreviousState) & key; }
	bool KeyReleased(unsigned short key)	{	return  (m_keyCurrentState & ~m_keyPreviousState) & key; }
	
	bool KeyUp(unsigned short key)			{	return  (m_keyCurrentState & key); }
	bool KeyDown(unsigned short key)		{	return (~m_keyCurrentState & key); }

  unsigned short CurrState() { return(m_keyCurrentState); }
  unsigned short PrevState() { return(m_keyPreviousState); }
protected:
	unsigned short m_keyCurrentState;
	unsigned short m_keyPreviousState;
};

#endif
