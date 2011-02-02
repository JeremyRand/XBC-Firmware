#include "Message.h"

CMessage::CMessage(unsigned char* msg) : m_isLittleEndian(false), m_routingNumber(0), m_parameter(0), m_checksum(0)
{
  long test = 0x11223344;
  unsigned char* uc = (unsigned char*) &test;
  if(*uc == 0x44) m_isLittleEndian = true;

  if(m_isLittleEndian)
    {
      m_routingNumber = msg[1] << 8 | msg[0]; 
      m_parameter = msg[5] << 24 | msg[4] << 16 | msg[3] << 8 | msg[2];
      m_checksum = msg[7] << 8 | msg[6];
    }
  else
    {

      m_routingNumber = msg[0] << 8 | msg[1];
      m_parameter = msg[2] << 24 | msg[3] << 16 | msg[4] << 8 | msg[5];
      m_checksum = msg[6] << 8 | msg[7];
    }

}

CMessage::~CMessage()
{
}

bool CMessage::IsChecksumValid()
{
  if(m_checksum > 0)
    {
      return true;
    }
  return false;
}

void CMessage::GetMessagePackage(unsigned char* msg)
{
  if(m_isLittleEndian)
    {
      msg[0] = (m_routingNumber && 0x00ff);
      msg[1] = (m_routingNumber && 0xff00) >> 8;
      msg[2] = (m_parameter && 0x000000ff);
      msg[3] = (m_parameter && 0x0000ff00) >> 8;
      msg[4] = (m_parameter && 0x00ff0000) >> 16;
      msg[5] = (m_parameter && 0xff000000) >> 24;
      m_checksum = msg[0] + msg[1] + msg[2] + msg[3] + msg[4] + msg[5];
      msg[6] = (m_checksum && 0x00ff);
      msg[7] = (m_checksum && 0xff00) >> 8;
    }
  else
    {
      msg[1] = (m_routingNumber && 0x00ff);
      msg[0] = (m_routingNumber && 0xff00) >> 8;
      msg[5] = (m_parameter && 0x000000ff);
      msg[4] = (m_parameter && 0x0000ff00) >> 8;
      msg[3] = (m_parameter && 0x00ff0000) >> 16;
      msg[2] = (m_parameter && 0xff000000) >> 24;
      m_checksum = msg[0] + msg[1] + msg[2] + msg[3] + msg[4] + msg[5];
      msg[7] = (m_checksum && 0x00ff);
      msg[6] = (m_checksum && 0xff00) >> 8;
    }
}
