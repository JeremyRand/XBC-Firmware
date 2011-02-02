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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "textdisp.h"
#include "menu.h"
#include "gba.h"

CMenu::CMenu()
	{
	m_numItems = 0;
	m_selectedItem = 0;
	m_topMargin = 0;
	}

CMenu::~CMenu()
	{
	}

int CMenu::AddItem(void (*callback)(int, int, int), int val, char *label)
	{
	if (m_numItems>=DM_SCREEN_HEIGHT || strlen(label)>DM_SCREEN_WIDTH)
		return -1;

	m_itemList[m_numItems].m_val = val;
	m_itemList[m_numItems].m_callback = callback;
	strcpy(m_itemList[m_numItems].m_label, label);
	
	m_numItems++;

	m_topMargin = (DM_SCREEN_HEIGHT - m_numItems)>>1;

	return 0;
	}

void CMenu::Printxyf(unsigned char x, unsigned char y, unsigned char length, bool emphasis, char *format, ...)
	{
	char buf[128];
	unsigned char i;

	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	// justify string within field
	if (length!=0)
		{
		for (i=0; buf[i]; i++)
			if (i==length)
				break;
		for (; i<length; i++)
			buf[i] = ' ';
		buf[i] = '\0';
		}

	if (emphasis)
		ptd->SetProperty(DP_REVERSE, true);
	ptd->SetPosition(x, y);
	ptd->Print(buf);
	ptd->SetProperty(DP_REVERSE, false);
	}

void CMenu::PrintItem(unsigned char item, bool emphasis)
	{
	unsigned char length = strlen(m_itemList[item].m_label);
	Printxyf((DM_SCREEN_WIDTH-length)>>1, m_topMargin+item, length, emphasis, m_itemList[item].m_label); 
	}


void CMenu::Execute()
	{
	unsigned char i;
	char c;
	bool quit = false;
	char *message0 = "Press 'Start' to return";
	char *message1 = "Press 'A' to return";
	ptd->Clear();
	
	// select
	while(!quit)
		{
		// print list
		for (i=0; i<m_numItems; i++)
			PrintItem(i, m_selectedItem==i);
		Printxyf((DM_SCREEN_WIDTH-strlen(message0))>>1, DM_SCREEN_HEIGHT-1, 0, false, message0);
		while(1)
			{
			c = GetInput();
			if (c=='D' && m_selectedItem<(m_numItems-1))
				{
				m_selectedItem++;
				break;
				}
			else if (c=='U' &&  m_selectedItem>0)
				{
				m_selectedItem--;
				break;
				}
			else if (c=='A')
				{
				ptd->Clear();
				Printxyf((DM_SCREEN_WIDTH-strlen(message1))>>1, DM_SCREEN_HEIGHT-1, 0, false, message1);
				ptd->SetPosition(0, 0);
				(*m_itemList[m_selectedItem].m_callback)(m_itemList[m_selectedItem].m_val, 0, 0);
				ptd->Clear();
				break;
				}
			else if (c=='S')
				{
				ptd->Clear();
				quit = true;
				break;
				}
			}
		}
	}

bool CMenu::Return()
	{
	return GetInput()=='A';
	}

char CMenu::GetInput()
	{
	volatile unsigned int d;
	unsigned short keys;
	char result = '\0';
	static bool keyPress = false;

	keys = GBA_REG_P1;
	if (!keyPress && GBA_REG_P1!=DM_KEY_NORMAL)
		{
		for (d=0; d<2000; d++);
		keys = GBA_REG_P1;
		if (keys!=DM_KEY_NORMAL)
			{
			keyPress = true;
			if (!(keys&GBA_KEY_A))
				result ='A';
			else if (!(keys&GBA_KEY_B))
				result = 'B';
			else if (!(keys&GBA_KEY_SL))
				result = 's';
			else if (!(keys&GBA_KEY_ST))
				result = 'S';
			else if (!(keys&GBA_KEY_RT))
				result = 'R';
			else if (!(keys&GBA_KEY_LFT))
				result = 'L';
			else if (!(keys&GBA_KEY_UP))
				result = 'U';
			else if (!(keys&GBA_KEY_DWN))
				result = 'D';
			else if (!(keys&GBA_KEY_R))
				result = 'r';
			else if (!(keys&GBA_KEY_L))
				result = 'l';
			}
		}
	else if (keyPress && GBA_REG_P1==DM_KEY_NORMAL)
		{
		for (d=0; d<2000; d++);
		if (GBA_REG_P1==DM_KEY_NORMAL)
			keyPress = false;
		}

	return result;
	}
