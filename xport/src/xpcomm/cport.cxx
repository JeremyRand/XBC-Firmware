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
#include <curses.h>
#include <termios.h>
#include "cport.h"
#include "objects.h"

HINSTANCE CXPcport::m_dllHandle;


CXPcport::CXPcport(CXPhal *phal, unsigned long timeout) : m_hostInQueue(XPCP_CQUEUE_SIZE)
	{
	m_phal = phal;
	m_timeout = timeout;

	memset(m_fileList, 0, sizeof(m_fileList));
	m_rpcBufSize = XPCP_RPC_BUF_SIZE;
	m_rpcBuf = new unsigned char[XPCP_RPC_BUF_SIZE];
	}

CXPcport::~CXPcport()
	{
	m_phal->WriteBit(XPHPP_HOST_TO_XPORT);
	m_phal->SetNC0(); // disable host control
	m_phal->WriteBit(XPH_CRESET, 1);
	m_phal->Delay(1);
	m_phal->WriteBit(XPH_CRESET, 0);

	RPCclear();
	delete [] m_rpcBuf;
	}

void CXPcport::SetHostControl(bool val)
	{
	m_phal->WriteBit(XPHPP_HOST_TO_XPORT);
	m_phal->SetNC0(); 
	m_phal->WriteBit(XPH_CD0, 1); // reset
	m_phal->WriteBit(XPH_CD1, val); // set host control
	m_phal->WriteBit(XPH_CRESET, 1);
	m_phal->Delay(1);
	m_phal->WriteBit(XPH_CRESET, 0);
	}

int CXPcport::Reset()
	{
	int res;

	if ((res=m_phal->Reset(false))==RES_NOT_CONNECTED)
		return res;

	return RES_OK;
	}

int CXPcport::SetTimeout(unsigned long timeout)
	{
	m_timeout = timeout;
	return RES_OK;
	}

int CXPcport::Write(unsigned char *data, unsigned long len, bool yield)
	{
	unsigned long i;
	unsigned long t0, t1;
	unsigned char c;

	m_phal->WriteBit(XPHPP_HOST_TO_XPORT);
	m_phal->WriteBit(XPH_CDIR, 1);

	t0 = t1 = Cplatform::GetClock();
	for (i=0; i<len; i++)
		{
        while (m_phal->ReadBit(XPH_CREADY))
			{
			if (yield)
				Cplatform::YieldTask();
			t1 = Cplatform::GetClock();
			if (t1-t0>m_timeout || DataAvail())
				return RES_TIMEOUT;
			m_phal->WriteBit(XPHPP_HOST_TO_XPORT);
			m_phal->WriteBit(XPH_CDIR, 1);
			}
		t0 = t1;
		c = data[i];
		m_phal->Write(PPORT_DATA, c);
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		c >>= 4;
		m_phal->Write(PPORT_DATA, c);		
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		}
	return RES_OK;
	}

int CXPcport::WriteChar(unsigned char c)
	{
	return Write(&c, 1);
	}

int CXPcport::Read(unsigned char *data, unsigned long len, bool yield)
	{
	unsigned long i;
	unsigned long t0, t1;
	unsigned char c;

	m_phal->WriteBit(XPHPP_XPORT_TO_HOST);
	m_phal->WriteBit(XPH_CDIR, 0);

	t0 = t1 = Cplatform::GetClock();
	for (i=0; i<len; i++)
		{
        while (m_phal->ReadBit(XPH_CREADY))
			{
			if (yield)
				Cplatform::YieldTask();
			t1 = Cplatform::GetClock();
			if (t1-t0 > m_timeout)
				return RES_TIMEOUT;
			}
		t0 = t1;
		c = m_phal->Read(PPORT_DATA)&0x0f;
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		c |= m_phal->Read(PPORT_DATA)<<4;
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		data[i] = c;
		}
	return RES_OK;
	}

int CXPcport::ReadFill(unsigned char *data, unsigned long len, long idle, unsigned short *checksum)
	{
	unsigned long i;
	unsigned long t0;
	unsigned char c;
	unsigned short sum;

	m_phal->WriteBit(XPHPP_XPORT_TO_HOST);
	m_phal->WriteBit(XPH_CDIR, 0);

	t0 = 0;
	for (i=0, sum=0; i<len; i++)
		{
        while (m_phal->ReadBit(XPH_CREADY))
			{
			t0++;
			if (t0>idle)
				return i;
			}
		t0 = 0;
		c = m_phal->Read(PPORT_DATA)&0x0f;
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		c |= m_phal->Read(PPORT_DATA)<<4;
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		data[i] = c;
		sum += c;
		}

	if (checksum)
		*checksum = sum;

	return i;
	}

int CXPcport::WriteFill(long idle)
	{
	unsigned long t0, bytes;
	unsigned char c;
	
	m_phal->WriteBit(XPHPP_HOST_TO_XPORT);
	m_phal->WriteBit(XPH_CDIR, 1);

	t0 = 0;
	bytes = 0;
	while(1)
		{
        while (m_phal->ReadBit(XPH_CREADY))
			{
			t0++;
			if (t0>idle)
				return bytes;			
			}
		t0 = 0;
		if (m_hostInQueue.DeQueue(&c, 1)==0)
			return bytes;
		m_phal->Write(PPORT_DATA, c);
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		c >>= 4;
		m_phal->Write(PPORT_DATA, c);		
		m_phal->WriteBit(XPH_CSTROBE, 1);
		m_phal->WriteBit(XPH_CSTROBE, 0);
		bytes++;
		}
	}

int CXPcport::ReadChar(unsigned char *c)
	{
	return Read(c, 1);
	}

bool CXPcport::DataAvail()
	{
	m_phal->WriteBit(XPHPP_XPORT_TO_HOST);
	m_phal->WriteBit(XPH_CDIR, 0);
	return (!m_phal->ReadBit(XPH_CREADY));
	}

int CXPcport::TermLoop(bool cb)
	{
	unsigned long t0 = 0;
	bool active = false;
	bool stranded = false;
	bool power = true;
	bool cont = true;
	int ci, co;
	unsigned char c, model;
	unsigned long long hist = 0;
	struct termios  tty;
	unsigned char buf[XPCP_CQUEUE_SIZE];
	int len;

	initscr();
	intrflush(stdscr, FALSE);
	reset_shell_mode();
	tcgetattr(0, &tty);
	tty.c_lflag &= ~ECHO;
	tcsetattr(0, TCSANOW, &tty);
	nonl();
	noecho();
	if (cb)
		cbreak();

	nodelay(stdscr, 1);
	while (getch()!=XPCP_ERR);
	SetHostControl(true);
	SetTimeout(0);

	m_debug.printf(1, "Xpcomm Cport console mode ready.\r\nPress ctrl-c to exit.\r\n\n");

	while(cont)
		{
		if (CXPhal::m_break)
			{
			if (cb)
				break;
			CXPhal::m_break = false;

			// pass ctrl-c thru
			c = 0x03; // ctrl-c
			m_hostInQueue.EnQueue(&c, 1);
			}

		if (power)
			{
			// only accept keyboard input if queue is empty
			if (!m_hostInQueue.Full())
				{
				for (len=0; (ci=getch())!=XPCP_ERR; len++)
					{
					buf[len] = (unsigned char)ci;
					hist <<= 8;
					hist |= buf[len];
					// look for $k#6b (quit) packet from gdb
					if ((hist&0xffffffffffll)==0x246b233662ll)
						cont = false;
					}
				if (len)
					m_hostInQueue.EnQueue(buf, len);
				}

			// try to empty queue
			if (m_hostInQueue.Full() && WriteFill(XPCP_FILL_IDLE))
				active = true;

			// read from Cport
			if (len=ReadFill(buf, XPCP_CQUEUE_SIZE-1, XPCP_FILL_IDLE))
				{
				buf[len] = '\0';
				fputs((char *)buf, stdout);
				fflush(stdout);
				active = true;
				}
				
			// CPU throttling logic
			if (active)
				{
				t0 = 0;
				active = false;
				}
			else if (t0>XPCP_YIELD_IDLE)
				{
				Cplatform::YieldTask();
				model = m_phal->GetModel(false, true);
				if (model==0 || model==0xff)
					{
					m_debug.printf(2, "--> Power off\r\n");
					power = false;
					}
				}
			else
				t0++;
			}
		else
			{
			// actively monitor power status of xport to ensure that slot 0 is always selected
			model = m_phal->GetModel(false, true);
			if (model!=0 && model!=0xff)
				{
				m_debug.printf(2, "--> Power on\r\n");
				m_phal->Delay(100);
				SetHostControl(true);
				power = true;
				}
			else
				Cplatform::YieldTask();
			}
		}
	endwin();
	return RES_OK;
	}

int CXPcport::SetProperty(EXPproperty property, unsigned long val)
	{
	switch(property)
		{
		case PROP_LEVEL_DEBUG:
			m_debug.SetLevel(val);
			break;

		default:
			return RES_PROPERTY_NOT_SUPPORTED;
		}

	return RES_OK;
	}

int CXPcport::GetProperty(EXPproperty property, unsigned long *pval)
	{
	return RES_PROPERTY_NOT_SUPPORTED;
	}


int CXPcport::RPCLoop()
	{
	unsigned long t0 = 0;
	bool active = false;
	bool stranded = false;
	bool power = true;
	int ci, co;
	unsigned char c, model;
	int len;
	unsigned char state = 0;
	unsigned long startCode = 0;
	unsigned char *cStartCode = (unsigned char *)&startCode;
	unsigned long rpcLen;
	unsigned char *cRpcLen = (unsigned char *)&rpcLen;
	unsigned short checksum;
	unsigned char *cChecksum = (unsigned char *)&checksum;
	unsigned short runningChecksum;
	unsigned short partialChecksum;
	int count;

	SetHostControl(true);
	SetTimeout(0);

	m_debug.printf(1, "Xpcomm Cport RPC mode ready.\r\nPress ctrl-c to exit.\r\n\n");

	while(1)
		{
		if (CXPhal::m_break)
			break;

		if (power)
			{
			// try to empty queue
			if (m_hostInQueue.Full() && WriteFill(XPCP_FILL_IDLE))
				active = true;

			// look for startcode
			if (state==0 && ReadFill(&c, 1, XPCP_FILL_IDLE)==1)
				{
				startCode >>= 8;
				cStartCode[3] = c;
				if (startCode==XPCP_RPC_STARTCODE)
					{
					// clear output queue of pending responses 
					m_hostInQueue.Clear();
					count = 0;
					runningChecksum = 0;
					rpcLen = 0;
					state = 1;
					}
				active = true;
				}

			// get length
			else if (state==1 && ReadFill(&c, 1, XPCP_FILL_IDLE)==1)
				{
				if (count<3)
					{
					cRpcLen[count] = c;
					runningChecksum += c;
					count++;
					}
				// length checksum
				else if (count==3)
					{
					cRpcLen[3] = 0;
					if ((runningChecksum&0x000000ff)!=c)
						{
						m_debug.printf(1, "\n\nERROR: RPC length checksum\n\n");
						state = 0;
						}
					else
						{
						// if buffer is too small, allocate bigger buffer
						if(rpcLen>m_rpcBufSize)
							{
							delete [] m_rpcBuf;
							m_rpcBufSize = rpcLen + 0x10000;
							m_rpcBuf = new unsigned char[m_rpcBufSize];
							}
						runningChecksum = 0;
						count = 0;
						state = 2;
						}
					}
				active = true;
				}

			// get buffer contents
			else if (state==2 && (len=ReadFill(m_rpcBuf+count, rpcLen-count, XPCP_FILL_IDLE, &partialChecksum)))
				{
				count += len;
				runningChecksum += partialChecksum;
				if (count==rpcLen)
					{
					count = 0;
					state = 3;
					}
				active = true;
				}

			// get checksum, compare, and dispatch
			else if (state==3 && ReadFill(&c, 1, XPCP_FILL_IDLE)==1)
				{
				cChecksum[count] = c;
				if (count==1)
					{
					runningChecksum += rpcLen;
					if (runningChecksum==checksum)
						RPCDispatch(m_rpcBuf, rpcLen);
					else
						m_debug.printf(1, "\n\nERROR: RPC data checksum\n\n");
					state = 0;
					}
				else
					count++;
				active = true;
				}
				

			// CPU throttling logic
			if (active)
				{
				t0 = 0;
				active = false;
				}
			else if (t0>XPCP_YIELD_IDLE)
				{
				Cplatform::YieldTask();
				model = m_phal->GetModel(false, true);
				if (model==0 || model==0xff)
					{
					m_debug.printf(2, "--> Power off\n");
					m_hostInQueue.Clear();
					RPCclear();
					state = 0;
					power = false;
					}
				}
			else
				t0++;
			}
		else
			{
			// actively monitor power status of xport to ensure that slot 0 is always selected
			model = m_phal->GetModel(false, true);
			if (model!=0 && model!=0xff)
				{
				m_debug.printf(2, "--> Power on\n");
				m_phal->Delay(100);
				SetHostControl(true);
				power = true;
				}
			else
				Cplatform::YieldTask();
			}
		}

	return RES_OK;
	}

int CXPcport::RPCDispatch(unsigned char *buf, unsigned long rpcLen)
	{
	unsigned char index = buf[0];

	buf++;

	switch (index)
		{
		case XPCP_RPC_PRINT:
			RPCPrint(buf);
			break;

		case XPCP_RPC_FOPEN:
			RPCFopen(buf);
			break;

		case XPCP_RPC_FCLOSE:
			RPCFclose(buf);
			break;

		case XPCP_RPC_FWRITE:
			RPCFwrite(buf, rpcLen-1);
			break;

		//case RPC_FOPEN:
		}

	return RES_OK;
	}

int CXPcport::RPCFixedResponse(unsigned char *data, unsigned long len)
	{
	unsigned long i;
	unsigned short checksum;

	for (i=0, checksum=0; i<len; i++)
		{
		checksum += data[i];
		m_hostInQueue.EnQueue(data + i, 1);
		}

	checksum += len;

	// send checksum
	m_hostInQueue.EnQueue((unsigned char *)&checksum, sizeof(checksum));

	return RES_OK;
	}

int CXPcport::RPCPrint(unsigned char *stack)
	{
	m_debug.printf(0, (char *)stack);

	return RES_OK;
	}

int CXPcport::RPCFopen(unsigned char *stack)
	{
	unsigned char result;

	for (result=0; result<0xff && m_fileList[result]!=0; result++);
	if (result==0xff) // fails
		result = 0;
	else
		{
		m_fileList[result] = fopen((char *)stack, "wb");
		if (m_fileList[result]==0)
			result = 0;
		else
			result++;
		}

	m_debug.printf(2, "--> Fopen file \"%s\" fd %d\n", (char *)stack, result);
	
	return RPCFixedResponse(&result, sizeof(result));
	}

int CXPcport::RPCFclose(unsigned char *stack)
	{
	unsigned char fd = *stack;

	if (fd==0)
		return RES_FAIL;

	fd--;

	if (m_fileList[fd])
		{
		fclose(m_fileList[fd]);
		m_fileList[fd] = 0;
		m_debug.printf(2, "--> Fclose fd %d\n", fd+1);
		}
	else
		m_debug.printf(2, "--> Fclose invalid fd %d\n", fd+1);

	return RES_OK;
	}

int CXPcport::RPCFwrite(unsigned char *stack, unsigned long len)
	{
	unsigned long result = 0;
	unsigned char fd = *stack;

	fd--;

	if (m_fileList[fd])
		{
		result = fwrite(stack+1, 1, len-1, m_fileList[fd]);
		m_debug.printf(2, "--> Fwrite fd %d result %d\n", fd+1, result);
		}
	else
		m_debug.printf(2, "--> Fwrite invalid fd %d\n", fd+1);

	return RPCFixedResponse((unsigned char *)&result, sizeof(result));
	}

int CXPcport::RPCclear()
	{
	unsigned char fd;
	for (fd=0; fd<0xff; fd++)
		{
		if (m_fileList[fd])
			{
			fclose(m_fileList[fd]);
			m_fileList[fd] = 0;
			}
		}

	return RES_OK;
	}

