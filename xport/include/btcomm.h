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


#ifndef _BTCOMM_H
#define _BTCOMM_H

#define DBC_BAUD_9600		2604
#define DBC_BAUD_19K		1302
#define DBC_BAUD_38K		651
#define DBC_BAUD_57K		434
#define DBC_BAUD_115K		217
#define DBC_BAUD_921K		26
#define DBC_BAUD_BASE		((unsigned long)24998400)

class CBluetoothComm
	{
public:
	CBluetoothComm(unsigned long base);
	int MasterConnect(char *serial, unsigned short timeoutMs=30000, unsigned short baud=DBC_BAUD_921K);
	int SlaveConnect(unsigned short baud=DBC_BAUD_921K);

	virtual int Reset(unsigned short baud=DBC_BAUD_921K);
	virtual unsigned int ReadData(char *buf, unsigned int len, unsigned short timeoutMs=500);
	virtual unsigned int ReadResponse(char *response, unsigned int len, unsigned short timeoutMs0=100, unsigned short timeoutMs1=30000);
	virtual unsigned int WriteData(const char *data, unsigned int len);
	virtual unsigned int WriteString(const char *string);

	int GetName(char *name, int size);  // Get the "friendly name" -- one that is queried on the network
	int SetName(const char *name);      // Set the "friendly name".  Once set, it is nonvolatile.
	int GetDeviceAddress(char *address, int size);  // get the physical address of your radio

	static void DelayMs(unsigned short delay);

	inline unsigned short ReceiveFifoCount()
		{
		return *m_regReceiveCount;
		}

protected:
	inline char ReadByte()
		{
		return (char)((*m_regData)&0xff);
		}
	inline unsigned short ReadBusy()
		{
		return *m_regStatus&0x0001;
		}
	inline char WaitReadByte()
		{
		while(ReadBusy());
		return ReadByte();
		}
	inline void WriteByte(char byte)
		{
		*m_regData = (unsigned short)byte;
		}
	inline unsigned short WriteBusy()
		{
		return *m_regStatus&0x0002;
		}
	inline void WaitWriteByte(char byte)
		{
		while(WriteBusy());
		WriteByte(byte);
		}

	volatile unsigned short *m_regStatus;
	volatile unsigned short *m_regData;
	volatile unsigned short *m_regBaud;
	volatile unsigned short *m_regReceiveCount;
	bool m_initialized;
	};

#endif
