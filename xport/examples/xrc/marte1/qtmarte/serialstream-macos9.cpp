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
#if defined(__MWERKS)
#include <string.h>
#include <extras.h>
#include <Serial.h>

#define MAC_HAS_DEBUGF

#ifdef MAC_HAS_DEBUGF
void debugf(char *fmt, ...);
#endif

void
SerialStream::osOpen()
{
  unsigned char *inName, *outName;

  assert(!streamValid);

  if (!stricmp(portname.c_str(), "Printer port")) {
    inName="\p.BIn";    
    outName="\p.BOut";
  } else if (!stricmp(portname.c_str(), "Modem port")) {
    inName="\p.AIn";
    outName="\p.AOut";
  } else {
    assert(0);
  }

  short inStream, outStream;
  int err;
  err= OpenDriver(outName, &outStream);
  if (err) {
    debugf("error %d opening output stream\n", err);
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
  err= OpenDriver(inName, &inStream);
  if (err) {
    debugf("output stream opened successfully\n");
    debugf("error %d opening input stream\n", err);
    CloseDriver(outStream);
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
  streamValid=1;
  ostream= outStream;
  istream= inStream;
}

void
SerialStream::osClose()
{
  assert(streamValid);
  streamValid=0;
  KillIO(ostream);
  CloseDriver(istream);
  CloseDriver(ostream);
}

void
SerialStream::osUpdateProperties()
{
  assert(streamValid);
  int init=0;
  switch (baud) {
  case 300:	init= baud300;	 break;
  case 600:	init= baud600;	 break;
  case 1200:	init= baud1200;	 break;
  case 2400:	init= baud2400;	 break;
  case 4800:	init= baud4800;	 break;
  case 9600:	init= baud9600;	 break;
  case 19200:	init= baud19200; break;
  case 38400:	init= baud38400; break;
  case 57600:	init= baud57600; break;
  default: THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  }
  switch (nDataBits) {
  case 5: init |= data5; break;
  case 6: init |= data6; break;
  case 7: init |= data7; break;
  case 8: init |= data8; break;
  default: THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  }
  switch (nStopBits) {
  case 1: init |= stop10; break;
  case 2: init |= stop20; break;
  default: THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  }
  init |= noParity;
  int err;
  err= SerReset(istream, init);
  if (err) {
    debugf("Error %d calling SerReset of istream\n", err);
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
  SerReset(ostream, init);
  if (err) {
    debugf("Error %d calling SerReset of ostream\n", err);
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
  SerShk flags= {0, // xon/xoff output flow
		 0, // CTS output flow
		 0, // xon char
		 0, // xoff char
		 0, // mask for errs which will terminate input
		 0, // mask for status changes which cause events
		 0, // xon/xoff input control flow
		 0}; // DTR input flow control (csCode 14 only)
  err= Control(ostream, 14, &flags);
  if (err) {
    debugf("Error %d calling Control on ostream\n", err);
    THROW SerialStreamError(SerialStreamError::IO_ERROR);
  }
}


long long
ss_utime()
{
  UnsignedWide usecs;
  Microseconds(&usecs);
  return (((long long)usecs.hi)<<32)+((long long)usecs.lo);
}

int
SerialStream::mtime()
{
  return (int) (ss_utime() / 1000);
}

int
SerialStream::osRead(unsigned char *buf, int len, int timeout)
{
  assert(streamValid);
  
  long long end_time= ss_utime() + (long long) timeout * 1000;
  int n_read= 0;

  while (1) {
    long long u_timeout= end_time - ss_utime();
    if (u_timeout < 0) u_timeout= 0;
    if (len <= 0) return n_read;

    long pending=0;
    int err;
    err= SerGetBuf(istream, &pending);
    if (err) {
      debugf("Error %d getting serial buffer\n", err);
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
    }
    if (pending>0) {
      debugf("%d chars are pending\n", pending);
      if (pending > len) pending= len;
      err= FSRead(istream, &pending, buf);
      if (err) {
	debugf("Error %d reading serial\n", err);
	THROW SerialStreamError(SerialStreamError::IO_ERROR);
      }
      debugf("Read %d chars\n", pending);
      n_read += pending;
      buf += pending;
      len -= pending;
    }
    if (u_timeout==0) return n_read;
  }
}

void
SerialStream::osWrite(const unsigned char *buf, int len)
{
  while (len > 0) {
    long nWritten= len;
    int err= FSWrite(ostream, &nWritten, buf);
    if (err || nWritten < 0) {
      debugf("Error %d writing serial\n", buf);
      THROW SerialStreamError(SerialStreamError::IO_ERROR);
    }
    len -= nWritten;
    buf += nWritten;
  }
}

vector<string>
SerialStream::candidatePortnames() {
  vector<string> ret;
  ret.push_back("Printer port");
  ret.push_back("Modem port");
  return ret;
}

void
SerialStream::msleep(int msec)
{
  int begintime= ss_utime();
  while (ss_utime()-begintime < msec*1000) {}
}
#endif