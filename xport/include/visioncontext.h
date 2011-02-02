#ifndef CVISIONCONTEXT_H
#define CVISIONCONTEXT_H

#include "dispcontext.h"
#include "vision.h"

#define TRACK_MINAREA_INDEX		0
#define TRACK_ENABLE_CH0_INDEX		1
#define TRACK_ENABLE_CH1_INDEX 		2
#define TRACK_ENABLE_CH2_INDEX 		3

#define TRACK_RENDER_COLOR_CH0_INDEX	4
#define TRACK_RENDER_COLOR_CH1_INDEX	5
#define TRACK_RENDER_COLOR_CH2_INDEX	6
#define TRACK_ENABLE_ORIENT_INDEX	7

class CVisionDispOptions {
public:
  CVisionDispOptions(ERenderMode initMode=RM_COMBINED, bool initEnable=true, 
		     bool initShowBlobs=true, bool initShowBlobText=false, 
		     int initMinBlobArea=0, bool initSortBlobs=true):
    mode(initMode), showBlobs(initShowBlobs), 
    showBlobText(initShowBlobText), minBlobArea(initMinBlobArea), 
    sortBlobs(initSortBlobs)
  {
    for(int i=0; i<DV_MODELS; i++) {
      enable[i] = initEnable;
    }
  }

  void EnableAll() {
    for(int i=0; i<DV_MODELS; i++) enable[i]=true;
  }
  void DisableAll() {
    for(int i=0; i<DV_MODELS; i++) enable[i]=false;
  }
  void SetEnable(int channel, bool value) {
    if(channel>=0 && channel<DV_MODELS) {
      enable[channel]=value;
    }
  }
  void SetEnableMask(unsigned short mask) {
    for(int i=0; i<DV_MODELS; i++) enable[i]=mask & (1<<i);
  }

  int GetEnableMask() const {
    int ret=0;
    for(int i=0; i<DV_MODELS; i++) {
      if(enable[i]) {
	ret |= (1<<i);
      }
    }
    return(ret);
  }

  static const char *GetIndexName(int index) {
    switch(index) {
    case TRACK_MINAREA_INDEX: return("Minarea");

    case TRACK_ENABLE_CH0_INDEX: return("Enable_Ch0");
    case TRACK_ENABLE_CH1_INDEX: return("Enable_Ch1");
    case TRACK_ENABLE_CH2_INDEX: return("Enable_Ch2");
    default:
      return("BAD");
    }
  }

  inline int GetValue(int index) {
    switch(index) {
    case TRACK_MINAREA_INDEX: return(minBlobArea);

    case TRACK_ENABLE_CH0_INDEX: return(enable[0]);
    case TRACK_ENABLE_CH1_INDEX: return(enable[1]);
    case TRACK_ENABLE_CH2_INDEX: return(enable[2]);
    default:
        return(-1);
    }
  }

  inline bool SetValue(int index, short val) {
    switch(index) {
    case TRACK_MINAREA_INDEX: minBlobArea = val; break;

    case TRACK_ENABLE_CH0_INDEX: enable[0] = val; break;
    case TRACK_ENABLE_CH1_INDEX: enable[1] = val; break;
    case TRACK_ENABLE_CH2_INDEX: enable[2] = val; break;
    default:
        return(false);
    }
    return(true);
  }

  void PrintSettings();

public:
  ERenderMode mode;
  bool enable[DV_MODELS];
  bool showBlobs;
  bool showBlobText;
  int  minBlobArea;
  bool sortBlobs;
  bool doBlobAxes;
};

// This context holds onto a reference to an associated vision object,
// but does not own the storage for the vision.  It is up to the user
// to make sure that the context is freed before its vision, and that
// the vision is eventually freed.
class CVisionContext : public IDispContext
{
public:
  CVisionContext(CVision &vision, 
		 const CVisionDispOptions &options = 
		 CVisionDispOptions()):
    m_vision(vision), m_options(options) {}
  virtual ~CVisionContext();

  virtual bool Setup();	    // Newly created context, now top of stack
  virtual bool Blur();	  // Newer context on top of stack, lose focus
  virtual bool Focus();	  // Newer context(s) done, top of stack again
  virtual bool Shutdown();     // Context done, being popped off stack

  virtual bool Process();	// Do a quantum of work and return

  virtual void SetDispOptions(const CVisionDispOptions &options);
  void GetDispOptions(CVisionDispOptions &options) const {
    options = m_options;
  }
  void CycleRenderMode();

  static bool Push(CVision &vision, 
		   const CVisionDispOptions &options = 
		   CVisionDispOptions()); 
 				// Push vision context onto top of
				// display stack if top is not already
				// showing vision
  static bool Pop();		// Pop off top of display stack if 
				// top is currently showing vision

protected:
  CVision &m_vision;
  CVisionDispOptions m_options;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  bool m_visionContextIgnoreBtns;
};

#endif
