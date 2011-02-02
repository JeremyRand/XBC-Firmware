#ifndef MESSAGE_H
#define MESSAGE_H

class CMessage
{
  bool m_isLittleEndian;
  unsigned char m_routingNumber;
  unsigned int m_parameter;
  unsigned short m_checksum;
public:
  CMessage(unsigned char* msg);
  ~CMessage();
  
  void SetRoutingNumber(unsigned char routingNumber) { m_routingNumber = routingNumber; }
  void SetParameter(unsigned int parameter) { m_parameter = parameter; }

  unsigned char GetRoutingNumber() { return m_routingNumber; }
  unsigned int GetParameter() { return m_parameter; }

  bool IsChecksumValid();
  void GetMessagePackage(unsigned char* msg); 
};

#endif
