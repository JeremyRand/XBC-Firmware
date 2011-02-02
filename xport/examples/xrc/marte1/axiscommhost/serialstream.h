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

#ifndef SERIALSTREAM_H
#define SERIALSTREAM_H

#ifdef _WIN32
#pragma warning(disable : 4786) 
#endif

#include <string>
#include <vector>

//////////////////////////////
// SerialStreamError struct //
//////////////////////////////

struct SerialStreamError {
  enum Status {
    NOT_CONNECTED= 1,
    IO_ERROR= 2,
    ILLEGAL_ARG= 3,
    NO_SUCH_DEVICE= 4,
    IN_USE= 5
  } status;

  inline
  SerialStreamError(Status stat) { status= stat; }
};

struct PortTest {
  std::string name;
  bool   inUse;
  bool   isModem;

  PortTest() {}
  
  PortTest(std::string nameArg, bool inUseArg, bool isModemArg) :
    name(nameArg),
    inUse(inUseArg),
    isModem(isModemArg) {}
};

class SerialStream {
public:

  inline
  SerialStream(int debugArg= 0) {
    debug= debugArg;
    streamValid= 0;
    nDataBits= 8;
    nStopBits= 1;
    baud=9600;
    parity='n';
    bufferLen= bufferBegin= 0;
  }

  ~SerialStream() {
    disconnect();
  }
    

  // setSpeed takes the actual baud rate as a number, e.g. 9600
  void
  setSpeed(int baudArg);
    

  // Change the format of the stream
  void
  setFormat(int nDataBitsArg,      // Number of data bits, e.g. 8
            char parityArg,        // Parity: 'E', 'O', 'N'
            int nStopBitsArg);     // Number of stop bits: 1, 2

  // Get the current baud rate
  int
  getSpeed();

  // Get the current format
  void
  getFormat(int *nDataBitsRet, char *parityRet, int *nStopBitsRet);
  
  // Set the portname and connect the stream
  // Under UNIX, portname would typically be something like "/dev/ttyS0"
  // Under Win32, portname would typically be something like "com1"
  // Returns true on success
  bool
  connect(std::string portnameArg);

  std::string
  getPortname();
  
  bool
  isConnected();
  
  void
  disconnect();

  // Write len bytes
  void
  write(const unsigned char *buf, int len);
  
  // Write a zero-terminated string (without the zero termination)
  void
  writeString(const char *buf);
  
  // Blocking read of len bytes
  void
  read(unsigned char *buf, int len);
  
  // Read until either len bytes have been read
  //     or timeout milliseconds have elapsed
  //
  // timeout=0 means return immediately, only reading bytes which have
  //   already arrived
  // timeout=-1 means wait forever (same as "read")
  //
  // Returns number of bytes read (may be zero)
  //
  int
  readTimeout(unsigned char *buf, int len, int timeout);

  
  // Read until either len bytes have been read
  //     or until mtime() reaches lastReadTime
  //
  // timeout=0 means return immediately, only reading bytes which have
  //   already arrived
  // timeout=-1 means wait forever (same as "read")
  //
  // Returns number of bytes read (may be zero)
  //
  int
  readUntil(unsigned char *dest, int len, int lastReadTime);

  // Return the time in milliseconds
  static int
  mtime();

  // Sleep for msec milliseconds
  static void
  msleep(int msec);

  // Find ports
  static std::vector<PortTest>
  findPorts(bool testForModem);

  // Try and test whether port is attached to a modem
  bool
  testIfModem();

  // Get a character within timeout milliseconds
  // Returns -1 on timeout
  // See readTimeout for interpretation of timeout argument
  inline int
  getcharTimeout(int timeout) {
    unsigned char buf[1];
    if (readTimeout(buf, 1, timeout) == 1) return buf[0];
    return -1;
  }

  // Get a character until mtime() reaches lastReadTime
  // Returns -1 on timeout
  inline int
  getcharUntil(int lastReadTime) {
    unsigned char buf[1];
    if (readUntil(buf, 1, lastReadTime) == 1) return buf[0];
    return -1;
  }

  // Discard any already-received input on port
  void
  discardInput();
  
  // Getchar is capitalized to not conflict with stdio #define
  inline int
  Getchar() {
    return getcharTimeout(-1);
  }
  
  // Putchar is capitalized to not conflict with stdio #define
  void
  Putchar(char c) {
    write((unsigned char*) &c, 1);
  }

  // Turn on or off break transmission
  void
  transmitBreak(bool on);

  // Transmits break for an approximate duration of time
  // Break will last at least as long as the time requested, and
  // at most double the time requested, plus half a second.
  // (Imprecision is due to the definition of cfsendbreak)
  // This function blocks during transmission of break, but may block
  // for longer than the break lasts (tries very hard not to block less)
  void
  transmitBreakFor(int mseconds);
  
  // Set DTR state. True means DTR high (positive)
  void
  setDTR(bool high);
  
  // Set RTS state. True means RTS high (positive)
  void
  setRTS(bool high);
  
  void *streamptr;
  static int  debug;
   
private:
  std::string portname;
  int baud;
  int nDataBits;   // 7, 8
  char parity;     // 'n':none, 'e':even, 'o':odd
  int nStopBits;   // 1, 1.5, 2
  int stream;
  int istream;
  int ostream;

//  void *streamptr;
  bool streamValid;
  unsigned char buffer[512];
  int bufferBegin;
  int bufferLen;
  
  void
  osOpen();
  
  void
  osClose();
  
  void
  osWrite(const unsigned char *buf, int len);
  
  int
  osRead(unsigned char *buf, int len, int timeout);
  
  void
  osUpdateProperties();
  
  void
  printBuffer();
  
  void
  copyFromBuffer(unsigned char *dest, int copy_len);
  
  void
  copyToBuffer(unsigned char *src, int copy_len);

  static std::vector<std::string>
  candidatePortnames();

  static void
  debugf(char *fmt, ...);
  
  // For now, implement no flow control
};

#endif
