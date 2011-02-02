#define DFX_FILLBUFFER		0
#define DFX_WRITEBUFFER		1
#define DFX_ERASE			2
#define DFX_LOCK			3
#define DFX_UNLOCK			4
#define DFX_READ			5
#define DFX_GETBUFFERSIZE	6
#define DFX_GETSECTORSIZE	7
#define DFX_BUFFERCHECKSUM	8
#define DFX_EXECUTE			9
#define DFX_VERSION			10
#define DFX_PRINT			11
#define DFX_SETMODE			12
#define DFX_SETLED			13

#define DFX_NORMAL_MODE					0xc001f00d
#define DFX_PRIVILEGED_MODE				0xbaadf00d

enum EXferMode
	{
	EXM_NONE,
	EXM_NORMAL,
	EXM_PRIVILEGED
	};
