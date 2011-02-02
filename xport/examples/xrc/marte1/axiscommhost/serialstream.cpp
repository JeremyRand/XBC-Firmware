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

#include "serialstream.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

using namespace std;

int g_serstrDebugErrors= 1;

#define THROW debugf("Throwing from " __FILE__ ":%d\n", __LINE__), throw

#if defined(_WIN32)
#include "serialstream-win32.cpp"
#elif defined(__MWERKS__)
// Mac OS 7/8/9
#include "serialstream-macos9.cpp"
#else
#include "serialstream-unix.cpp"
#endif

int SerialStream::debug;

vector<PortTest>
SerialStream::findPorts(bool testForModem)
{
  int begin= mtime();
  vector<PortTest> ret;
  SerialStream test;
  debugf("%d: about to find candidate portnames\n", mtime()-begin);
  vector<string> candidates= candidatePortnames();
  debugf("%d: found candidate portnames\n", mtime()-begin);
  unsigned int i;
  for (i= 0; i< candidates.size(); i++) {
    string name= candidates[i];
    try {
      debugf("%d: about to connect to %s\n", mtime()-begin, name.c_str());
      test.connect(name);
      bool isModem= false;
      if (testForModem) {
        debugf("%d: about to test if modem\n", mtime()-begin);
        isModem= test.testIfModem();
        debugf("%d: done testing, modem=%d\n", mtime()-begin, isModem);
      }
      ret.push_back(PortTest(name, false, isModem));
      debugf("%d: about to disconnect\n", mtime()-begin);
      test.disconnect();
      debugf("%d: disconnected\n", mtime()-begin);
    }
    catch (SerialStreamError e) {
      switch (e.status) {
      case SerialStreamError::IN_USE:
        ret.push_back(PortTest(name, true, false));
        break;
      default:
        break;
      }
    }
  }
  return ret;
}

void
SerialStream::printBuffer()
{
  unsigned int i;
  for (i= 0; i< sizeof(buffer); i++) putchar(isprint(buffer[i]) ? buffer[i] : '-');
}

// Private
void
SerialStream::copyFromBuffer(unsigned char *dest, int copy_len)
{
  int original_copy_len= copy_len;
  assert(copy_len <= bufferLen);
  bufferLen -= copy_len;
  if (debug && copy_len) {
    debugf("Read:%s: ", portname.c_str());
  }
    
  //debugf("Copying %d bytes from buffer\n", copy_len);
  while (copy_len) {
    int bytes_to_copy= copy_len;
    if (bytes_to_copy > (int)sizeof(buffer) - bufferBegin)
      bytes_to_copy = sizeof(buffer) - bufferBegin;
    memcpy(dest, buffer+bufferBegin, bytes_to_copy);
    dest += bytes_to_copy;
    //memset(buffer+bufferBegin, 0, bytes_to_copy);
    bufferBegin = (bufferBegin + bytes_to_copy) % sizeof(buffer);
    copy_len -= bytes_to_copy;
  }
  if (debug && copy_len) {
    for (int i= 0; i< original_copy_len; i++) {
      debugf("[0x%02X]", dest[i]);
    }
    debugf("\n");
  }
  //printBuffer();
}

void
SerialStream::copyToBuffer(unsigned char *src, int copy_len)
{
  assert(copy_len <= (int)sizeof(buffer)-bufferLen);
  //debugf("Copying %d bytes to buffer\n", copy_len);
  int buffer_end= (bufferBegin + bufferLen) % sizeof(buffer);
  bufferLen += copy_len;
  while (copy_len) {
    int bytes_to_copy= copy_len;
    if (bytes_to_copy > (int)sizeof(buffer) - buffer_end)
      bytes_to_copy = sizeof(buffer) - buffer_end;
    memcpy(buffer+buffer_end, src, bytes_to_copy);
    src += bytes_to_copy;
    buffer_end = (buffer_end + bytes_to_copy) % sizeof(buffer);
    copy_len -= bytes_to_copy;
  }
  //printBuffer();
}

bool
SerialStream::connect(string portnameArg)
{
  if (streamValid) osClose();
  portname= portnameArg;
  osOpen();
  osUpdateProperties();
  return streamValid;
}

void
SerialStream::disconnect()
{
  if (!streamValid) return;
  bufferLen= bufferBegin= 0;
  osClose();
}

bool
SerialStream::isConnected()
{
  return streamValid;
}

void
SerialStream::write(const unsigned char *buf, int len)
{
  if (!streamValid) THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  if (debug && len) {
    int i;
    debugf("Write:%s: ", portname.c_str());
    for (i= 0; i< len; i++) {
      debugf("[0x%02X]", buf[i]);
    }
    debugf("\n");
  }
  osWrite(buf, len);
}

void
SerialStream::writeString(const char *buf)
{
  write((unsigned char*) buf, strlen(buf));
}

void
SerialStream::read(unsigned char *buf, int len)
{
  readTimeout(buf, len, -1);
}

int
SerialStream::readTimeout(unsigned char *dest, int len, int timeout)
{
  if (!streamValid) THROW SerialStreamError(SerialStreamError::NOT_CONNECTED);
    
  unsigned char tmp[sizeof(buffer)];
  unsigned char *destptr= dest;
  int fill_amount= bufferLen;
  if (fill_amount > len) fill_amount= len;
  copyFromBuffer(destptr, fill_amount);
  destptr += fill_amount;
  len -= fill_amount;
  if (!len) return destptr - dest;
  
  // Do a polling read of as many chars as possible
  int polled_read= osRead(tmp, sizeof(tmp), 0);
  assert(polled_read >= 0);
  if (polled_read > 0) {
    copyToBuffer(tmp, polled_read);
    fill_amount= bufferLen;
    if (fill_amount > len) fill_amount= len;
    copyFromBuffer(destptr, fill_amount);
    destptr += fill_amount;
    len -= fill_amount;
  }

  if (!len || !timeout) return destptr - dest;
  
  // Do a blocking read of the rest
  int blocking_read= osRead(destptr, len, timeout);
  assert(blocking_read >= 0);
  return (destptr - dest) + blocking_read;
}

int
SerialStream::readUntil(unsigned char *dest, int len, int lastReadTime)
{
  int timeout= lastReadTime - mtime();
  if (timeout < 0) return 0;
  return readTimeout(dest, len, timeout);
}

// Discard any already-received input on port
void
SerialStream::discardInput()
{
  unsigned char buf[100];
  while (readTimeout(buf, 100, 0));
}

// setSpeed takes the actual baud rate as a number, e.g. 9600
void
SerialStream::setSpeed(int new_baud)
{
  if (new_baud <= 0)
    THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  
  if (baud != new_baud) {
    baud= new_baud;
    if (debug) debugf("Change %s speed to %d baud\n",
                      portname.c_str(), new_baud);
    if (streamValid) osUpdateProperties();
  }
}

int
SerialStream::getSpeed()
{
  return baud;
}

void
SerialStream::setFormat(int nDataBitsArg, char parityArg, int nStopBitsArg)
{
  if (nDataBitsArg < 4 || nDataBitsArg > 8)
    THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);

  switch (parityArg) {
  case 'e': case 'E':
  case 'o': case 'O':
  case 'n': case 'N':
  case 'm': case 'M':
  case 's': case 'S':
    break;
  default:
    THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  }
  
  if (nStopBitsArg < 1 || nStopBitsArg > 2)
    THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
  
  if (nDataBits != nDataBitsArg ||
      parity != parityArg ||
      nStopBits != nStopBitsArg) {
    nDataBits= nDataBitsArg;
    parity= parityArg;
    nStopBits= nStopBitsArg;
    if (streamValid) osUpdateProperties();
  }
}

void
SerialStream::getFormat(int  *nDataBitsRet,
                        char *parityRet,
                        int  *nStopBitsRet)
{
  if (nDataBitsRet) *nDataBitsRet= nDataBits;
  if (parityRet)    *parityRet= parity;
  if (nStopBitsRet) *nStopBitsRet= nStopBits;
}

bool
SerialStream::testIfModem()
{
  discardInput();
  writeString("\r\nAT\r\n");
  msleep(100);
  writeString("\r\nAT\r\n");
  msleep(100);
  writeString("\r\nAT\r\n");
  unsigned char buf[100];
  int nread= readTimeout(buf, 100, 100);
  for (int i= 0; i< nread-1; i++) {
    if (buf[i] == 'O' && buf[i+1] == 'K') {
      return true;
    }
  }
  return false;
}

string
SerialStream::getPortname()
{
  return portname;
}

#ifdef __MWERKS__
#define HAS_DEBUGF
extern void vdebugf(char *fmt, va_list args);
#endif

void
SerialStream::debugf(char *fmt, ...)
{
  if (!debug) return;
  va_list args;
  va_start(args, fmt);
#ifdef HAS_DEBUGF
  ::vdebugf(fmt, args);
#else
  vfprintf(stderr, fmt, args);
#endif
  va_end(args);
}



