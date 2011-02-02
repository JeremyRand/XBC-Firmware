#ifndef CNULLCOMMDEVICE_H
#define CNULLCOMMDEVICE_H

class CNullCommDevice : public CCommunicationDevice
{
public:
  CNullCommDevice() { m_isConnected = true; }
  virtual ~CNullCommDevice() {}
  void Connect(int timeout = 0) {}
  int QueryReadCount()
	{
		return 0;
	}
  virtual int Read(char* buf,  unsigned short length, int timeout = 0) {
    return(0);
  }
  virtual int Write(const char* buf,  unsigned short length, int timeout = 0) {
    return(length);
  }
  int SetBaudRate(int baud)
	{
		return 1;
	}
	int GetBaudRate()
	{
		return 0;
	}
};

#endif
