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

#ifndef _MENU_H
#define _MENU_H

#define DM_SCREEN_WIDTH   30
#define DM_SCREEN_HEIGHT  20

#define DM_KEY_NORMAL   0x03ff

struct SMenuItem
	{
	void (*m_callback)(int, int, int);
	int m_val;
	char m_label[DM_SCREEN_WIDTH+1];
	};

class CMenu
	{
public:
	CMenu();
	~CMenu();

	int AddItem(void (*callback)(int, int, int), int val, char *label); // can add "type" field later

	static void Printxyf(unsigned char x, unsigned char y, unsigned char length, bool emphasis, char *format, ...); 
	static char GetInput();
	static bool Return();

	void Execute();
	

private:
	void CMenu::PrintItem(unsigned char item, bool emphasis);

	unsigned char m_numItems;
	unsigned char m_selectedItem;
	SMenuItem m_itemList[DM_SCREEN_HEIGHT];

	unsigned char m_topMargin;
	};

#endif
