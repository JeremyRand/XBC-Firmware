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

#include <textdisp.h>
#include <diffrobot1.h>

// Put your Bluetooth Device Address here.  Use hexadecimal, capital letters, NOT separated by colons
#define HOST "00A0961F2025"

CTextDisp td(TDM_LCD);

void ExecuteCommand(CDiffRobot1 *pdr, char *command)
	{
	printf("Executing %s...\n", command);
	if (command[0]=='f' || command[0]=='F')
		{
		pdr->Move(0, pdr->GetPosition(0)+1000, 1000, 2000);
		pdr->Execute();
		while(!pdr->Done(0));
		}
	else if (command[0]=='b' || command[0]=='B')
		{
		pdr->Move(0, pdr->GetPosition(0)-1000, -1000, 2000);
		pdr->Execute();
		while(!pdr->Done(0));
		}
	else if (command[0]=='l' || command[0]=='L')
		{
		pdr->Move(1, pdr->GetPosition(1)+1571, 1000, 2000);
		pdr->Execute();
		while(!pdr->Done(1));
		}
	else if (command[0]=='r' || command[0]=='R')
		{
		pdr->Move(1, pdr->GetPosition(1)-1571, -1000, 2000);
		pdr->Execute();
		while(!pdr->Done(1));
		}
	}

int main(void)
	{
	int result, i=0;
	char c, command[0x100];
	volatile unsigned long d;

	CDiffRobot1 dr(5810, 26100, 0);

	// connect to host (Master)
	printf("Connecting to host %s...", HOST);
	result = dr.MasterConnect(HOST);
	if (result<0)
		printf("error.\n");
	else
		printf("success.\n");


	// command loop
	dr.WriteString("Move: ");
	while(1)
		{
		if (dr.ReadData(&c, 1, 0))
			{
			if (c=='\r' || i==0xff)
				{
				dr.WriteData(&c, 1);
				c = '\n';
				dr.WriteData(&c, 1);
				command[i] = '\0';
				if (i)
					ExecuteCommand(&dr, command);
				i = 0;
				dr.WriteString("Move: ");
				}
			else if (c=='\b')
				{
				if (i>0)
					{
					dr.WriteData(&c, 1);
					i--;
					}
				}
			else
				{
				dr.WriteData(&c, 1);
				command[i++] = c;
				}
			}
		}
	}
