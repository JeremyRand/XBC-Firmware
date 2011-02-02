#include "vision.h"
#include <intcont.h>
#include <textdisp.h>
#include <gba.h>
#include <regbits.h>
#include <string.h>
#include <display.h>

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

//#define HALT_ON_ERROR
//#define VALGRIND
//#define BLOB_TIMER

#ifdef BLOB_TIMER
#include "simptimer.h"
#endif

CVision::CVision(CInterruptCont *pIntCont, unsigned long baseAddr,
	unsigned char frameVector, unsigned char lineQueueVector, unsigned char segmentQueueVector,
	unsigned short version) :
	m_camera(baseAddr+sizeof(short)),
	m_pIntCont(NULL)
	{
	unsigned char i;

	// initialize member variables
	m_version = version;
	m_base = baseAddr;
    m_frameVector = frameVector;
	m_lineQueueVector = lineQueueVector;
	m_segmentQueueVector = segmentQueueVector;
	m_renderVram = (unsigned short *)GBA_BASE_VRAM;
	m_segmentBufferGrabIndex = 0;
	m_segmentWriteIndex = 0;

	// setup fpga vision registers
	m_grabConfig = (volatile unsigned short *)baseAddr;
	m_lutMemory = (volatile unsigned long *)baseAddr + 0x400;
	m_grabMemory = (volatile unsigned short *)baseAddr + 0x400;
	m_grabDequeue = &m_grabMemory[0x200];
	m_colorRenderReg = &m_grabConfig[6];
	m_colorConfig = &m_grabConfig[16];
	m_segmentQueue = (volatile unsigned long *)&m_grabConfig[18];
	m_segmentQueueEnable = false;

	// set segment reject filter 
	*m_colorConfig &= ~DV_COLOR_CONFIG_REJECT;
	*m_colorConfig |= DV_COLOR_REJECT_DEFAULT<<DV_COLOR_CONFIG_REJECT_OFFS; 

	// reset color keys
	for (i=0; i<DV_MODELS; i++)
		SetRenderColor(i, 0);

	// allocate segment memory
	for (i=0; i<DV_SEGMENT_BUFFERS; i++) {
	    m_segmentBuffer[i] = new unsigned long[DV_SEGMENT_BUFFER_SIZE];
	    DBG(printf("m_segmentBuffer[%d] = 0x%x\n", m_segmentBuffer[i]));
	}

	// set render mode to render nothing by default
	SetRenderMode(RM_NONE);

	// set offset for rendering full screen
	SetRenderOffset(0, 0);

	// set interleaves, etc
	SetRenderFrames(DV_RENDER_CONTINUOUS);
	SetRenderInterleave(0);

	// register and enable interrupts
	if (m_camera.Connected())
		SetInterruptCont(pIntCont);
	}

CVision::~CVision()
	{
	unsigned char i;

	if (m_pIntCont)
		{
		m_pIntCont->UnRegister(m_frameVector);
		m_pIntCont->UnRegister(m_lineQueueVector);
		m_pIntCont->UnRegister(m_segmentQueueVector);
		}

	// restore scaling
	SetRenderScaling(0x100, 0x100);

	// free memory
	for (i=0; i<DV_SEGMENT_BUFFERS; i++)
		delete [] m_segmentBuffer[i];
	}

void CVision::SetupDisplay() 
	{
	// set high-res graphics mode
	// This used to do:
	//   GBA_REG_DISPCNT = GBA_BG_MODE_3 | GBA_BG2_ENABLE<<8;
	// the problem is that that bashed the other bits, such as the 
	// sprite enable bits.  The following does the same thing, but 
	// also preserves the other bits.  Go ahead and turn off the
	// other background layers though since we're going to write
	// over their memory anyway
	BFSET(GBA_REG_DISPCNT, videoMode, GBA_BG_MODE_3);
	BFSET(GBA_REG_DISPCNT, drawBG2, 1);
	BFSET(GBA_REG_DISPCNT, drawBG0, 0);
	BFSET(GBA_REG_DISPCNT, drawBG1, 0);
	BFSET(GBA_REG_DISPCNT, drawBG3, 0);
	BFSET(GBA_REG_DISPCNT, forceScreenBlank, 0);

	// scale for full screen display
	SetRenderScaling((DV_GRAB_COLS<<8)/DV_GBA_SCREEN_WIDTH, 
		(DV_GRAB_ROWS<<8)/DV_GBA_SCREEN_HEIGHT);

	// Invalidate the global display font and palettes
	CDisplay::SetCurrPalette(NULL);
	CDisplay::SetCurrDispFont(NULL);
	}

void CVision::SetInterruptCont(CInterruptCont *pIntCont)
	{
	m_pIntCont = pIntCont;
	if (m_pIntCont)
		{
		// register interrupts
		m_pIntCont->Register(this, m_frameVector);
		m_pIntCont->Register(this, m_lineQueueVector);
		m_pIntCont->Register(this, m_segmentQueueVector);

		// enable frame interrupt
		m_pIntCont->Unmask(m_frameVector);
		// enable segment queue interrupt 
		SetSegmentGrab(true);
		}
	}

int CVision::UploadModelPartial(unsigned char modelIndex,
                                unsigned short hMin, unsigned short hMax, 
                                unsigned char sMin, unsigned char sMax,
                                unsigned char vMin, unsigned char vMax,
                                int partBegin, int partEnd)
{

  if (modelIndex>=DV_MODELS) return -1;

  m_model[modelIndex].hMin=hMin;
  m_model[modelIndex].hMax=hMax;
  m_model[modelIndex].sMin=sMin;
  m_model[modelIndex].sMax=sMax;
  m_model[modelIndex].vMin=vMin;
  m_model[modelIndex].vMax=vMax;

  int rBegin= CXRCLut::rSize*partBegin/MODEL_PARTS;
  int rEnd= CXRCLut::rSize*partEnd/MODEL_PARTS;
  
  m_lut[modelIndex].ComputeFromHSVCubePartial(hMin, hMax, sMin, sMax,
					      vMin, vMax, rBegin, rEnd);

  int lutBegin= DV_LUT_DEPTH*partBegin/MODEL_PARTS;
  int lutEnd= DV_LUT_DEPTH*partEnd/MODEL_PARTS;
  for (int i=lutBegin; i<lutEnd; i++)
    m_lutMemory[i] = m_lut[0].lut[i] | 
      ((unsigned long)(m_lut[1].lut[i])<<DV_LUT_WIDTH) | 
      ((unsigned long)(m_lut[2].lut[i])<<DV_LUT_WIDTH*2);
  
  return 0;
}

// recode the following to call UploadModelPartial(1,2,3,4,5,6,0,MODEL_PARTS)

int CVision::UploadModel(unsigned char modelIndex,
                         unsigned short hMin, unsigned short hMax, 
                         unsigned char sMin, unsigned char sMax,
                         unsigned char vMin, unsigned char vMax)
	{
	int i;

	if (modelIndex>=DV_MODELS)
		return -1;

	m_model[modelIndex].hMin=hMin;
	m_model[modelIndex].hMax=hMax;
	m_model[modelIndex].sMin=sMin;
	m_model[modelIndex].sMax=sMax;
	m_model[modelIndex].vMin=vMin;
	m_model[modelIndex].vMax=vMax;

	m_lut[modelIndex].ComputeFromHSVCube(hMin, hMax, sMin, sMax, vMin, vMax);
	for (i=0; i<DV_LUT_DEPTH; i++)
		m_lutMemory[i] = m_lut[0].lut[i] | 
			((unsigned long)(m_lut[1].lut[i])<<DV_LUT_WIDTH) | 
			((unsigned long)(m_lut[2].lut[i])<<DV_LUT_WIDTH*2);

	return 0;
	}

// These are the same as above, but take a CColorModelHSV for 
// more convenient use
int CVision::UploadModel(unsigned char index, const CColorModelHSV &model)
{
  return(UploadModel(index, model.hMin, model.hMax, 
		     model.sMin, model.sMax,
		     model.vMin, model.vMax));
}

int CVision::UploadModelPartial(unsigned char index, 
				const CColorModelHSV &model, 
				int partBegin, int partEnd)
{
  return(UploadModelPartial(index, model.hMin, model.hMax, 
			    model.sMin, model.sMax,
			    model.vMin, model.vMax, 
			    partBegin, partEnd));
}

int CVision::SetRenderMode(ERenderMode mode)
	{

	switch(mode)
		{
		case RM_NONE:
			// disable line queue interrupt
			if (m_pIntCont)
				m_pIntCont->Mask(m_lineQueueVector);
			break;

		case RM_RAW:
			// set graphics mode
			SetupDisplay();
			// set background and match mbits
			*m_grabConfig |= DV_GRAB_CONFIG_BGROUND;
			*m_grabConfig |= DV_GRAB_CONFIG_MATCH;
			break;

		case RM_COMBINED: // Start combined showing raw
			// set graphics mode
			SetupDisplay();
			*m_grabConfig |= DV_GRAB_CONFIG_BGROUND;
			*m_grabConfig &= ~DV_GRAB_CONFIG_MATCH;
			break;

        case RM_COMBINED_REVERSE:
			if (m_version==1)
				return -1;
			// set graphics mode
			SetupDisplay();
			*m_grabConfig &= ~DV_GRAB_CONFIG_BGROUND;
			*m_grabConfig |= DV_GRAB_CONFIG_MATCH;
			break;

		case RM_PROCESSED:
			// set graphics mode
			SetupDisplay();
			*m_grabConfig &= ~DV_GRAB_CONFIG_BGROUND;
			*m_grabConfig &= ~DV_GRAB_CONFIG_MATCH;
			break;

		default:
			return -1;
		}
	m_renderMode = mode;
	return 0;
	}

int CVision::SetGamma(bool gamma)
	{
	if (m_version==1)
		return -1;

	if (gamma)
		*m_grabConfig &= ~DV_GRAB_CONFIG_NOGAMMA;
	else
		*m_grabConfig |= DV_GRAB_CONFIG_NOGAMMA;

	return 0;
	}

int CVision::SetRenderColor(unsigned char modelIndex, unsigned short color)
	{
	if (modelIndex>=DV_MODELS)
		return -1;

	m_colorRender[modelIndex] = color;
	if(m_enableRender[modelIndex]) {
	  m_colorRenderReg[modelIndex] = color;
	}
	return 0;
	}

unsigned short CVision::GetRenderColor(unsigned char modelIndex) const
	{
	if (modelIndex>=DV_MODELS)
		return 0;

	return(m_colorRender[modelIndex]);
	}

int CVision::SetRenderEnable(unsigned char modelIndex, bool enable)
	{
	if (modelIndex>=DV_MODELS)
		return -1;

	m_enableRender[modelIndex] = enable;
	if(enable) {
	  m_colorRenderReg[modelIndex] = m_colorRender[modelIndex];
	}
	else {
	  m_colorRenderReg[modelIndex] = 0;
	}
	// Set enable for the built-in blob assembler to match rendering
	m_blobAssembler.SetEnable(modelIndex, enable);
	return 0;
	}

bool CVision::GetRenderEnable(unsigned char modelIndex) const
	{
	if (modelIndex>=DV_MODELS)
		return false;

	return(m_enableRender[modelIndex]);
	}

bool CVision::NewSegmentData()
	{
	return m_flagGrabSegFinished[0] || m_flagGrabSegFinished[1];
	}

int CVision::LockSegmentBuffer(unsigned char *index)
	{
	// can't lock buffer if segment queue is not enabled
	if (!m_segmentQueueEnable)
		return -1;

	// wait for segment buffer to become available
	// this should block only when we are running over framerate
	while (!NewSegmentData());
	if (m_flagGrabSegFinished[0])
		{
		m_flagLockSegBuffer[0] = true;
		*index = 0;
		}
	else if (m_flagGrabSegFinished[1])
		{
		m_flagLockSegBuffer[1] = true;
		*index = 1;
		}
	// no default case	

	return 0;
	}

int CVision::UnlockSegmentBuffer(unsigned char index)
	{
	if (index>=DV_SEGMENT_BUFFERS)
		return -1;

	// invalidate current buffer
	m_flagGrabSegFinished[index] = false;
	// unlock
	m_flagLockSegBuffer[index] = false;
	return 0;
	}

unsigned int CVision::GetBlobs(CBlob *blob[DV_MODELS])
	{
	unsigned char i;

	if (!m_segmentQueueEnable)
		return ((unsigned int)-1);

	unsigned int fnum = BuildBlobs(m_blobAssembler); 

	for (i=0; i<DV_MODELS; i++)
		blob[i] = m_blobAssembler.GetChannelBlobs(i);
	return(fnum);
	}

unsigned int CVision::BuildBlobs(CMultiChannelBlobAssembler &blobAssm)
{
    unsigned char bufIndex;
    unsigned short len;
    unsigned long *segmentBuffer;
    unsigned int fnum;

    // If we wanted to be sure not to block we could check if
    // there could be newer data than is already in blob
    // assembler.  The newest data we could have is m_currFrameNum
    // - 1, since m_currFrameNum isn't done yet, so there's no
    // point in doing processing if m_currFrameNum -
    // m_blobFrameNum < 2.  If not, just return since
    // m_blobAssember already has as good as we're going to get at
    // this point
    //if(m_currFrameNum - m_blobFrameNum < 2) {
    //return(m_blobFrameNum);
    //}

    // Tell blob assembler that we're about it invalidate the 
    // raw segment buffer
    blobAssm.Reset();

    // There could be newer data, lock buffer, etc
    LockSegmentBuffer(&bufIndex);
    len = m_segments[bufIndex];
    segmentBuffer = m_segmentBuffer[bufIndex];
    fnum = m_segBufferFrameNum[bufIndex];

    if(len>DV_SEGMENT_BUFFER_SIZE) {
	printf("error (BuildBlobs): len=%d > %d\n", 
	       len, DV_SEGMENT_BUFFER_SIZE);
#ifdef HALT_ON_ERROR
	while(1);
#endif
	UnlockSegmentBuffer(bufIndex);
	return(fnum);
    }

    // Copy all the segments into m_segmentBuffer and unlock
    // the global buffer
    blobAssm.CopyToRawSegmentBuffer(fnum, segmentBuffer, len);
    UnlockSegmentBuffer(bufIndex);

    blobAssm.ProcessRawSegmentBuffer();
    return(fnum);
}

int CVision::SetRenderFrames(unsigned long frames)
	{
	m_renderFrameCount.Reset(frames);
	return 0;
	}

unsigned long CVision::GetRenderFrames()
	{
	return m_renderFrameCount.GetCount();
	}

int CVision::SetRenderOffset(unsigned short x, unsigned short y)
	{
	m_renderOffset = x + y*DV_GBA_SCREEN_WIDTH;
	return 0;
	}

int CVision::SetRenderScaling(unsigned short x, unsigned short y)
	{
	GBA_REG_BG2PA = x;
	GBA_REG_BG2PD = y;
	return 0;
	}

int CVision::SetRenderInterleave(unsigned char interleave)
	{
	m_renderInterleaveCount.Reset(interleave);
	return 0;
	}

int CVision::SetSegmentGrab(bool enable)
	{
	unsigned int i;
	if (m_pIntCont)
		{
		if (enable)
			{
			// reset buffer flags
			for (i=0; i<DV_SEGMENT_BUFFERS; i++)
				UnlockSegmentBuffer(i);
			// enable interrupts
			m_pIntCont->Unmask(m_segmentQueueVector);
			}
		else
			m_pIntCont->Mask(m_segmentQueueVector);

		// only set to true if we have an interrupt controller 
		m_segmentQueueEnable = enable;
		}

	return 0;
	}

void CVision::HandleFrameInterrupt()
	{
	unsigned short val;

	// if we are rendering to the LCD
	if (m_renderMode!=RM_NONE)
		{
		// reset vram pointer
		m_renderVram = (unsigned short *)GBA_BASE_VRAM + m_renderOffset;
		
		// empty line queue to keep old lines out of the next frame
		while(*m_grabConfig&DV_GRAB_CONFIG_QUEUE_COUNT)
			val = *m_grabDequeue;
		
		// deal with interleave, end of frame
		if (m_renderFrameCount.m_count<m_renderFrameCount.m_val &&
			m_renderInterleaveCount.m_count==0) 
			{
			m_renderFrameCount.m_count++;
			m_renderInterleaveCount.m_count = m_renderInterleaveCount.m_val;
			}
		if (m_renderInterleaveCount.m_count)
			m_renderInterleaveCount.m_count--;
		
		// update variables
		m_renderFrameCount.Update();
		m_renderInterleaveCount.Update();
		
		// unmask or mask line queue interrupt for next frame
		if (m_renderFrameCount.m_count<m_renderFrameCount.m_val &&
		    m_renderInterleaveCount.m_count==0) {
			m_pIntCont->Unmask(m_lineQueueVector);
			// If in RM_COMBINED mode toggle the state of
			// *m_grabConfig & DV_GRAB_CONFIG_BGROUND. So that
			// if the last frame was RAW the next will be
			// processed, or vice versa.  When RM_COMBINED
			// is implemented in the xilinx this will no
			// longer be necessary
			if(m_version==1 && m_renderMode==RM_COMBINED) {
				*m_grabConfig ^= DV_GRAB_CONFIG_BGROUND;
			}
		}
		else
			m_pIntCont->Mask(m_lineQueueVector);
		}
	
	// if we are grabbing segments
	if (m_segmentQueueEnable)
		{
		// empty rest of segment queue
		HandleSegmentQueueInterrupt();
		
		// set number of segments
		m_segments[m_segmentBufferGrabIndex] = m_segmentWriteIndex;
		// set finished bit
		m_flagGrabSegFinished[m_segmentBufferGrabIndex] = true;
		// set frame number
		m_segBufferFrameNum[m_segmentBufferGrabIndex] = m_currFrameNum;
		
		// select grab buffer for next frame
		// first, try to cycle through buffers
		if (m_segmentBufferGrabIndex==1 && !m_flagLockSegBuffer[0])
			m_segmentBufferGrabIndex = 0;
		else if (m_segmentBufferGrabIndex==0 && !m_flagLockSegBuffer[1])
			m_segmentBufferGrabIndex = 1;
		// if this fails, look for a buffer that isn't locked 
		else if (!m_flagLockSegBuffer[0])
			m_segmentBufferGrabIndex = 0;
		else if (!m_flagLockSegBuffer[1])
			m_segmentBufferGrabIndex = 1;
		// there is no default case -- we have no way to accommodate two locked buffers
		
		// invalidate this buffer
		m_flagGrabSegFinished[m_segmentBufferGrabIndex] = false;
		// reset write index
		m_segmentWriteIndex = 0;
		}
	// Increment current frame number
	m_currFrameNum++;
	}

void CVision::HandleLineQueueInterrupt()
	{
	unsigned short val;
	unsigned long *buffer;

	while(*m_grabConfig&DV_GRAB_CONFIG_QUEUE_COUNT)
		{
		// make sure we don't go outside the 80k frame buffer space
		if (m_renderVram>=GBA_BASE_VRAM+0x14000)
			break;
		// setup DMA channel 3 to transfer a line of camera video to video memory
		GBA_REG_DMA3SAD = (unsigned long)m_grabMemory;
		GBA_REG_DMA3DAD = (long)m_renderVram;
		GBA_REG_DMA3CNT_L = DV_GRAB_COLS;
		GBA_REG_DMA3CNT_H = 0x8000;
		
		// dequeue line
		val = *m_grabDequeue;
		
		// advance vram pointer
		m_renderVram += DV_GBA_SCREEN_WIDTH;

		// empty segment queue
		// spend a fixed amount of time to prevent the line queue from overflowing. 
		if (m_segmentQueueEnable &&
			(*m_colorConfig&DV_COLOR_CONFIG_QUEUE_COUNT)>=30 && m_segmentWriteIndex+30<=DV_SEGMENT_BUFFER_SIZE)
			{
			buffer = m_segmentBuffer[m_segmentBufferGrabIndex];
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			buffer[m_segmentWriteIndex++] = *m_segmentQueue;
			}
		}	
	}

void CVision::HandleSegmentQueueInterrupt()
	{
	unsigned char len;
	unsigned short end;
	unsigned long *buffer;

	// empty segment queue
	if ((len=*m_colorConfig&DV_COLOR_CONFIG_QUEUE_COUNT))
		{
		end = len+m_segmentWriteIndex;
		if (end>DV_SEGMENT_BUFFER_SIZE)
			end = DV_SEGMENT_BUFFER_SIZE;
		buffer = m_segmentBuffer[m_segmentBufferGrabIndex];
		for (; m_segmentWriteIndex<end; m_segmentWriteIndex++)
			buffer[m_segmentWriteIndex] = *m_segmentQueue;
		}	
	}


void CVision::Interrupt(unsigned char vector)
	{
	// end of frame interrupt
	if (vector==m_frameVector)
		HandleFrameInterrupt();

	// line queue full interrupt
	else if (vector==m_lineQueueVector)
		HandleLineQueueInterrupt();

	// segment queue full interrupt
	else if (vector==m_segmentQueueVector)
		HandleSegmentQueueInterrupt();
	}

///////////////////////////////////////////////////////////////////////
// Multi-channel blob assembler to allow having CVision fill in a blob
// structure that the caller holds on to and doesn't have to worry
// about changing
CMultiChannelBlobAssembler::CMultiChannelBlobAssembler(unsigned short mask, 
						       bool doSort) :
  m_mask(mask), m_frameNum(0), m_doSort(doSort)
{
    m_segmentBuffer = new unsigned long[DV_SEGMENT_BUFFER_SIZE];
    DBG(printf("CMCBA: m_segmentBuffer = 0x%x\n", m_segmentBuffer));
}
CMultiChannelBlobAssembler::~CMultiChannelBlobAssembler() 
{
  Reset();
  delete [] m_segmentBuffer;
}

void 
CMultiChannelBlobAssembler::Reset()
{
  for (int i=0; i<DV_MODELS; i++) {
      m_blobAssembler[i].Reset();
  }
  m_frameNum=0;
  m_numRawSegments=0;
}

void 
CMultiChannelBlobAssembler::StartFrame(unsigned int frameNum)
{
  Reset();
  m_frameNum = frameNum;
  DBG(printf("BlobAssm: StartFrame %d\n", frameNum));
}

void 
CMultiChannelBlobAssembler::EndFrame()
{
  DBG(printf("BlobAssm: EndFrame %d\n", m_frameNum));
  for (int i=0; i<DV_MODELS; i++) {
    if(!GetEnable(i)) continue;	// If not enabled, skip this channel
    m_blobAssembler[i].EndFrame();
    if(m_doSort) {
      DBG(printf("  BlobAssm: Sorting %d\n", i));
      m_blobAssembler[i].SortFinished();
      DBG(printf("  BlobAssm: Sorting %d DONE\n", i));
    }
  }
}

CBlob *
CMultiChannelBlobAssembler::GetChannelBlobs(unsigned short channel)
{
  if(channel>DV_MODELS || !GetEnable(channel)) {
    return(NULL);
  }
  else {
    return(m_blobAssembler[channel].finishedBlobs);
  }
}

void
CMultiChannelBlobAssembler::CopyToRawSegmentBuffer(unsigned int frameNum, 
						   unsigned long *bufPtr, 
						   int n)
{
    memcpy(m_segmentBuffer, bufPtr, n*sizeof(unsigned long));
    m_numRawSegments = n;
    m_frameNum = frameNum;
    DBG(printf("BlobAssm: CopyToRawSegmentBuffer %d, 0x%x, %d\n", 
	       frameNum, bufPtr, n));
}

void 
CMultiChannelBlobAssembler::ProcessRawSegmentBuffer()
{
#ifdef VALGRIND
    if (len)
	printf(">\n");
#endif
#ifdef BLOB_TIMER
    CSimpTimer timer;
    unsigned long long startTime;
    unsigned long long endTime;
    timer.GetCount(&startTime);
#endif

    // uncompress and add segments
    for (int i=0; i<m_numRawSegments; i++) {
	unsigned long compSegment = m_segmentBuffer[i];
#ifdef VALGRIND
	printf("%x\n", compSegment);
#endif
	AddRawSegment(compSegment);
    }

#ifdef VALGRIND
    if (len)
	printf("<\n");
#endif

    // finish
    EndFrame();

#ifdef BLOB_TIMER
    timer.GetCount(&endTime);
    int interval = (int)((endTime-startTime)/1000);
    printf("Time = %d msec, %d segments\n", interval, len);
#endif
}

void
CMultiChannelBlobAssembler::AddRawSegment(unsigned long compSegment)
{
	SSegment segment;
	unsigned char model;
	unsigned long inputSegment = compSegment;

	model = compSegment&0x07;
	if(model>=DV_MODELS) {
	    DBG(printf("error (AddRawSegment): %d %d %d %d [0x%x]\n", model, 
		       segment.row, segment.startCol, segment.endCol, 
		       inputSegment));
#ifdef HALT_ON_ERROR
	  while(1);
#endif
	  return;
	}
	if(!GetEnable(model) ) {
	  // Ignoring this channel
	  return;
	}

	compSegment >>= 3;
	segment.row = compSegment&0x1ff;
	compSegment >>= 9;
	segment.startCol = compSegment&0x3ff;
	compSegment >>= 10;
	segment.endCol = compSegment&0x3ff;

	if (segment.startCol>355 || segment.endCol>355 || 
	    segment.row>290 || segment.endCol<segment.startCol) {
	    DBG(printf("error (AddRawSegment): %d %d %d %d [0x%x]\n", model, 
		       segment.row, segment.startCol, segment.endCol, 
		       inputSegment));
#ifdef HALT_ON_ERROR
	  while(1);
#endif
	}
	else {
	  m_blobAssembler[model].Add(segment);
	}
}

// For emacs to interpret formatting uniformly despite dotfile differences:
//   Local variables:
//    comment-column: 40
//    c-indent-level: 4
//    c-basic-offset: 4
//   End:
