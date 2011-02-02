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
#include <dsound.h>
#include "muchacha.inl"
#include "ahh.inl"
#include "blue.inl"
#include "hokey.inl"
#include "ouch.inl"
#include "prado.inl"

CTextDisp td(TDM_LCD);

#define TURN      0 
#define STRAIGHT  1
#define BUMP      2

// DirectSound play routine
void play(CDirectSound *pds, int type, int random)
	{
	switch (type)
		{
		case TURN:
			if (!pds->Playing())
				{
				if ((random&0x03)==2)
					pds->Play((char *)blue, sizeof(blue));
				else if ((random&0x03)==3)
					pds->Play((char *)hokey, sizeof(hokey));
				}
			break;

		case STRAIGHT:
			if (!pds->Playing())
				{
				if (random&0x01)
					pds->Play((char *)prado, sizeof(prado));
				else
					pds->Play((char *)muchacha, sizeof(muchacha));
				}
			break;

		case BUMP:
			pds->Stop();
			if (random&0x01)
				pds->Play((char *)ahh, sizeof(ahh)); 
			else 
				pds->Play((char *)ouch, sizeof(ouch));
			break;
		}
	}

// Backup and avoid routine
void Avoid(CDiffBase *pdb, CDirectSound *pds, short diff, short dir)
	{
	short leftTorque = 0, rightTorque = 0;
	AxisPosition dest;

	play(pds, BUMP, pdb->GetPosition(0));
	
	pdb->Stop(0);
	pdb->Stop(1);

	if (dir>0)
		{
		dest = pdb->GetPosition(0)+1000;
		pdb->Move(0, dest, 1000, 2000);
		}
	else
		{
		dest = pdb->GetPosition(0)-1000;
		pdb->Move(0, dest, -1000, 2000);
		}
	if (diff>0)
		pdb->Move(1, pdb->GetPosition(1)+1571, 1000, 2000);
	else
		pdb->Move(1, pdb->GetPosition(1)-1571, -1000, 2000);

	pdb->Execute();

	while((!pdb->Done(0)) || (!pdb->Done(1)))
		{
		if (pdb->GetCurrent(0)>9000)
			rightTorque++;

		if (pdb->GetCurrent(1)>9000)
			leftTorque++;

		if (leftTorque>50 || rightTorque>50)
			{
			Avoid(pdb, pds, rightTorque-leftTorque, 1);
			break;
			}
		printf("Avoiding %d %d\n", pdb->GetPosition(0), pdb->GetPosition(1));
		}
	}


// Wander routine
int main(void)
	{

	short leftDist, rightDist, minDist;
	short diffDist;
	short stopped = 0;
	int going = 0;
	long transVel, rotVel;
	short leftTorque = 0, rightTorque = 0;

	CDiffRobot1 dr(5810, 26100, 0);

	CDirectSound ds(dr.GetInterruptCont());

	// configure XIR sensors
	dr.SetMode(0xff);
	dr.SetXirPower(0, 0x30);
	dr.SetXirPower(1, 0x30);
	dr.SetXirRingLength(2);
	dr.SetXirAverager(2);

	while(1)
		{
		// determine minimum distance
		leftDist = dr.GetXir(1);
		rightDist = dr.GetXir(0);
		if (leftDist<rightDist)
			minDist = leftDist;
		else
			minDist = rightDist;
		diffDist = leftDist - rightDist;

		// calculate relative torque delivered to wheels
		if (dr.GetCurrent(0)>9000)
			rightTorque++;
		if (dr.GetCurrent(1)>9000)
			leftTorque++;

		// calculate rotation velocity
		rotVel = 30*diffDist + 30*stopped;
		if (rotVel>2000)
			rotVel = 3000;
		if (rotVel<-3000)
			rotVel = -3000;

		// calculate translation velocity
		transVel = 2000 - 20*(512 - minDist);
		if (transVel<0)
			transVel = 0;
		if (transVel>2500)
			transVel = 2500;

		// if we are going slow, increment 'stopped', which is a rough estimate
		// of how frustrated the robot is...
		if (transVel<25)
			{
			if (diffDist>0 && stopped<150)
				stopped++;
			else if (diffDist<0 && stopped>-150)
				stopped--;
			}
		else 
			{
			if (stopped>50)
				stopped -= 10;
			else if (stopped<-50)
				stopped += 10;
			else
				stopped = 0;
			}

		// if we are going along, increment 'going', which is a rough esimate 
		// of how happy the robot is...
		if (transVel>100)
			going++;

		if (diffDist>150 || diffDist<-150 || stopped>100 || stopped<-100)
			play(&ds, TURN, dr.GetPosition(0));
		else if (going>7000)
			{
			going = 0;
			play(&ds, STRAIGHT, dr.GetPosition(0));
			}

		// check to see if wheels have "locked-up"
		if (leftTorque>50 || rightTorque>50)
			{
			Avoid(&dr, &ds, rightTorque-leftTorque, -1);
			leftTorque = 0;
			rightTorque = 0;
			}
		else // else move according to calculated rotational and translational velocities...
			{
			dr.Move(0, dr.GetPosition(0)+10000, transVel, 2000);
			
			if (rotVel>0)
				dr.Move(1, dr.GetPosition(1)+10000, rotVel, 2000);
			else
				dr.Move(1, dr.GetPosition(1)-10000, rotVel, 2000);
			dr.Execute();
			}
		printf("Wandering %d %d\n", transVel, rotVel);
		}
	}
