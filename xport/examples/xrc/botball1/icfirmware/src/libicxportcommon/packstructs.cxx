#include <string.h>
#include "pcodesim.h"
#include "packstructs.h"
#include "ICVersion.h"

#define PACKED __attribute__((packed))
#pragma pack(1)

struct processtableinfo{
	short startaddress;
	short numberofprocesses;
} PACKED;

struct targetinfo{

	/* target info header */
	char i;
	char c;
	char infoversionmajor;
	char infoversionminor;
	char lengthoficsection; //include padding
	char flags;
	char smallestaddressdatabite;
	char datapointerbytes;
	char smallestaddressproggrambits;
	char programpointerbits;

	/* pcode info section */
	short numberofbytesforsection; //padding after
	char p;
	char cc;
	char pcodeversionmajor;
	char pcodeversionminor;

	int featuresmask;
	short processtableinfopointer;
	short zeroarray;
	int targetspecificfeatures;

	char targetname[17];

	char padding;

	/* data memory info section */

	short datasectionlength;
	short memorystart;
	short memorysize;
	int memoryusage;
	char memorytype; /* ram */
	char memorytypedetail;
	char memorytimetoaccess;
	char memoryreserved;

	short memoryuistart;
	short memoryuisize;
	int memoryuiusage;
	char memoryuitype; /* ram */
	char memoryuitypedetail;
	char memoryuitimetoaccess;
	char memoryuireserved;
} PACKED;


//Stupid gameboy is stupid littlendian so all the stupid
//vars > 1byte are flipped in the following structs.
//I hate computers.
const struct processtableinfo ProcessInfo = {
	(short)0x00C3               //start of processtable
		,(short)0x2000                              // number of processes
};

const struct targetinfo Inf = {
	'I','C',
		1, //infoversionmajor
		0, // infoversionminor
		10, //length of this section
		3, //flags (first bit is on for bigendian, off for little. GBA is little)		
		8, //smallestaddressdatabite
		16, //datapointerbytes
		8, //smallestaddressproggrambits
		16, //programpointerbits
		// pcode info section 
		0x2400, //number of bytes for this section (including padding)
		'P','C',
	        IC_VERSION_MAJOR, //pcodeversionmajor
		IC_VERSION_MINOR, // pcodeversionminor
		0, //featuresmask
		0xC800, //processtableinfopointer
		0, //zeroarray
		0, //targetspecificfeatures
		//targetname[]
	{ 'X',
	'p',
	'o',
	'r',
	't',
	' ',
	'R',
	'o',
	'b',
	'o',
	't',
	' ',
	' ',
	' ',
	' ',
	' ',
	0 }
	,
		0, //padding
		// data memory info section 	
		0x1A00, //datasectionlength
		//MEMORY is main memory array
		0x0002 , //memorystart
		0xFEFF, //memorysize
		0x03000000, //memoryusage
		0, //memorytype
		0, //memorytypedetail
		1, //memorytimetoaccess
		0, //memoryreserved
		// MEMORYUI is user interaction
		0x00c2, //memoryuistart
		0xFF00, //memoryuisize
		0x0C000000, //memoryuiusage
		0, //memoryuitype
		0, //memoryuitypedetail
		1, //memoryuitimetoaccess		
		0 //memoryuireserved
};

void CopyPackStructsToICMem()
{
#ifndef USE_OWN_CLASS
  memcpy(&mem[0x64], (void*)&Inf, sizeof(Inf));	
  memcpy(&mem[0xC8], (void*)&ProcessInfo, sizeof(ProcessInfo));
#endif /* #ifndef USE_OWN_CLASS */
}
