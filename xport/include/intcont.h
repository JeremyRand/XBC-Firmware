#ifndef _CINTCONT_H
#define _CINTCONT_H

#define DCI_MAX_VECTORS		32
class IInterrupt;

class CInterruptCont
	{
public:
	CInterruptCont();
	~CInterruptCont();

	int Register(IInterrupt *pint, unsigned char vector);
	IInterrupt * GetRegistered(unsigned char vector);
	int UnRegister(unsigned char vector);

	void Mask(unsigned char vector);
	void Unmask(unsigned char vector);
	void MaskAll();
	void UnmaskAll();
	unsigned short GetStatus(unsigned char vector);
	unsigned char ServiceCheck(unsigned char vector);

	void Interrupt(unsigned char vector);

	static IInterrupt *m_vectors[DCI_MAX_VECTORS];
	static unsigned short m_compare[DCI_MAX_VECTORS];
	};

#endif
