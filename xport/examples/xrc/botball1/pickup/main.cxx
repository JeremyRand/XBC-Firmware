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

#include "intcont.h"
#include "textdisp.h"
#include "vision.h"
#include "axesc.h"
#include "diffbotball1.h"
#include "../logic/botball1.h"
#include "gpioint.h"
#include "btnstate.h"
#include "pid.h"
#include "gripper.h"
#include "pursue.h"

CTextDisp td(TDM_LCD_AND_CPORT);

void print_blobs(CBlob *blob, int minarea)
	{
	SMomentStats stats;
	int i=0;
		
	printf("---------------------------------------------------------\n");
    printf("Blob stats:\n");
	
	while(blob!=NULL) 
		{
		blob->moments.GetStats(stats);
		if(stats.area >= minarea) 
			{
			printf("\tBlob %d: area %d, centroid (%f, %f)\n", 
				i++, stats.area, stats.centroidX, stats.centroidY);
			}
	    blob = blob->next;
		}
	}





int main(void)
	{
#if 0
	volatile long long ll;
	ll=3510;
	ll*=3143529;
	printf("%d\n", (int)(ll/1000));
	while(1);
#endif
	CInterruptCont intCont;
#if 0
  CAxesClosed m_axes(&intCont, BEMF4_BASE, BEMF4_VECTOR);

  m_axes.SetPIDGains(600, 0, 750);

  printf("Moving first\n");
  m_axes.Move(0, -50000, 10000, 10000);
  m_axes.Execute();
  for(volatile int i = 0; i < 1000000; ++i); //Very short wait
  printf("Moving second\n");
  m_axes.Move(1, 50000, 10000, 10000);
  m_axes.Execute();
  while(1);


#endif
	printf("Creating vision object...\n");
	CVision vision(&intCont);
	CDiffBase db(5830, 26100, &intCont, BEMF4_BASE, BEMF4_VECTOR);
	CGpioInt gpio(&intCont);

	db.SetPIDGains(600, 0, 750);
	db.SetCalibrate(0, 2048 -55);

#if 0
	CAxesClosed &ac = db;
	//CDiffBase &ac = db;
	((CDiffBase &)ac).Move(1, 1571, 1800, 1800);
	(CDiffBase &)ac.Execute();
	while(!(CDiffBase &)ac.Done(1))
		printf("%d\n", (CDiffBase &)ac.GetPosition(0));
#endif

	// set interleave to 3 frames to save CPU (for example)
//	vision.SetRenderInterleave(3);

	// Setup display options
	CVisionDispOptions options; 
 	options.minBlobArea=100;

 	printf("Setting up vision menu...\n");

	CDiffBotball1Menu *menu = new CDiffBotball1Menu(vision, options, db, gpio);

	vision.SetRenderMode(RM_NONE);
	vision.SetRenderEnable(0, true);
	vision.SetRenderEnable(1, false);
	vision.SetRenderEnable(2, false);

	menu->HandleMenu();

	unsigned int fnum;
	CBlob *blobs[DV_MODELS];

#if 0
	while(1)
		{
		fnum = vision.GetBlobs(blobs);
		print_blobs(blobs[0], 100);
		}
#endif
#if 0
	int x, y;
	int hVel, tVel;
	SMomentStats stats;
	Cpid heading(2000, 0, 0);
	Cpid trans(10000, 0, 0);
	CBlob *blob;
	while(1)
		{
		fnum = vision.GetBlobs(blobs);
		blob = blobs[0];
		tVel = 0;
		hVel = 0;
		if (blob)
			{
			blob->moments.GetStats(stats);
			if (stats.area>100)
				{
				x = (int)stats.centroidX; 
				y = (int)stats.centroidY;
				tVel = trans.Compensate(200-y);
				hVel = heading.Compensate(170-x);
				printf("%d %d %d %d\n", x, y, hVel, tVel);
				}
			}
		db.MoveVelocity(0, tVel, 20000);
		db.MoveVelocity(1, hVel, 20000);
		db.Execute();
		}
#endif
#if 0
	printf("setpwm\n");
	// open
	// db.SetPWM(2, 120);
	db.SetPWM(2, -40);
	while(1);
#endif
#if 0
	CBtnState btnState;
	CGripper gripper(&db, 2);

	while(1)
		{
		gripper.Open();
		while(1)
			{
			btnState.PollKeys();
			if (btnState.KeyHit(CBtnState::A_BUTTON))
				break;
			}

		printf("%d\n", gripper.Close());
		while(1)
			{
			btnState.PollKeys();
			if (btnState.KeyHit(CBtnState::A_BUTTON))
				break;
			}
		}
#endif
#if 1
	CPursue pursue(&db, &vision);
	CGripper gripper(&db, 2);
	int i=0, x, y, pos0, pos1, pos;
	volatile int d;

	while(1)
		{
		pursue.Find(0);
		printf("found\n");
		pos0 = db.GetPosition(0);
		if (pursue.Pursue(2500)==0)
			{		
			pos1 = db.GetPosition(0);
			pursue.Estimate(&x, &y);
			printf("%d %d\n", x, y);
			gripper.Open();
			db.MoveVelocity(0, 600, 10000);
			db.Execute();
			while(1)
				{
				pos = db.GetPosition(0);
#if 0
				printf("pos %d %d\n", pos, x-300);
				if (pos-pos0>x-300)
#else
				printf("pos %d %d\n", pos-pos1);
				if (pos-pos1>900)				
#endif
					{
					db.MoveVelocity(0, 0, 5000);
					db.Execute();
					printf("stop\n");
					while(!db.Done(0));
					printf("*");
					break;
					}
				}
			printf("close\n");
			gripper.Close();
			
			// rotate 180 degrees
			printf("rotate\n");
			db.Move(1, db.GetPosition(1)+3142, 2500, 5000);
			db.Execute();
			while(!db.Done(1));
			
			// go back
			pos = db.GetPosition(0);
			printf("return\n");
#if 0
			db.Move(0, db.GetPosition(0)+x, 2500, 5000);
#else
			db.Move(0, db.GetPosition(0)+pos-pos0, 2500, 5000);
#endif
			db.Execute();
			while(!db.Done(0));
			
			// open
			gripper.Open();
			for (d=0; d<500000; d++);
			
			// rotate 180 degrees
			printf("rotate\n");
			db.Move(1, db.GetPosition(1)-3142, 2500, 5000);
			db.Execute();
			while(!db.Done(1));
			
			// release
			gripper.Release();
			}
		else
			{
			db.Stop(0);
			db.Stop(1);
			db.Execute();
			}
		}
	while(1);
#endif
	}
