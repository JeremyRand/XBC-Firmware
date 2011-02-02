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


#include <assert.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include "hal.h"
#include "bitstream.h"
#include "cport.h"
#include "objects.h"
#include <signal.h>

bool CXPhal::m_break = false;

CXPhal::CXPhal() : CXPmodel(0)
	{
	m_delayRead = XPH_DELAY_READ;
	m_delayWrite = XPH_DELAY_WRITE;
	m_invMask[PPORT_DATA]		= 0x00;
	m_invMask[PPORT_STATUS]		= 0x80;
	m_invMask[PPORT_CONTROL]	= 0x08;
	m_shiftRight[PPORT_DATA]	= 0;
	m_shiftRight[PPORT_STATUS]	= 3;
	m_shiftRight[PPORT_CONTROL]	= 0;
	m_methodReset = RESET_POWER_TOGGLE;
	m_port = 0; // default LPT0
	m_ppport = NULL;
	m_pcport = NULL;
	m_readConsistency = 1;
	m_configx = false;
	m_initialized = false;
	}

CXPhal::~CXPhal()
	{
	if (m_pcport)
		delete m_pcport;
	if (m_initialized)
		{
		SetPC0();
		if (m_configx)
			PulseReset();	// load PC0
		}
	if (m_ppport)
		delete m_ppport;
	}


void CXPhal::SigHandler()
	{
	CXPhal::m_break = true;
	}

int CXPhal::Initialize()
	{
	int res;

	m_ppport = new CXPpport;
	res = (int)m_ppport->Open(m_port);
	if (res!=RES_OK)
		return res;

	m_pcport = new CXPcport(this);
	m_initialized = true;
	m_writeState[PPORT_DATA] = 0xff;
	m_writeState[PPORT_CONTROL] = 0xff;
	Write(PPORT_CONTROL, 0);
	Write(PPORT_DATA, 0);
	Cplatform::RegisterHandler(SigHandler);
	return res;
	}

unsigned char CXPhal::GetModel(bool config, bool hostControl)
	{
	int i;
	unsigned char model, bit;

	// set up for no config
	WriteBit(XPHPP_HOST_TO_XPORT);
	WriteBit(XPH_CD0, config);
	WriteBit(XPH_CD1, hostControl); // set host control
	WriteBit(XPH_CCLOCK, 0);
	WriteBit(XPH_CDATA, 0);
	WriteBit(XPH_CRESET, 1);
	WriteBit(XPHPP_XPORT_TO_HOST);

	for (i=0, model=0; i<8; i++)
		{
		bit	= ReadBit(XPH_CD1);
		bit <<= i;
		model |= bit;
		WriteBit(XPH_CCLOCK, 1);
		WriteBit(XPH_CCLOCK, 0);
		}
	WriteBit(XPHPP_HOST_TO_XPORT);
	WriteBit(XPH_CRESET, 0);

	return model;
	}

int CXPhal::Detect(bool *pconnected, bool *ppower, bool config)
	{
	int i;
	unsigned char model;

	if (pconnected)
		{
		WriteBit(XPH_CD0, 0);
		WriteBit(XPH_CCLOCK, 0);
		WriteBit(XPH_CDATA, 0);

		*pconnected = true;
		for (i=0; i<10; i++)
			{
			WriteBit(XPH_CRESET, 1);
			if (ReadBit(XPH_CSENSE))
				{
				*pconnected = false;
				break;
				}
			WriteBit(XPH_CRESET, 0);
			if (!ReadBit(XPH_CSENSE))
				{
				*pconnected = false;
				break;
				}
			}	
		WriteBit(XPH_CRESET, 0);
		}
	if (ppower)
		{
		*ppower = true;
		// determine if we are powered up
		model = GetModel(config);
		for (i=0; i<10; i++)
			{
			if (model!=(model=GetModel(config)))
				*ppower = false;
			}
		if (model==0 || model==0xff)
			*ppower = false;
		if (*ppower)
			SetModel(model);
		}

	return RES_OK;
	}

int CXPhal::WaitDetectPower(bool power, bool config, unsigned long timeout)
	{
	unsigned long t0;
	int i;
	bool qconnected, qpower;
	unsigned char model;

	Detect(&qconnected);
	if (!qconnected)
		return RES_FAIL;

	t0 = Cplatform::GetClock();
	while ((Cplatform::GetClock()-t0)<timeout)
		{
		Detect(NULL, &qpower, config);
		Delay(20);
		if (!qconnected)
			return RES_FAIL;
		if (qpower==power)
			{
			if (qpower)
				{
				Delay(100); // wait for fpga to configure
				for (i=0, model=0xff; i<10 && (model==0xff || model==0); i=i+1, model=GetModel(config));
				SetModel(model);
				}
			return RES_OK;
			}
		if (m_break)
			throw(EXCEPT_CTRL_C);
		}

	return RES_TIMEOUT;
	}

void CXPhal::WriteBit(unsigned char port, unsigned char bit, unsigned char data)
	{
	unsigned char out;

	if (port<XPH_PORTS_NUM)
		{
		if (data)
			data = 0x01;
		out = (m_writeState[port]&~(1<<bit))|(data<<bit);
		Write(port, out);
		}
	}

void CXPhal::ToggleBit(unsigned char port, unsigned char bit)
	{
	unsigned char out;

	if (port<XPH_PORTS_NUM)
		{
		out = (m_writeState[port])^(1<<bit);
		Write(port, out);
		}
	}


void CXPhal::Delay(unsigned long ms)
	{
	unsigned long t0;
	
	for (t0=Cplatform::GetClock(); (Cplatform::GetClock()-t0)<ms; Cplatform::YieldTask());
	}

int CXPhal::ConfigureSerial(char *filename, bool reset)
	{
	std::ifstream is(filename, std::ios::binary);//|ios::nocreate); 

	if(is.fail() || is.eof())
		{
		m_debug.printf(0, "Could not open %s\n", filename);
		return RES_FAIL;
		}
	
	return ConfigureSerial(is, reset);
	}

int CXPhal::ConfigureSerial(const unsigned char *data, unsigned long len, bool reset)
	{
	std::string str ((char *)data, len);
	std::istringstream  is(str);


	return ConfigureSerial(is, reset);
	}

void CXPhal::ConfigureByte(unsigned char byte)
	{
	for(int i=0; i<8; i++)
		{
		WriteBit(XPH_CDATA, byte&0x01);  
		byte >>= 1;
		WriteBit(XPH_CCLOCK, 1);
		WriteBit(XPH_CCLOCK, 0);
		}
	}

int CXPhal::ConfigureSerial(std::istream &is, bool reset)
	{
	unsigned char bs[XPH_SIZE_BUFFER];;
	unsigned long len = XPH_SIZE_BUFFER;
	SBitstreamInfo info;
	int i, res;

	if ((res=Reset(reset, RESET_LOGIC_SERIAL))!=RES_OK)
		return res;

	if ((res=CBitstream::ParseBitstream(is, bs, &len, &info))!=RES_OK)
		return res;

	WriteBit(XPH_CCLOCK, 0);
	WriteBit(XPHPP_HOST_TO_XPORT);
	for (i=0; i<(int)len; i++)
		ConfigureByte(bs[i]);

	for (i=0; i<32; i++)
		{
		WriteBit(XPH_CCLOCK, 1);
		WriteBit(XPH_CCLOCK, 0);
		}

	Delay(100);

	return RES_OK;
	}

int CXPhal::Reset(bool resetCPU, EXPresetMode mode)
	{
	bool connected, power;

	Detect(&connected, &power);
	if (!connected)
		{
		m_debug.printf(0, "Xport is not detected.  Please check that it is plugged into PC through Cport.\n");
		return RES_NOT_CONNECTED;
		}

	WriteBit(XPHPP_HOST_TO_XPORT);
	if (mode!=RESET_CPU_ONLY)
		SetSerial();

	if (resetCPU)
		{
		if (!power)
			{
			m_debug.printf(0, "Please switch Game Boy on.\n");
			WaitDetectPower(true);
			m_debug.printf(1, "On\n");
			}
		else if (m_methodReset==RESET_POWER_TOGGLE)	// power==true
			{
			m_debug.printf(0, "Please power Game Boy off and back on again.\n");
			WaitDetectPower(false);
			m_debug.printf(1, "Off...");
			WaitDetectPower(true);
			m_debug.printf(1, "On\n");
			}
		else if (m_methodReset==RESET_AUTO)
			{
			if (mode==RESET_CPU_ONLY)
				{
				// reset logic -- load PC0
				SetPC0();
				PulseReset();
				Delay(50);
				}
			else
				{
				// reset logic 
				SetSerial();
				PulseReset();
				}
			// reset CPU
			SetCPUReset();
			PulseReset();
			}
		Delay(500);
		}
	else if (!power)
		return RES_NO_POWER;

	if (mode==RESET_LOGIC_SERIAL)
		{
		SetSerial();
		PulseReset();
		m_configx = true;
		}
	else if (mode==RESET_LOGIC_PC0)
		{
		SetPC0();
		PulseReset();
		}
	else if (mode==RESET_LOGIC_PC1)
		{
		SetPC0();
		WriteBit(XPH_CD1, 0);
		PulseReset();
		m_configx = true;
		}

	Delay(500);

	return RES_OK;
	}

int CXPhal::SetProperty(EXPproperty property, unsigned long val)
	{
	// pass to cport
	if (m_pcport)
		m_pcport->SetProperty(property, val);

	switch(property)
		{
		case PROP_LEVEL_DEBUG:
			m_debug.SetLevel(val);
			break;

		case PROP_METHOD_RESET:
			m_methodReset = (EXPresetMethod)val;
			break;

		case PROP_PPORT_DELAY_READ:
			m_delayRead = val*10 + XPH_DELAY_READ;
			break;

		case PROP_PPORT_DELAY_WRITE:
			m_delayWrite = val*10 + XPH_DELAY_WRITE;
			break;

		case PROP_PPORT_ADDRESS:
			m_ppport->SetAddress((unsigned short)val);
			break;

		case PROP_PPORT_NUM:
			m_port = (unsigned char)val;
			break;

		case PROP_READ_CONSISTENCY:
			m_readConsistency = val;
			break;

		case PROP_TEST:
			m_test = val;
			break;

		default:
			return RES_PROPERTY_NOT_SUPPORTED;
		}

	return RES_OK;
	}

int CXPhal::GetProperty(EXPproperty property, unsigned long *pval)
	{
	// no properties supported here, so pass to cport
	if (m_pcport)
		return m_pcport->GetProperty(property, pval);
	}

