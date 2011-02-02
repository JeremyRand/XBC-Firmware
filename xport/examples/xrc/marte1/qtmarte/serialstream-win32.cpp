// Copyright (c) 2001 Randy Sargent
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions are 
// met:
//
// Redistributions of source code must retain the above copyright notice, 
// this list of conditions and the following disclaimer. 
//
// Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the following disclaimer in the documentation 
// and/or other materials provided with the distribution. 
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
// TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE 
// LIABLE FOR ANY DIRECT, INDIRECT, INIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
// THE POSSIBILITY OF SUCH DAMAGE.

// included from serialstream.cpp
#include <windows.h>
// WIN32-specific
void
SerialStream::osOpen()
{
  assert(sizeof(streamptr) >= sizeof(HANDLE));
  assert(!streamValid);
  streamptr = CreateFileA(portname.c_str(),
                         GENERIC_READ | GENERIC_WRITE,
                         0,    // comm devices must be opened w/exclusive-access
                         NULL, // no security attributes
                         OPEN_EXISTING, // comm devices must use OPEN_EXISTING
                         0,    // not overlapped I/O
                         NULL  // hTemplate must be NULL for comm devices
    );
  streamValid= (streamptr != INVALID_HANDLE_VALUE);
  if (!streamValid) {
    int err= GetLastError();
    switch (err) {
    case ERROR_FILE_NOT_FOUND:
      THROW SerialStreamError(SerialStreamError::NO_SUCH_DEVICE);
      break;
    case ERROR_ACCESS_DENIED:
      THROW SerialStreamError(SerialStreamError::IN_USE);
      break;
    default:
      printf("Could not open %s, error=%d\n",
             portname.c_str(),
             GetLastError());
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
      break;
    }
  }

  COMMTIMEOUTS cto;
  cto.ReadIntervalTimeout= 0;
  cto.ReadTotalTimeoutMultiplier=0;
  cto.ReadTotalTimeoutConstant=0;
  cto.WriteTotalTimeoutMultiplier=0;
  cto.WriteTotalTimeoutConstant=0;

  if (!SetCommTimeouts(streamptr, &cto)) {
    osClose();
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }

  if (!SetupComm(streamptr, 2048, 2048)) {
    osClose();
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }

  if (!SetCommMask(streamptr, 0)) {
    osClose();
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}

// WIN32-specific
void
SerialStream::osClose()
{
  assert(streamValid);
  streamValid= 0;
  if (!CloseHandle(streamptr)) {
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}

// WIN32-specific
void
SerialStream::osUpdateProperties()
{
  assert(streamValid);

  DCB dcb;
  memset(&dcb, sizeof(dcb), 0);
  
  dcb.DCBlength= sizeof(DCB);
  BOOL fSuccess = GetCommState(streamptr, &dcb);
  
  if (!fSuccess) {
    // Handle the error.
    // printf ("GetCommState failed with error %d.\n", GetLastError());
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }

  // We will build on the current configuration, and skip setting the size
  // of the input and output buffers with SetupComm.
  
  dcb.DCBlength= sizeof(DCB);
  dcb.BaudRate = baud;          // current baud rate 
  dcb.fBinary = 1;              // binary mode, no EOF check 
  dcb.fOutxCtsFlow = 0;         // CTS output flow control 
  dcb.fOutxDsrFlow = 0;         // DSR output flow control 
  dcb.fDtrControl = DTR_CONTROL_DISABLE; // DTR flow control type 
  dcb.fDsrSensitivity = 0;      // DSR sensitivity 
  dcb.fTXContinueOnXoff= 0;     // XOFF continues Tx 
  dcb.fOutX = 0 ;               // XON/XOFF out flow control 
  dcb.fInX =  0 ;               // XON/XOFF in flow control 
  dcb.fErrorChar=0;             // enable error replacement 
  dcb.fNull=0;                  // enable null stripping 
  dcb.fRtsControl= RTS_CONTROL_TOGGLE;// RTS flow control 
  dcb.fAbortOnError= FALSE;     // abort on error
  if (nDataBits < 4 || nDataBits > 8) {
    THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  }
    
  dcb.ByteSize = nDataBits;     // number of bits/byte, 4-8
  switch (parity) {
  case 'e': case 'E':
    dcb.Parity = EVENPARITY;    // 0-4=no,odd,even,mark,space 
    dcb.fParity = 0;            // enable parity checking
    break;
  case 'o': case 'O':
    //printf("ODD PARITY\n");
    dcb.Parity = ODDPARITY;     // 0-4=no,odd,even,mark,space 
    dcb.fParity = 0;            // enable parity checking 
    break;
  case 'n': case 'N':
    dcb.Parity = NOPARITY;      // 0-4=no,odd,even,mark,space 
    dcb.fParity = 0;            // enable parity checking 
    break;
  case 'm': case 'M':
    dcb.Parity = MARKPARITY;    // 0-4=no,odd,even,mark,space 
    dcb.fParity = 0;            // enable parity checking 
    break;
  case 's': case 'S':
    dcb.Parity = SPACEPARITY;   // 0-4=no,odd,even,mark,space 
    dcb.fParity = 0;            // enable parity checking 
    break;
  default:
    THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  }
  switch (nStopBits) {
  case 1: dcb.StopBits = ONESTOPBIT; break;
  case 2: dcb.StopBits = TWOSTOPBITS; break;
  default:
    THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  }
  fSuccess = SetCommState(streamptr, &dcb);

  if (!fSuccess) {
    // Handle the error.
    //printf ("SetCommState failed with error %d.\n", GetLastError());
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
  //printf ("Serial port successfully reconfigured.\n");
}

// WIN32-specific
int
SerialStream::osRead(unsigned char *buf, int len, int timeout)
{
  if (!streamValid) {
    THROW SerialStreamError(SerialStreamError::NOT_CONNECTED);
  }
  
  COMMTIMEOUTS cto;
  cto.ReadIntervalTimeout= 0;
  cto.ReadTotalTimeoutMultiplier=0;
  cto.ReadTotalTimeoutConstant=0;
  if (timeout == 0) {
    cto.ReadIntervalTimeout= MAXDWORD;
  } else if (timeout > 0) {
    cto.ReadTotalTimeoutConstant=timeout;
  }
  cto.WriteTotalTimeoutMultiplier=0;
  cto.WriteTotalTimeoutConstant=0;
  if (!SetCommTimeouts(streamptr, &cto)) {
    if (debug) printf("error %d in setcommtimeouts", GetLastError());
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
  
  DWORD nread;
  nread= 0;
  if (!ReadFile(streamptr, buf, len, &nread, NULL)) {
    if (debug) printf("error %d in readfile", GetLastError());
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
  //printf("read %d of %d bytes\n", nread, len);
  return nread;
}

// WIN32-specific
void
SerialStream::osWrite(const unsigned char *buf, int len)
{
  assert(streamValid);
  if(len == 0) return;
  DWORD nwritten = 0;
  DWORD totalwritten = 0;
  int leftToWrite = len;
  int writeInPass = 0;
  while(leftToWrite > 0)
  {
	if(leftToWrite > 1024) writeInPass = 1024;
	else writeInPass = leftToWrite;
	leftToWrite -= writeInPass;
	if (!WriteFile(streamptr, buf+(int)totalwritten, writeInPass, &nwritten, NULL))
	{

		THROW SerialStreamError(SerialStreamError::IO_ERROR);
	}
	totalwritten += nwritten;
  }

  if (totalwritten != len)
  {
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}

// WIN32-specific
int
SerialStream::mtime()
{
  return GetTickCount();
}
  
// WIN32-specific
void
SerialStream::msleep(int msec)
{
  Sleep(msec);
}

// WIN32-specific
void
SerialStream::transmitBreak(bool on)
{
  if (on) {
    SetCommBreak(streamptr);
  } else {
    ClearCommBreak(streamptr);
  }
}

// WIN32-specific
void
SerialStream::transmitBreakFor(int mseconds)
{
  SetCommBreak(streamptr);
  msleep(mseconds);
  ClearCommBreak(streamptr);
}

vector<string>
SerialStream::candidatePortnames()
{
  vector<string> ret;
  for (int i= 1; i<= 32; i++) {
    char name[20];
    sprintf(name, "\\\\.\\com%d", i);
    ret.push_back(name);
  }
  return ret;
}

