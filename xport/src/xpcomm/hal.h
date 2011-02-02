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

#ifndef _HAL_H
#define _HAL_H

#include "property.h"
#include "pport.h"
#include "debug.h"
#include "model.h"
#include "objects.h"
#include <fstream>

#define XPH_PORTS_NUM		3

#define XPH_SIZE_BUFFER		0x40000

#define XPH_DELAY_READ		0
#define XPH_DELAY_WRITE		0

#define XPH_CRESET			PPORT_CONTROL,	0
#define XPH_CDATA			PPORT_CONTROL,	3
#define XPH_CDIR			XPH_CDATA
#define XPH_CCLOCK			PPORT_CONTROL,	2	
#define XPH_CSTROBE			XPH_CCLOCK
#define XPH_CD0				PPORT_DATA,     0
#define XPH_CD1				PPORT_DATA,     1
#define XPH_CD2				PPORT_DATA,     2
#define XPH_CD3				PPORT_DATA,     3
#define XPH_CREADY			PPORT_STATUS,	1
#define XPH_CSENSE			PPORT_STATUS,	2

#define XPHPP_XPORT_TO_HOST	PPORT_CONTROL,	5,	1
#define XPHPP_HOST_TO_XPORT	PPORT_CONTROL,	5,	0
#define XPHXP_XPORT_TO_HOST	XPH_CDIR,		0
#define XPHXP_HOST_TO_XPORT	XPH_CDIR,		1


enum EXPresetMode
	{
	RESET_CPU_ONLY = 0,
	RESET_LOGIC_SERIAL,		// reset logic, configure slave serial
	RESET_LOGIC_PC0,		// reset logic, use parallel configuration 0
	RESET_LOGIC_PC1			// reset logic, use parallel configuration 1
	};

class CXPcport;

class CXPhal : public IXPproperty, public CXPmodel
	{
public:
	CXPhal();
	~CXPhal();
	int Initialize();
	int Detect(bool *pconnected, bool *ppower=NULL, bool config=true);
	int WaitDetectPower(bool power, bool config=true, unsigned long timeout=0xffffffff);
	unsigned char GetModel(bool config=true, bool hostControl=false);
	int ConfigureSerial(const unsigned char *data, unsigned long len, bool reset=true);
	int ConfigureSerial(std::istream &is, bool reset=true);
	int ConfigureSerial(char *filename, bool reset=true);
	int Reset(bool resetCPU=true, EXPresetMode mode=RESET_CPU_ONLY);
	static void Delay(unsigned long ms);
	void WriteBit(unsigned char port, unsigned char bit, unsigned char data);
	void ToggleBit(unsigned char port, unsigned char bit);
	inline unsigned char Read(unsigned char port)
		{
		if (port<XPH_PORTS_NUM)
			{
			volatile unsigned long d;
			unsigned char prevRead;
			int i = m_readConsistency;

			if (!m_ppport)
				return 0;
			for (d=0; d<m_delayRead; d++);
			prevRead = (m_ppport->Read(port)^m_invMask[port])>>m_shiftRight[port];
			while(i)
				{
				for (d=0; d<m_delayRead; d++);
				if (prevRead==(prevRead=(m_ppport->Read(port)^m_invMask[port])>>m_shiftRight[port]))
					i--;
				else
					i = m_readConsistency;
				}
			return prevRead;
			}
		else
			return 0;
		}
	inline unsigned char ReadBit(unsigned char port, unsigned char bit)
		{
		return Read(port)&(1<<bit) ? 1 : 0;
		}
	inline void Write(unsigned char port, unsigned char val)
		{
		if (port<XPH_PORTS_NUM)
			{
			volatile unsigned long d;

			if (m_writeState[port]==val)
				return;
			if (!m_ppport)
				return;
			m_writeState[port] = val;
			for (d=0; d<m_delayWrite; d++);
			m_ppport->Write(port, val^m_invMask[port]);
			for (d=0; d<m_delayWrite; d++);
			}
		}
	inline void XportToHost()
		{
		WriteBit(XPHPP_XPORT_TO_HOST);
		WriteBit(XPHXP_XPORT_TO_HOST);
		}
	inline void HostToXport()
		{
		WriteBit(XPHXP_HOST_TO_XPORT);
		WriteBit(XPHPP_HOST_TO_XPORT);
		}
	inline void SetPC0()
		{
		WriteBit(XPHPP_HOST_TO_XPORT);
		WriteBit(XPH_CRESET, 0);
		WriteBit(XPH_CD0, 0);
		WriteBit(XPH_CD1, 1);
		WriteBit(XPH_CCLOCK, 1);
		WriteBit(XPH_CDATA, 1);
		}
	inline void SetSerial()
		{
		WriteBit(XPHPP_HOST_TO_XPORT);
		WriteBit(XPH_CD0, 1);
		WriteBit(XPH_CD1, 0);
		WriteBit(XPH_CCLOCK, 1);
		WriteBit(XPH_CDATA, 1);
		}
	inline void SetCPUReset()
		{
		WriteBit(XPHPP_HOST_TO_XPORT);
		WriteBit(XPH_CD0, 1);
		WriteBit(XPH_CD1, 1);
		WriteBit(XPH_CCLOCK, 1);
		WriteBit(XPH_CDATA, 1);
		}
	inline void SetNC0()
		{
		WriteBit(XPHPP_HOST_TO_XPORT);
		WriteBit(XPH_CD0, 0);
		WriteBit(XPH_CD1, 0);
		WriteBit(XPH_CCLOCK, 0);
		WriteBit(XPH_CDATA, 1);
		}
	inline void PulseReset()
		{
		WriteBit(XPH_CRESET, 1);
		Delay(100);
		WriteBit(XPH_CRESET, 0);
		}

	static bool m_break;
	CXPcport *m_pcport;

protected:
	int m_test;
	static void SigHandler();

	// IXPproperty methods
	virtual int SetProperty(EXPproperty property, unsigned long val);
	virtual int GetProperty(EXPproperty property, unsigned long *pval);


private:
	CXPpport *m_ppport;
	bool m_initialized;
	unsigned char m_invMask[XPH_PORTS_NUM];
	unsigned char m_writeState[XPH_PORTS_NUM];
	unsigned char m_shiftRight[XPH_PORTS_NUM];
	EXPresetMethod m_methodReset;
	unsigned long m_delayRead;
	unsigned long m_delayWrite;
	unsigned char m_port;  // LPT number for printer port
	bool m_configx;	// true if we are configured with something other than pc0
	CXPdebug m_debug;
	int m_readConsistency;

	void ConfigureByte(unsigned char byte);
	};

#endif //_HAL_H
