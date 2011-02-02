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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#ifdef linux
#include <linux/serial.h>
#endif
#include <sys/ioctl.h>
#include <sys/time.h>
#include <errno.h>

void
SerialStream::msleep(int msec)
{
	usleep(msec*1000);
}

// Linux-specific
void
SerialStream::osOpen()
{
	int begin=mtime();
#ifdef DEBUG
	printf("%d: about to open\n", mtime()-begin);
#endif
#if defined(O_NDELAY)
	/* Open with NDELAY to make sure we don't hang
		because of handshaking lines! */
	stream= open(portname.c_str(), O_RDWR | O_NDELAY);
	if (stream < 0) THROW SerialStreamError(SerialStreamError::IO_ERROR);
#ifdef DEBUG
	printf("%d: about to take off O_NDELAY\n", mtime()-begin);
#endif 
	/* Now take the O_NDELAY back off so we actually wait
		for buffers to flush when we do writes */
	fcntl(stream, F_SETFL, ~O_NDELAY & fcntl(stream, F_GETFL, 0));
#else
#error "first time this has been used.  be sure to test."
	stream= open(portname.c_str(), O_RDWR);
	if (stream < 0) THROW SerialStreamError(SerialStreamError::IO_ERROR);
#endif
#ifdef DEBUG
	printf("%d: done\n", mtime()-begin);
#endif 
	streamValid= 1;
}

// Linux-specific
void
SerialStream::osClose()
{
	assert(streamValid);
	streamValid= 0;
	if (close(stream) == -1)
		THROW SerialStreamError(SerialStreamError::IO_ERROR);
}

// Unix-specific
// Returns -1 if unrecognized baud
int
ss_baud2B(int baud)
{
	switch (baud) {
		case 300: return B300;
		case 1200: return B1200;
		case 2400: return B2400;
		case 4800: return B9600;
		case 9600: return B9600;
#ifdef B19200
		case 19200: return B19200;
#else
		case 19200: return EXTA;
#endif
#ifdef B38400
		case 38400: return B38400;
#else
		case 38400: return EXTB;
#endif
#ifdef B57600
		case 57600: return B57600;
#endif
#ifdef B115200
		case 115200: return B115200;
#endif
#ifdef B230400
		case 230400: return B230400;
#endif
#ifdef B460800
		case 460800: return B460800;
#endif
		default:
			return -1;
	}
}

// Linux-specific

void
SerialStream::osUpdateProperties()
{
	assert(streamValid);
	
	struct termios tio;
	int baudb = ss_baud2B(baud);
	
	if (tcgetattr(stream, &tio) == -1)
		THROW SerialStreamError(SerialStreamError::IO_ERROR);
	
	tio.c_iflag= IGNBRK;
	tio.c_oflag= 0;
	tio.c_cflag= CREAD | CLOCAL;
	switch (nDataBits) {
		case 5: tio.c_cflag |= CS5; break;
		case 6: tio.c_cflag |= CS6; break;
		case 7: tio.c_cflag |= CS7; break;
		case 8: tio.c_cflag |= CS8; break;
		default:
			THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
	}
	switch (nStopBits) {
		case 1: break;
		case 2: tio.c_cflag |= CSTOPB; break;
		default:
			THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
	}
	switch (parity) {
		case 'e': case 'E': tio.c_cflag |= PARENB; break;
		case 'o': case 'O': tio.c_cflag |= PARENB | PARODD; break;
		case 'n': case 'N': break;
		default:
			THROW SerialStreamError(SerialStreamError::ILLEGAL_ARG);
	}
	tio.c_lflag= 0;
	bzero(&tio.c_cc, sizeof(tio.c_cc)); /* clear control characters */
	
	cfsetspeed(&tio, (baudb == -1 ? B38400 : baudb));
	
	if (tcsetattr(stream, TCSADRAIN, &tio) == -1)
		THROW SerialStreamError(SerialStreamError::IO_ERROR);
	
#ifdef linux  
	struct serial_struct serinfo;
	if (ioctl(stream, TIOCGSERIAL, &serinfo) == -1)
		THROW SerialStreamError(SerialStreamError::IO_ERROR);
	
	if (baudb == -1) {
		int divisor;
		int real_baud;
		double err_factor;
		
		divisor = (int)((serinfo.baud_base / (double)baud) + 0.5);
		real_baud = serinfo.baud_base / divisor;
		err_factor = (double)baud/(double)real_baud;
		
		if (err_factor < 0.98 || err_factor > 1.02) {
			printf("Can't achieve %d baud; using %d\n", baud, real_baud);
		}
		
		serinfo.custom_divisor = divisor;
		serinfo.flags = (serinfo.flags & ~ASYNC_SPD_MASK) | ASYNC_SPD_CUST;
	} else {
		if (baud == 38400) {
			/* We have to reset the spd_cust flag to achieve this speed... */
			serinfo.flags = (serinfo.flags & ~ASYNC_SPD_MASK);
		}
	}
	if (ioctl(stream, TIOCSSERIAL, &serinfo) == -1)
		THROW SerialStreamError(SerialStreamError::IO_ERROR);
#endif  
}

// Linux-specific
void
SerialStream::transmitBreakFor(int mseconds)
{
	if (!streamValid) THROW SerialStreamError(SerialStreamError::NOT_CONNECTED);
	int duration=(mseconds + 249) / 250;
	if (duration < 1) duration= 1;
	tcsendbreak(stream, duration);
	msleep(duration*500);
}

// Linux-specific
long long
ss_utime()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ((long long) tv.tv_sec * (long long) 1000000 +
			(long long) tv.tv_usec);
}

// Linux-specific
int
SerialStream::mtime()
{
	return (int) (ss_utime() / 1000);
}

// Linux-specific
int
SerialStream::osRead(unsigned char *buf, int len, int timeout)
{
	assert(streamValid);
	
	long long end_time= ss_utime() + (long long) timeout * 1000;
	fd_set read_set, write_set, exceptional_set;
	int n_read= 0;
	
	while (1) {
		int nfound;
		while (1) {
			struct timeval timeval_timeout;
			long long u_timeout= end_time - ss_utime();
			if (u_timeout < 0) u_timeout= 0;
			timeval_timeout.tv_sec= (long) (u_timeout / 1000000);
			timeval_timeout.tv_usec= (long) (u_timeout % 1000000);
			
			FD_ZERO(&read_set);
			FD_ZERO(&write_set);
			FD_ZERO(&exceptional_set);
			
			FD_SET(stream, &read_set);
			FD_SET(stream, &exceptional_set);
			
			nfound= select(stream+1, &read_set, &write_set, &exceptional_set,
						   timeout < 0 ? 0 : &timeval_timeout);
			if (nfound < 0) {
				if (errno != EINTR && errno != EAGAIN) {
					if (g_serstrDebugErrors) perror("osRead/select");
					THROW SerialStreamError(SerialStreamError::IO_ERROR);
				}
			} else {
				break;
			}
		}
		
		if (nfound == 0) return n_read;
		if (FD_ISSET(stream, &exceptional_set)) {
			if (n_read > 0) return n_read;
			if (g_serstrDebugErrors)
				fprintf(stderr, "ERROR: Exceptional set in select\n");
			THROW SerialStreamError(SerialStreamError::IO_ERROR);
		}
		if (FD_ISSET(stream, &read_set)) {
			int n= ::read(stream, buf, len);
			if (n < 0) {
				if (errno != EINTR && errno != EAGAIN) {
					if (g_serstrDebugErrors) perror("osRead/read");
					THROW SerialStreamError(SerialStreamError::IO_ERROR);
				}
			} else {
				n_read += n;
				buf += n;
				len -= n;
				if (len <= 0) return n_read;
			}
		}
	}
}


// Linux-specific
void
SerialStream::osWrite(const unsigned char *buf, int len)
{
	assert(streamValid);
	
	while (len) {
		int n_written= ::write(stream, buf, len);
		if (n_written >= len) break;
		if (n_written >= 0) {
			buf += n_written;
			len -= n_written;
		} else if (errno != EAGAIN && errno != EINTR) {
			THROW SerialStreamError(SerialStreamError::IO_ERROR);
		}
	}
	return;
}


// Set DTR state. True means DTR high (positive)
void
SerialStream::setDTR(bool high)
{
	int i= TIOCM_DTR;
	if (ioctl(stream, high ? TIOCMBIS : TIOCMBIC, &i)) {
		THROW SerialStreamError(SerialStreamError::IO_ERROR);
	}
}


// Set RTS state. True means RTS high (positive)
void
SerialStream::setRTS(bool high)
{
	int i= TIOCM_RTS;
	if (ioctl(stream, high ? TIOCMBIS : TIOCMBIC, &i)) {
		THROW SerialStreamError(SerialStreamError::IO_ERROR);
	}
}



#if defined(__MACH__) && defined(__APPLE__)
#include "serialstream-candidate-portnames-darwin.cpp"
#else
// Linux-specific
vector<string>
SerialStream::candidatePortnames()
{
	char name[20];
	vector<string> ret;
    
	for (int i= 0; i< 64; i++) {
		sprintf(name, "/dev/ttyS%d", i); // Linux
		ret.push_back(name);
	}
	for (int c= 'a'; c<='z'; c++) {
		sprintf(name, "/dev/tty%c", c); // Sun
		ret.push_back(name);
	}
	return ret;
}
#endif

