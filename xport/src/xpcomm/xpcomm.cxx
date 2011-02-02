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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include "configuration.h"
#include "xpcomm.h"
#include "debug.h"
#include "cport.h"
#include "objects.h"
#include <curses.h>

// **** static definitions, declarations

CXPdebug g_debug;
bool g_timer = false;
bool g_serial = false;
bool g_startup = false;
bool g_console = false;
bool g_rpc = false;
bool g_error = false;

static void stringToupper(char *string)
	{
	while (*string)
		{
		if (*string>='a' && *string<='z')
			*string -= 'a'-'A';
		string++;
		}
	}

static int getnumarg(char *string)
	{
	int result = 0;

	if (string[1]=='x' || string[1]=='X')
		sscanf(string, "%x", &result);
	else
		sscanf(string, "%d", &result);

	return result;
	}

static bool iswhite(char c)
	{
	return c==' ' || c=='\t' ||  c=='\r' || c=='\n';
	}

static char *nonwhite(char *str)
	{
	while (str && *str && iswhite(*str))
		str++;
	return str;
	}

static char *white(char *str)
	{
	while (str && *str && !iswhite(*str))
		str++;
	return str;
	}

static EXPcommand detect(char *filename)
	{
	char c[4];
	std::ifstream *pifs;
	EXPcommand res;
	void *dir;

	pifs = new std::ifstream(filename, std::ios::binary);//|ios::nocreate);
	
	pifs->read(c, 4); 
	if (pifs->fail() || pifs->eof() || pifs->gcount()!=4) 
		{
		pifs->close();
		delete pifs;
		dir = Cplatform::OpenDir(filename);
		if (dir)
			{
			Cplatform::CloseDir(dir);
			return COMM_CONFIG_CHOOSE_PROGRAM;
			}
		else
			return (EXPcommand)RES_FAIL;
		}

	// look for inital jump instruction
	if ((c[0]==(char)0x2e || c[0]==(char)0x37) && c[1]==(char)0x00)
		res = COMM_FLASH_PROGRAM;
	// look for lack of any header whatsoever
	else if (c[0]==(char)0x00 && c[1]==(char)0x00 && c[2]==(char)0x00 && c[3]==(char)0x00)
		res = COMM_FLASH_PROGRAM;
	// look for S-record
	else if (c[0]=='S')
		res = COMM_FLASH_PROGRAM;
	// assume it must be a bitstream
	else
		res = COMM_CONFIG_PROGRAM;

	pifs->close();
	delete pifs;
	return res;
	}

static void PrintVersion()
	{
	g_debug.printf(0, "\nXPCOMM version %s (%s)\n", XPC_VERSION, __DATE__);
	g_debug.printf(0, "Obtain the most recent version of Xpcomm at www.charmedlabs.com\n");
	g_debug.printf(0, "Send feedback and bug reports to support@charmedlabs.com\n");				
	}

static int parseargs(int argc, char *argv[], CXPexec *pexec)
	{
	int i;
	unsigned long loop = 1;
	EXPcommand fileDetect;

	for (i=1; i<argc; i++)
		{
		fileDetect = (EXPcommand)0;
		if (argv[i][0]=='-' || argv[i][0]=='/')
			{
			argv[i]++;
			stringToupper(argv[i]);
			if (!strcmp(argv[i], "VERSION"))
				{
				PrintVersion();
				if (argc==2)
					exit(0);
				}
			else if (!strcmp(argv[i], "C"))
				{
				g_serial = true;
				i++;
				if (i<argc && (fileDetect=detect(argv[i]))>=0)
					pexec->AddCommand(COMM_CONFIG, argv[i], 0, 0);
				}
			else if (!strcmp(argv[i], "PF"))
				{
				i++;
				if (i<argc && (fileDetect=detect(argv[i]))>=0)
					pexec->AddCommand(COMM_FLASH_PROGRAM, argv[i], 0, 0);
				}
			else if (!strcmp(argv[i], "PFA")) 
			// xpcomm -PFA program address, e.g. xpcomm -PFA icxbc.bin 0x20000
				{
				i+=2; // i++ changed to i+=2
				if (i<argc && (fileDetect=detect(argv[i-1]))>=0) // i changed to i-1
					pexec->AddCommand(COMM_FLASH_PROGRAM, argv[i-1], getnumarg(argv[i]), 0); // i changed to i-1, second argument added
				}
			else if (!strcmp(argv[i], "VF"))
				{
				i++;
				if (i<argc && (fileDetect=detect(argv[i]))>=0)
					pexec->AddCommand(COMM_FLASH_VERIFY, argv[i], 0, 0);
				}
			else if (!strcmp(argv[i], "PVF"))
				{
				i++;
				if (i<argc && (fileDetect=detect(argv[i]))>=0)
					{
					pexec->AddCommand(COMM_FLASH_PROGRAM, argv[i], 0, 0);
					pexec->AddCommand(COMM_FLASH_VERIFY, argv[i], 0, 0);
					}
				}
			else if (!strcmp(argv[i], "RF"))
				{
				i+=2;
				if (i<argc)
					pexec->AddCommand(COMM_FLASH_READ, argv[i-1], getnumarg(argv[i]), 0);
				}
			else if (!strcmp(argv[i], "PC"))
				{
				i++;
				if (i<argc)
					{
					if ((fileDetect=detect(argv[i]))!=COMM_CONFIG_CHOOSE_PROGRAM)
						pexec->AddCommand(COMM_CONFIG_PROGRAM, argv[i], 0, 0);
					else
						pexec->AddCommand(COMM_CONFIG_CHOOSE_PROGRAM, argv[i], 0, 0);
					}
				}
			else if (!strcmp(argv[i], "VC"))
				{
				i++;
				if (i<argc)
					{
					if ((fileDetect=detect(argv[i]))!=COMM_CONFIG_CHOOSE_PROGRAM)
						pexec->AddCommand(COMM_CONFIG_VERIFY, argv[i], 0, 0);
					else
						pexec->AddCommand(COMM_CONFIG_CHOOSE_VERIFY, argv[i], 0, 0);
					}
				}
			else if (!strcmp(argv[i], "PVC"))
				{
				i++;
				if (i<argc)
					{
					if ((fileDetect=detect(argv[i]))!=COMM_CONFIG_CHOOSE_PROGRAM)
						{
						pexec->AddCommand(COMM_CONFIG_PROGRAM, argv[i], 0, 0);
						pexec->AddCommand(COMM_CONFIG_VERIFY, argv[i], 0, 0);
						}
					else
						{
						pexec->AddCommand(COMM_CONFIG_CHOOSE_PROGRAM, argv[i], 0, 0);
						pexec->AddCommand(COMM_CONFIG_CHOOSE_VERIFY, argv[i], 0, 0);
						}
					}
				}
			else if (!strcmp(argv[i], "I"))
				pexec->AddCommand(COMM_INFO, 0, 0, 0);
			else if (!strcmp(argv[i], "U"))
				{
				g_serial = true;
				pexec->AddCommand(COMM_UPDATE, 0, 0, 0);
				}
			else if (!strcmp(argv[i], "RESET") || !strcmp(argv[i], "EXECUTE"))
				pexec->AddCommand(COMM_CPU_RESET, 0, 0, 0);
			else if (!strcmp(argv[i], "LOOP"))
				{
				i++;
				if (i<argc)
					loop = getnumarg(argv[i]);
				if (loop<1)
					loop = 1;
				}
			else if (!strcmp(argv[i], "PAUSE"))
				{
				i++;
				if (i<argc)
					pexec->AddCommand(COMM_PAUSE, 0, getnumarg(argv[i]), 0);
				}
			else if (!strcmp(argv[i], "RESETAUTO"))
				pexec->AddProperty(PROP_METHOD_RESET, RESET_AUTO);
			else if (!strcmp(argv[i], "DELAY"))
				{
				i++;
				if (i<argc)
					{
					pexec->AddProperty(PROP_PPORT_DELAY_READ, getnumarg(argv[i]));
					pexec->AddProperty(PROP_PPORT_DELAY_WRITE, getnumarg(argv[i]));
					}
				}
			else if (!strcmp(argv[i], "RC"))
				{
				i++;
				if (i<argc)
					pexec->AddProperty(PROP_READ_CONSISTENCY, getnumarg(argv[i]));
				}
			else if (!strcmp(argv[i], "TEST"))
				{
				i++;
				if (i<argc)
					pexec->AddProperty(PROP_TEST, getnumarg(argv[i]));
				}
			else if (!strcmp(argv[i], "WRITEDELAY"))
				{
				i++;
				if (i<argc)
					pexec->AddProperty(PROP_PPORT_DELAY_WRITE, getnumarg(argv[i]));
				}
			else if (!strcmp(argv[i], "READDELAY"))
				{
				i++;
				if (i<argc)
					pexec->AddProperty(PROP_PPORT_DELAY_READ, getnumarg(argv[i]));
				}
			else if (!strcmp(argv[i], "PORTADDR"))
				{
				i++;
				if (i<argc)
					pexec->AddProperty(PROP_PPORT_ADDRESS, getnumarg(argv[i]));
				}
			else if (!strcmp(argv[i], "PORTNUM"))
				{
				i++;
				if (i<argc)
					pexec->AddProperty(PROP_PPORT_NUM, getnumarg(argv[i])-1);
				}
			else if (!strcmp(argv[i], "DEBUG"))
				{
				i++;
				if (i<argc)
					pexec->AddProperty(PROP_LEVEL_DEBUG, getnumarg(argv[i]));
				}
			else if (!strcmp(argv[i], "STARTUP"))
				g_startup = true;
			else if (!strcmp(argv[i], "TIME"))
				g_timer = true;
			else if (!strcmp(argv[i], "CONSOLE"))
				g_console = true;
			else if (!strcmp(argv[i], "RPC"))
				g_rpc = true;
 			else 
				{
				g_debug.printf(0, "Invalid argument \"%s\"\n", argv[i]);
				g_error = true;
				break;
				}
			}
		else // argument doesn't begin with '-' so treat it as a file
			{
			fileDetect = detect(argv[i]);
			if (fileDetect>=0)
				pexec->AddCommand(fileDetect, argv[i], 0, 0);
			}

		if (fileDetect<0)
			{
			g_debug.printf(0, "File \"%s\" cannot be opened or read\n", argv[i]);
			g_error = true;
			break;
			}
		if (i>=argc)
			{
			g_debug.printf(0, "Missing argument\n");
			g_error = true;
			break;
			}
		}
	return loop;
	}

static int xppmain(int argc, char *argv[])
	{
	int loop, res;
	unsigned long t0;
	CXPexec exec;
	int pid;

#ifdef LINUX
	pid = Cplatform::FindProcess("xpcomm");
#else
	pid = Cplatform::FindProcess("xpcomm.exe");
#endif

	loop = parseargs(argc, argv, &exec);

	if (pid)
		{
		if (exec.CommandsNum() ||  g_console)
			{
			char reply[128];
			g_debug.printf(0, "Another Xpcomm process is running.  Would you like to kill it (y/n)? ");
			reply[0] = 0;
			fgets(reply,128,stdin);
			if (reply[0]=='y' || reply[0]=='Y')
				Cplatform::KillProcess(pid);
			else
				return -1;
			}
		else
			Cplatform::KillProcess(pid);
		}

	if (argc>1 && g_error)
		return 0;

	if (g_timer)
		t0 = Cplatform::GetClock();
	if (exec.CommandsNum())
		{
		try 
			{
			exec.ExecuteCommands(loop, !g_startup, g_serial);
			}
		catch(int exception)
			{
			if (exception==EXCEPT_CTRL_C)
				g_debug.printf(0, "\nQuit\n");
			else
				g_debug.printf(0, "\nUnknown exception 0x%x\n");
			}
		}
	else
		{
		CXPhal *phal = new CXPhal();
		exec.SetProperties((IXPproperty *)phal);
		if ((res=phal->Initialize())!=RES_OK)
			{
			g_debug.printf(0, "Failed to initialize (Result=0x%x)\n", res);
			return res;
			}
		// some properties will not stick until after initialization
		exec.SetProperties((IXPproperty *)phal);
		if (!g_startup)
			{
			if ((res=phal->m_pcport->Reset())==RES_OK)
				{
				if (g_rpc)
					phal->m_pcport->RPCLoop();
				else
					phal->m_pcport->TermLoop(g_console);
				}
			}
		delete phal;
		}

	if (g_timer)
		{
		g_debug.printf(0, "Time: %f seconds\n", (Cplatform::GetClock()-t0)/1000.0);	
		//getchar();
		}
	return 0;
	}

int main(int argc, char *argv[], char *envp[])
	{
	int i, j;
	char *xppargv[256];
	char *args;
	char temp[128];

	memcpy((void *)xppargv, (void *)argv, argc*sizeof(char *));

	// look for environment variable and pack it into argc, argv if it exists
	for (i=0; envp[i]; i++)
		{
		for (j=0; envp[i][j] && envp[i][j]!='='; j++)
			temp[j] = envp[i][j];
		temp[j] = '\0';
		if (!strcmp(temp, XPC_ENV_VARIABLE))
			{
			args = &envp[i][j+1];	
			while (1)
				{
				args = nonwhite(args);
				if (*args=='\0')
					break;
				xppargv[argc++] = args;
				args = white(args);
				if (*args=='\0')
					break;
				args[0] = '\0';
				args++;
				}
			break;
			}
		}
	return xppmain(argc, xppargv);	
	}


// **** CXPexec methods

CXPexec::CXPexec()
	{
	memset(commands, 0, sizeof(commands));
	memset(properties, 0, sizeof(properties));
	commandsNum = 0;
	propertiesNum = 0;
	m_pconfig = 0;
	m_methodReset = RESET_POWER_TOGGLE;
	m_finalExec = false;
	m_initReset = true;
	}

CXPexec::~CXPexec()
	{
	if (m_pconfig)
		delete m_pconfig;
	}

int CXPexec::AddCommand(EXPcommand command, char *sarg, unsigned long iarg1, unsigned long iarg2)
	{
	if (commandsNum<XPC_COMMAND_MAX)
		{
		commands[commandsNum].command = command;
		commands[commandsNum].sarg = sarg;
		commands[commandsNum].iarg1 = iarg1;
		commands[commandsNum++].iarg2 = iarg2;
		}
	else
		return RES_FAIL;

	return RES_OK;
	}

int CXPexec::AddProperty(EXPproperty property, unsigned long val)
	{
	if (propertiesNum<XPC_PROPERTY_MAX)
		{
		properties[propertiesNum].property = property;
		properties[propertiesNum++].val = val;
		if (property==PROP_METHOD_RESET)
			m_methodReset = (EXPresetMethod)val;
		}
	else
		return RES_FAIL;

	return RES_OK;
	}

int CXPexec::ExecuteCommands(unsigned long iter, bool reset, bool serial)
	{
	unsigned long i, j;
	int res = RES_OK;
	unsigned char model;
	
	m_pconfig = new CXPconfiguration();
	SetProperties((IXPproperty *)m_pconfig);
	if ((res=m_pconfig->Initialize(reset, serial))!=RES_OK)
		{
		m_debug.printf(0, "Failed to initialize (Result=0x%x)\n", res);
		return res;
		}
	
	// some properties will not stick until after initialization
	SetProperties((IXPproperty *)m_pconfig);
	if(reset && !m_pconfig->Supported())
		{
		m_debug.printf(0, "This Xport model (%s) is not supported in this version of Xpcomm.\n", m_pconfig->Name());
		m_debug.printf(0, "Please obtain the latest version at www.CharmedLabs.com\n");
		return RES_FAIL;
		}
	
	for (i=0; i<iter; i++)
		{
		for (j=0; j<commandsNum; j++)
			{
			if ((res=ExecuteCommand(&commands[j]))!=RES_OK)
				break;
			}
		if (res!=RES_OK)
			break;
		}	
			
	if (m_finalExec)
		{
		if (m_methodReset==RESET_AUTO)
			m_pconfig->Reset();
		else
			m_debug.printf(0, "Turn Game Boy off and back on again to execute new code or configuration.\n");
		}
	
	delete m_pconfig;
	m_pconfig = NULL;
			
	return res;
	}

int CXPexec::SetProperties(IXPproperty *pclass)
	{
	unsigned long i;
	int res = RES_OK;
	
	for (i=0; i<propertiesNum; i++)
		res |= pclass->SetProperty(properties[i].property, properties[i].val);
		
	return res;
	}

void CXPexec::PrintInfo()
	{
	char program[16];
	unsigned short code;

	m_pconfig->GetCode(&code);

	PrintVersion();
	m_debug.printf(0, "Xport model number:  %s\n", m_pconfig->Name());
	m_debug.printf(0, "Xport version:       %s\n", m_pconfig->Version());
	m_debug.printf(0, "Description:         %s\n", m_pconfig->Description());
	if (m_pconfig->GetName(program)!=RES_OK)
		m_debug.printf(0, "Query error\n");
	m_debug.printf(0, "Flash code:          %x\n", code);
	m_debug.printf(0, "Flash program:       %s\n", program);
	m_pconfig->PrintInfo();
	m_debug.printf(0, "Checking for updates...\n");
	m_pconfig->Update();
	m_debug.printf(0, "Verifying configuration integrity...\n");
	if (m_pconfig->Verify(0)!=RES_OK || m_pconfig->Verify(1)!=RES_OK)
		m_debug.printf(0, "Warning: configuration is corrupt.\n");
	else
		m_debug.printf(0, "Success.\n");
	}


int CXPexec::ExecuteCommand(SXPcommand *pcommand)
	{
	int res = RES_FAIL;
	char buf[0x400];

	switch(pcommand->command)
		{
		case COMM_CONFIG:
			res = m_pconfig->ConfigureSerial(pcommand->sarg, false);
			break;

		case COMM_CPU_RESET:
			res = m_pconfig->Reset();
			break;

		case COMM_PAUSE:
			res = RES_OK;
			m_pconfig->Delay(pcommand->iarg1);
			break;

		// Modified by Jeremy to allow start address != 0, 2007 10 30
		case COMM_FLASH_PROGRAM:
			res = m_pconfig->CXPflash::Program(pcommand->sarg, pcommand->iarg1);
			m_finalExec |= (res==RES_OK);
			break;

		case COMM_FLASH_VERIFY:
			res = m_pconfig->CXPflash::Verify(pcommand->sarg);
			break;

		case COMM_FLASH_READ:
			res = m_pconfig->Read(pcommand->sarg, pcommand->iarg1, 0);
			break;

		case COMM_CONFIG_PROGRAM:
			res = m_pconfig->Write(0, pcommand->sarg);
			m_finalExec |= (res==RES_OK);
			break;

		case COMM_CONFIG_VERIFY:
			res = m_pconfig->Verify(0, pcommand->sarg);
			break;

		case COMM_CONFIG_CHOOSE_PROGRAM:
			if (res=m_pconfig->ChooseBitstream(pcommand->sarg, buf)==RES_OK)
				res = m_pconfig->Write(0, buf);
			else
				m_debug.printf(0, "Cannot find a compatible logic configuration in %s directory.\n", pcommand->sarg);
			break;

		case COMM_CONFIG_CHOOSE_VERIFY:
			if (res=m_pconfig->ChooseBitstream(pcommand->sarg, buf)==RES_OK)
				res = m_pconfig->Verify(0, buf);
			else
				m_debug.printf(0, "Cannot find a compatible logic configuration in %s directory.\n", pcommand->sarg);
			break;
	
		case COMM_UPDATE:
			res = m_pconfig->Update(true);
			break;

		case COMM_INFO:
			res = RES_OK;
			PrintInfo();
			break;
		}
	return res;
	}



