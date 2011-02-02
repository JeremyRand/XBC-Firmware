#include <gba.h>
#include "visioncontext.h"
#include "textcontext.h"
#include "btnstate.h"
#include <textdisp.h>

#ifdef DEBUG
#define DBG(x) x
#else
#define DBG(x) 
#endif

/////////////////////////////////////////////////////////////////////////
// Vision Display Options
void
CVisionDispOptions::PrintSettings()
{
  printf("Vision Display Options:\n");
  for(int i=0; i<DV_MODELS; i++) {
    printf("  enable[%d] = %s\n", i, enable[i]?"true":"false");
  }
  printf("  showBlobs = %s\n", showBlobs?"true":"false");
  printf("  showBlobText = %s\n", showBlobText?"true":"false");
  printf("  minBlobArea = %d\n", minBlobArea);
  printf("  sortBlobs = %s\n", sortBlobs?"true":"false");
  printf("  doBlobAxes = %s\n", doBlobAxes?"true":"false");

}
/////////////////////////////////////////////////////////////////////////
// Vision Context
CVisionContext::~CVisionContext() 
{
}

void 
CVisionContext::SetDispOptions(const CVisionDispOptions &options)
{
  m_options = options;
  //printf("Setting RenderMode to %d...", m_options.mode);
  m_vision.SetRenderMode(m_options.mode);
  //printf("done\nSetting RenderEnables...");
  for(int i=0; i<DV_MODELS; i++) {
    m_vision.SetRenderEnable(i, m_options.enable[i]);
  }
  SMoments::computeAxes = m_options.doBlobAxes;
  //printf("done\n");
}

bool 
CVisionContext::Setup()
{
  return(Focus());
}

bool 
CVisionContext::Blur()
{
  m_vision.SetRenderMode(RM_NONE);
  return(true);
}

bool 
CVisionContext::Focus()
{
  // Setup display registers for displaying video
  m_vision.SetupDisplay();
  // Setup display options to make sure render mode and enables are
  // setup properly
  SetDispOptions(m_options);

  // Ignore whatever buttons are currently being pressed
  m_visionContextIgnoreBtns=true;
  return(true);
}

bool 
CVisionContext::Shutdown()
{
  return(Blur());
}

void 
CVisionContext::CycleRenderMode()
{
    // Cycle between RM_RAW, RM_PROCESSED, and RM_COMBINED
    if(m_options.mode == RM_RAW) {
      DBG(printf("Showing processed video\n"));
      m_options.mode = RM_PROCESSED;
    }
    else if(m_options.mode == RM_PROCESSED) {
      DBG(printf("Showing combined video\n"));
      m_options.mode = RM_COMBINED;
    }
    else {
      DBG(printf("Showing raw video\n"));
      m_options.mode = RM_RAW;
    }
    SetDispOptions(m_options);
}

bool 
CVisionContext::Process()
{
  static CBtnState btnState;

  // The following is to allow whatever button is being pressed when 
  // the context comes into focus to be ignored
  if(m_visionContextIgnoreBtns) {
    btnState.PollKeys();
    m_visionContextIgnoreBtns=false;
  }

  btnState.PollKeys();

  if(btnState.KeyHit(CBtnState::A_BUTTON)) {
    CycleRenderMode();
  } else if(btnState.KeyHit(CBtnState::B_BUTTON)) {
    // Exit vision processing
    return(false);
  } else if(btnState.KeyHit(CBtnState::SELECT_BUTTON)) {
    // Push text display onto top of stack 
    CTextContext::Push();
    return(true);
  }
  return(true);
}

bool 
CVisionContext::Push(CVision &vision, 
		     const CVisionDispOptions &options)
{
  CVisionContext *vc = dynamic_cast<CVisionContext*>(DispContextStackSingleton.GetCurrentContext());

  // Check if top of stack is already CVisionContext
  if(vc!=NULL) {
    // Top is already CVisionContext, just set options
    vc->SetDispOptions(options);
    return(false);
  }
  // Create a vision context and push it onto the stack above this menu
  CVisionContext *tContext = new CVisionContext(vision, options );
  DispContextStackSingleton.PushContext(tContext);
  return(true);
}

bool 
CVisionContext::Pop()
{
  // Check if top of stack is already CVisionContext
  if(dynamic_cast<CVisionContext*>(DispContextStackSingleton.GetCurrentContext())) {
    // Top is vision context, pop it
    DispContextStackSingleton.PopContext();
    return(true);
  }
  else {
    // Top is not vision context, do nothing
    return(false);
  }
}
