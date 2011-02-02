#include "CBluetoothDevice.h"
#include "CInterruptContSingleton.h"

CBluetoothDevice::CBluetoothDevice() : CBluetoothQueue( CInterruptContSingleton::InstancePtr(), UART_BASE, UART_VECTOR)
{
	m_isConnected = false;
	globBaud = DBC_BAUD_38K;
}

CBluetoothDevice::~CBluetoothDevice()
{

}

/*void CBluetoothDevice::Discover(char [][10] devices,int msTimeout=2000)
{
	if(DiscoverDevices(devices,msTimeout) >= 0) return 1;
	return -1;
}*/

void CBluetoothDevice::Connect(int msTimeout)
{
	if(SlaveConnect(globBaud) >= 0) m_isConnected = true;
}

void CBluetoothDevice::MConnect(char * addr, int msTimeout)
{
	if(MasterConnect(addr,msTimeout,globBaud) >= 0) m_isConnected = true;
}

int CBluetoothDevice::Read(char *buf, unsigned short len, int timeout)
{
	int result;
	if((result = ReadData(buf, len)) < len)
		m_isConnected = false;
	return result;
	//return ReadData(buf, len, 10);
}

int CBluetoothDevice::Write(const char *outData, unsigned short len, int timeout)
{
	return WriteData(outData, len);
}

int CBluetoothDevice::QueryReadCount()
{
  return ReceiveCount();
}

int CBluetoothDevice::SetBaudRate(int baud)
{
	int result = CBluetoothComm::Reset(baud);
	if(result < 0)
		return 0;
	globBaud = baud;
	return 1;
}

int CBluetoothDevice::GetBaudRate()
{
	return globBaud;
}
