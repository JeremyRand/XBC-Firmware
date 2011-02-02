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

#ifndef _DSOUND_H
#define _DSOUND_H

#include "iinterrupt.h"
#include "intcont.h"

class CDirectSound : public IInterrupt
	{
public:
	CDirectSound(CInterruptCont *pIntCont);
	virtual ~CDirectSound();

	int Play(char *data, int size, bool munch = false);
	void Stop();
    bool Playing();

private:
	// IInterrupt 
	virtual void Interrupt(unsigned char vector);

	bool m_playing;

	char * m_munch_data;
	bool m_playing_munch;
	int m_munch_size;

	CInterruptCont *m_pIntCont;
	};


#endif
