#ifndef CCOMMUNICATIONDEVICE_H
#define CCOMMUNICATIONDEVICE_H


class CCommunicationDevice
{
protected:
	bool m_isConnected;
public:
	int globBaud;
	CCommunicationDevice();
	virtual ~CCommunicationDevice();
	bool IsConnected() { return m_isConnected; }
	void Disconnect() { m_isConnected = false;}
	virtual void Connect(int timeout = 0) = 0;
    virtual int QueryReadCount() = 0;
	virtual int Read(char* buf,  unsigned short length, int timeout = 0) = 0;
	virtual int Write(const char* buf,  unsigned short length, int timeout = 0) = 0;
	virtual int SetBaudRate(int baud) = 0;
	virtual int GetBaudRate() = 0;
};

#endif
