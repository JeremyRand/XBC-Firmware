#ifndef _VISION_H
#define _VISION_H

#include <iinterrupt.h>
#include "camera.h"
#include "xrc_lut.h"
#include "colormodel.h"
#include "blob.h"

#define DV_MODELS					3
#define DV_LUT_WIDTH				9
#define DV_LUT_DEPTH				(1<<10)

#define DV_VISION_BASE				0x9ffe000

// do not change
#define DV_GRAB_COLS				176
#define DV_GRAB_ROWS				143

#define DV_GBA_SCREEN_WIDTH			240
#define DV_GBA_SCREEN_HEIGHT		160

#define DV_COLOR_REJECT_DEFAULT		5

#define DV_SEGMENT_BUFFERS			2
#define DV_SEGMENT_BUFFER_SIZE		0x800

// grab config bits	
#define DV_GRAB_CONFIG_BANK			0x0001 // will be removed
#define DV_GRAB_CONFIG_QUEUE_COUNT	0x000f
#define DV_GRAB_CONFIG_GRAB_MODE	0x0030
#define DV_GRAB_CONFIG_HSYNC		0x0040
#define DV_GRAB_CONFIG_VSYNC		0x0080
#define DV_GRAB_CONFIG_BGROUND		0x0100
#define DV_GRAB_CONFIG_MATCH		0x0200
#define DV_GRAB_CONFIG_NOGAMMA		0x0400
#define DV_GRAB_CONFIG_RESERVED		0x7800
#define DV_GRAB_CONFIG_INT_STATUS	0x8000

// color config bits
#define DV_COLOR_CONFIG_QUEUE_COUNT	0x00ff
#define DV_COLOR_CONFIG_REJECT_OFFS	8
#define DV_COLOR_CONFIG_REJECT		0x0700
#define DV_COLOR_CONFIG_INT_STATUS	0x8000

// render frames
#define DV_RENDER_CONTINUOUS		0xffffffff

#define DV_BUILD_COLOR(r, g, b)	(r&0x1f) | ((g&0x1f)<<5) | ((b&0x1f)<<10)

class CInterruptCont;

// This class holds interleave and other count variables
// It is double-buffered so it can be synchronized with the next frame
class CBufferedCount
	{
public:
	CBufferedCount()
		{
		m_buffered = false;
		m_valBuffered = 0;
		m_val = 0;
		m_count = 0;
		}

	inline void Reset(unsigned long val=0)
		{
		m_valBuffered = val;
		m_buffered = true;
		}

	inline unsigned long GetCount()
		{
		// if we are buffered, return the future (not current) value
		if (m_buffered)  
			return 0;
		else
			return m_count;
		}
		
	inline void Update()
		{
		// transfer new value 
		if (m_buffered)
			{
			m_val = m_valBuffered;
			m_count = 0;
			m_buffered = false;
			}
		}

	unsigned long m_val;
	unsigned long m_count;

private:
	bool m_buffered;
	unsigned long m_valBuffered;
	};

// Rendering modes
enum ERenderMode
	{
	RM_NONE,		// rendering off
	RM_RAW,			// render camera video as-is 
	RM_PROCESSED,	// render processed (filtered) video 
	RM_COMBINED,	// render pixels not in model(s) as RAW, and 
	                // pixels that are in models as PROCESSED
	RM_COMBINED_REVERSE // render model pixels as raw, and background pixels as black
	};

///////////////////////////////////////////////////////////////////////
// Multi-channel blob assembler to allow having CVision fill in a blob
// structure that the caller holds on to and doesn't have to worry
// about changing
class CMultiChannelBlobAssembler {
public:
  CMultiChannelBlobAssembler(unsigned short mask= (1<< DV_MODELS) - 1, 
			     bool doSort=true);
  ~CMultiChannelBlobAssembler();

  unsigned int GetFrameNum() {
    return(m_frameNum);
  }

  void EnableAll() {
    m_mask = (1 << DV_MODELS) - 1;
  }
  void DisableAll() {
    m_mask = 0;
  }
  void SetEnable(unsigned short channel, bool value) {
    if(channel<DV_MODELS) {
      if(value) 
	m_mask |= 1 << channel;
      else 
	m_mask &= ~(1 << channel);
    }
  }
  bool GetEnable(unsigned short channel) {
    if(channel<DV_MODELS) {
      return(m_mask & (1 << channel));
    }
    else {
      return(0);
    }
  }
  void SetEnableMask(unsigned short mask) {
    m_mask = mask;
  }
  unsigned short GetEnableMask() {
    return(m_mask);
  }
  unsigned short GetNumChannels() {
    return(DV_MODELS);
  }

  // The pointer returned by this is only valid until the next time
  // the segments are cleared!  Do not free this pointer, it is owned 
  // by the blob assembler.
  CBlob *GetChannelBlobs(unsigned short channel);

  //////////////////////////////////////////////////////////////////
  // These are called by CVision to fill in the data
  void Reset();
  void StartFrame(unsigned int frameNum);
  void CopyToRawSegmentBuffer(unsigned int frameNum, 
			      unsigned long *bufPtr, int n);
  void ProcessRawSegmentBuffer();
  void AddRawSegment(unsigned long compSegment);
  void EndFrame();

protected:
  unsigned short m_mask;	// Indicates which channels to process
  unsigned int   m_frameNum;	// Frame number of current dataset
  bool           m_doSort;	// True if should sort the blobs
  CBlobAssembler m_blobAssembler[DV_MODELS]; 

    unsigned long  *m_segmentBuffer;
    int             m_numRawSegments;
};

///////////////////////////////////////////////////////////////////////
class CVision : public IInterrupt // IIinterrupt is a interrupt callback interface
	{
public:
	CVision(CInterruptCont *pIntCont, unsigned long baseAddr=DV_VISION_BASE, 
		unsigned char frameVector=17, unsigned char lineQueueVector=18, unsigned char segmentQueueVector=19, 
		unsigned short version=2);
	virtual ~CVision();

	void SetInterruptCont(CInterruptCont *pIntCont);
	  CInterruptCont *GetInterruptCont() const {
	    return(m_pIntCont);
	  }

	  //////////////////////////////////////////////////////////
	  // Color model

	  // Number of parts for partial model setting.  Shouldn't be
	  // larger than CXRCLut::rSize, which is 32
	  enum {MODEL_PARTS=32};

	  // Hue from 0 to 360
	  // 0:   red
	  // 60:  yellow
	  // 120: green
	  // 180: cyan
	  // 240: blue
	  // 300: magenta
	  
	  int UploadModel(unsigned char modelIndex,
			  unsigned short hMin, unsigned short hMax, 
			  unsigned char sMin, unsigned char sMax,
			  unsigned char vMin, unsigned char vMax);
	  

	  // Computes parts from partBegin to partEnd-1
	  int UploadModelPartial(unsigned char modelIndex,
				 unsigned short hMin, unsigned short hMax, 
				 unsigned char sMin, unsigned char sMax,
				 unsigned char vMin, unsigned char vMax,
				 int partBegin, int partEnd);

	  // These are the same as above, but take a CColorModelHSV for 
	  // more convenient use
	  int UploadModel(unsigned char index, const CColorModelHSV &model);
	  int UploadModelPartial(unsigned char index, 
				 const CColorModelHSV &model, 
				 int partBegin, int partEnd);

	  // This fills in the most recent values for the color model
	  // set for a given channel index.  Be aware that if
	  // UploadModelPartial was used then only the most recent
	  // slice is represented here, and that it may not accurately
	  // reflect the entire state of the LUT ram.
	  bool GetCurrentModel(unsigned char index, 
			       CColorModelHSV &model) {
	      if (index>=DV_MODELS) return false;
	      model = m_model[index];
	      return(true);
	  }

	  ////////////////////////////////////////////////////////////
	  // Blob support

	  // Get the frame number of the frame currently being
	  // received.  If segment acquisition is enabled subtract 1
	  // to get the most recent completed frame.
	  unsigned int GetCurrFrameNum() {
	    return(m_currFrameNum);
	  }

	// Control whether we grab segments or not
	int SetSegmentGrab(bool enable);

	// Returns true if new segment data is ready to be processed
	// Call this if you want to check to see if GetBlobs will busy-wait
	bool NewSegmentData();

	// This returns a pointer into the CMultiChannelBlobAssembler
	// inside the CVision object, and is not safe to use if
	// more than one part of the code is using blob processing.
	unsigned int GetBlobs(CBlob *blob[DV_MODELS]);

	// This builds blobs into a CMultiChannelBlobAssembler passed
	// in by the user, so it is safe for use y multiple parts of
	// the code which don't know about each other
	unsigned int BuildBlobs(CMultiChannelBlobAssembler &blobAssm);

	  ////////////////////////////////////////////////////////////
	  // Display support
	int SetRenderMode(ERenderMode mode);
	int SetGamma(bool gamma);

	// Set the number of frames to render and then stop
	// Pass in DV_RENDER_CONTINUOUS to render continuously 
	// Pass in 1 to render a single frame and stop
	int SetRenderFrames(unsigned long frames);

	// Get the number of frames rendered
	// Use this to poll for a single frame grab, for example
	unsigned long GetRenderFrames();
    
	// This sets the render color of a given model in processed render mode.
	// If bit 15 of 'color' is set, camera data is passed thru as-is.
	int SetRenderColor(unsigned char modelIndex, unsigned short color);
	unsigned short GetRenderColor(unsigned char modelIndex) const;

	// These support enabing and disabling display of a given
	// model without affecting the registered color
	int SetRenderEnable(unsigned char modelIndex, bool enable);
	bool GetRenderEnable(unsigned char modelIndex) const;

	int SetRenderOffset(unsigned short x, unsigned short y);

	// 0x100 scale is 1.0, 0x200 is 2.0 etc
	int SetRenderScaling(unsigned short x, unsigned short y);

	// 0 -> render each frame
	// 1 -> render every other frame
	// 2 -> render every 3rd frame, etc
	int SetRenderInterleave(unsigned char interleave);

	// Context switching support 
	void SetupDisplay();

	// camera object
	CCamera m_camera;

protected:
	// from IInterrupt interface
	virtual void Interrupt(unsigned char vector);
	
	int LockSegmentBuffer(unsigned char *index);
	int UnlockSegmentBuffer(unsigned char index);

	void HandleFrameInterrupt();
	void HandleLineQueueInterrupt();
	void HandleSegmentQueueInterrupt();

	ERenderMode m_renderMode;

	CColorModelHSV m_model[DV_MODELS];
	CXRCLut m_lut[DV_MODELS];
	
	// Pointer to interrupt controller
	CInterruptCont *m_pIntCont;  

	long m_base;
	unsigned char m_frameVector;
	unsigned char m_lineQueueVector;
	unsigned char m_segmentQueueVector;

	bool m_segmentQueueEnable;

	unsigned short *m_renderVram;
	unsigned short m_renderOffset;
	CBufferedCount m_renderFrameCount;
	CBufferedCount m_renderInterleaveCount;
	volatile unsigned int m_currFrameNum;

	// fpga registers
	volatile unsigned long *m_lutMemory;
	volatile unsigned short *m_grabMemory;
	volatile unsigned short *m_grabDequeue;
	volatile unsigned short *m_colorRenderReg;
	volatile unsigned short *m_colorConfig;
	volatile unsigned short *m_grabConfig;
	volatile unsigned short *m_dummy1;
	volatile unsigned short *m_dummy2;
	volatile unsigned short *m_dummy3;
	volatile unsigned long *m_dummy4;

	// Blob segment queue: reading m_blobSegment automatically dequeues segment value.
	// Each 32 bit value consists of (starting from LSB):
	// 3 bits: model index
	// 9 bits: row
	// 10 bits: start column
	// 10 bits: end column
	volatile unsigned long *m_segmentQueue;

	// segment buffer variables
	unsigned long *m_segmentBuffer[DV_SEGMENT_BUFFERS];
	unsigned char m_segmentBufferGrabIndex;
	unsigned short m_segmentWriteIndex;
	unsigned short m_segments[DV_SEGMENT_BUFFERS]; 

	unsigned char m_flagLockSegBuffer[DV_SEGMENT_BUFFERS];
	unsigned char m_flagGrabSegFinished[DV_SEGMENT_BUFFERS];
	unsigned int m_segBufferFrameNum[DV_SEGMENT_BUFFERS];

	// Holds most recently assembled blobs for each color channel
	CMultiChannelBlobAssembler m_blobAssembler;

	// Holds render color most recently set for each channel to 
	// support enable/disable
	unsigned short m_colorRender[DV_MODELS];
	// Holds whether render is enabled or not for a given channel
	unsigned short m_enableRender[DV_MODELS];

	unsigned short m_version;

	};


#endif

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//   End:
