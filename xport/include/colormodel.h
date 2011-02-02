#ifndef CCOLORMODEL_H
#define CCOLORMODEL_H

#define MAX_H 360
#define MAX_S 224
#define MAX_V 224

#define MAX_HRANGE 120

// Support for setting and getting value by index
#define HMIN_INDEX 0
#define HMAX_INDEX 1

#define SMIN_INDEX 2
#define SMAX_INDEX 3

#define VMIN_INDEX 4
#define VMAX_INDEX 5

#define MODEL_MININDEX 0
#define MODEL_MAXINDEX 5

class CColorModelHSV {
public:
  CColorModelHSV(short ihMin=0, short ihMax=0,
		 short isMin=0, short isMax=0,
		 short ivMin=0, short ivMax=0);
  void Set(short ihMin, short ihMax,
	   short isMin, short isMax,
	   short ivMin, short ivMax);
  void Limit();

  void Print() const;

  static inline int HueDist(int hmin, int hmax) {
    return((hmax - hmin + MAX_H) % MAX_H);
  }

  inline int GetHueRange() {
    return(HueDist(hMin, hMax));
  }

  static inline const char *GetIndexName(int index) {
    switch(index) {
    case HMIN_INDEX: return("hMin"); break;
    case SMIN_INDEX: return("sMin"); break;
    case VMIN_INDEX: return("vMin"); break;

    case HMAX_INDEX: return("hMax"); break;
    case SMAX_INDEX: return("sMax"); break;
    case VMAX_INDEX: return("vMax"); break;
    default:
      return("BAD");
    }
  }

  inline int GetValue(int index) {
    switch(index) {
    case HMIN_INDEX: return(hMin); break;
    case SMIN_INDEX: return(sMin); break;
    case VMIN_INDEX: return(vMin); break;

    case HMAX_INDEX: return(hMax); break;
    case SMAX_INDEX: return(sMax); break;
    case VMAX_INDEX: return(vMax); break;
    default:
        return(-1);
    }
  }

  inline bool SetValue(int index, short val) {
    switch(index) {
    case HMIN_INDEX: hMin = val; break;
    case SMIN_INDEX: sMin = val; break;
    case VMIN_INDEX: vMin = val; break;

    case HMAX_INDEX: hMax = val; break;
    case SMAX_INDEX: sMax = val; break;
    case VMAX_INDEX: vMax = val; break;
    default:
        return(false);
    }
    return(true);
  }

public:
  short hMin, hMax;
  short sMin, sMax;
  short vMin, vMax;
};

//////////////////////////////////////////////////////////////////////
// Color Model Selection HSV

// This class encapsulates the mapping of HSV onto a plane.  Hue maps
// to Y, SV to X.  The hue values go from hTop a the top and wrap to
// hTop-1 at the bottom.  Saturation and value are maximum at xCenter
// and decrease to sMin and vMin at the left and right edges
// respectively.  The pattern looks like below:
//              x=0         x=xCenter   x=xMax
//              s=sMin      s=MAX_S     s=MAX_S
//     y=0      v=MAX_V     v=MAX_V     v=vMin
//     h=hTop    -------------------------       => X
//              |                         |
//     h=MAX_H  |                         |
//     h=0      |            R            |
//              |            a            |
//              |            i            |
//  <= White    |            n            |  Black =>
//              |            b            |
//              |            o            |
//              |            w            |
//     y=maxY   |                         |
//     h=hTop-1  -------------------------
//
//              ||
//              \/ Y

// The most vivid, canonical, representation of any Hue value is at
// (S=MAX_S, V=MAX_V), which is marked "Rainbow" above.  These are the
// values mapped to the xCenter column.  For doing color segmentation
// you're going to get the best results the closer you are to this
// column where both saturation and value are at maximum.  

///////////////////////////////////////////////////////////////////////
// Bounding boxes and Color models

// A CColorModelHSV can be thought of as specifying a bounding box in
// the color selection plane if you constrain the models to have sMax
// = MAX_S and vMax = MAX_V.  This prevents you from segmenting on
// black or white, but is exactly what you want for training on
// primary colors, which is really the most practical way to use color
// segmentation in relatively unstructured lighting and environmental
// conditions.

// The top left corner of the bounding box corresponds to (m.hMin, m.sMin)
// with m.sMin<=sMin mapping to X=0, m.sMin = MAX_S mapping to X=xCenter.

// The the bottom right corner corresponds to (m.hMax, m.vMin).  with
// m.vMin<=vMin mapping to X=maxX and m.vMin = MAX_V mapping to X=xCenter.

// Moving either edge towards the center line constrains the range of
// accepted color values to only include more vivid colors (ie only
// accept things that are more like AstroBrights paper).  If
// everything you want is beig accepted by so is a lot of other junk
// you don't want, move the corners closer to the center.

// Moving either edge towards the edges loosens the model to include
// less vivid colors.  Moving the left edge out accepts colors that
// are closer to pastel than what is currently accepted.  Move the
// right edge out accepts darker colors than what is currently
// accepted.

// Moving the top and bottom edges up and down changes the range of
// hues accepted by the model.  The implementation of xrc_lut
// constrains the range of accepted hues to be no larger than
// MAX_HRANGE, which is currently set to 120.  This means that the hue
// values represented by the top and bottom edges of the bounding box
// can never be further apart than 120.  

///////////////////////////////////////////////////////////////////////
// Training strategy

// It's probably easiest to do training by opening up the S and V
// ranges by moving the side edges outwards, opening up the top and
// bottom edges as far as they go (MAX_HRANGE), then moving the whole
// range up and down until it includes what you want to accept.  Then
// close down the top and bottom edges until they're as close together
// as they go before cutting out part of what you want to keep.

// After you have the top and bottom set up well, start moving the
// side edges closer to the center until you have cut out everything
// you want to get rid of.

///////////////////////////////////////////////////////////////////////
// Configuration parameters

// xMax, xCenter, yMax:
//   These define the demensions of the selection plane in x, y space
//   and the x location of the "Rainbow".  These should be chosen to
//   correspond to the x and y dimensions of the color reference image
//   you want to generate/use.  xCenter is the column of the Rainbow
//   and can be anywhere between 0 and xSize

// hTop:
//   Hue should really be thought of as an angle since the closesness
//   of the colors represented by 0 and MAX_H as just as similar to
//   one another as 0 and 1.  To map it onto a plane you have to
//   arbitrarily cut two closely related colors apart from one
//   another and put them at opposite ends of the plane, making
//   them less convenient to use.  hTop determines what hue value
//   gets treated this way, so you you should choose hTop to be a
//   color that is less likely to be useful for your application...

// sMin, vMin:
//   Regardless of Hue, at (S=0, V=MAX_V) you get white and at
//   (S=MAX_S, V=0) you get black.  The closer you get to the
//   edges, the less useful the values are going to be for color
//   segmentation, either because they're too dark (too far to the
//   right), or too washed out (too far to the left).  The hue
//   values you get from RGB->HSV conversion are less and less well
//   defined, the closer you get to either edge.  At some point as
//   you get towards the edges it becomes useless to try to train
//   on those values and there's not really any point including
//   them in the model selection plane.  However, what values you
//   want to give up at are very dependent on your application and
//   your camera, so you can choose where to put the cutoff by
//   choosing values for the sMin and vMin parameters.  If you 
//   wanted to include everything you would set both to 0.  If you 
//   wanted to only provide very primary colors you would set them to
//   values approaching MAX_S or MAX_V.

class CHsvPlane {
public:
  CHsvPlane(int ixSize, int iySize, int ixCenter=-1, 
	       int ihTop=0, int isMin=0, int ivMin=0);
  ~CHsvPlane() {}

  ///////////////////////////////////////////////////////////////////
  // Ranges
  //   These represent the number of disccrete values of S or V which
  //   are mapped across x 
  int SRange() const { return(MAX_S-sMin-1); }
  int VRange() const { return(MAX_V-vMin-1); }

  ///////////////////////////////////////////////////////////////////
  // HSV -> XY

  // Given a hue value, return the Y position in the selection plane
  // corresponding to it.  
  int HToY(unsigned short h) const;

  // LEFT
  // Given a saturation value, return the X position in the selection
  // plane corresponding to it.  This will be a value from 0 and
  // xCenter.  If s<=sMin this will return 0.
  int SToX(unsigned short s) const;

  // RIGHT
  // Given a value, return the X position in the selection plane
  // corresponding to it.  This will be a value from xCenter to
  // xSize-1.  If v<=vMin this will return xSize-1.
  int VToX(unsigned short v) const;

  // Given a model, return the (x, y) locations of the corners of the
  // bounding box.  See "Bounding boxes and Color models" section
  // above for details.
  void ModelToBBox(const CColorModelHSV &model,
		   int &left, int &top, int &right, int &bottom) const;

  // Given an individual (h, s, v) value return the row and the left
  // and right column bounds for it.  The longer this segment is, the
  // less suitable the color is for color segmentation.  The shorter,
  // the better.
  void HSVToSegment(unsigned short &h, unsigned char &s, unsigned char &v,
		    int &y, int &leftX, int &rightX) const;

  ///////////////////////////////////////////////////////////////////
  // XY -> HSV 

  // Given an (x, y) position in the selection plane, return the
  // corresponding HSV value.  This is a well defined mapping and can
  // be used to generate color reference images
  void XYToHSV(int x, int y,
	       unsigned short &h, unsigned char &s, unsigned char &v) const;

  ///////////////////////////////////////////////////////////////////
  // Hue wrap support
  int WrapHue(int hinit, int mod, bool *trunc=0);
  int HueDist(int hmin, int hmax);

public:
  // X, Y parameters
  int xSize;
  int ySize;
  int xCenter;

  // HSV parameters
  int hTop;
  int sMin;
  int vMin;
};

/////////////////////////////////////////////////////////////////////////
// Parameters to use with CHSVPlane to generate a model selection
// image suitable for display to the right of the live video feed from
// the camera on the GBA screen.

// Model selection image support
#define MSI_XSIZE 62
#define MSI_YSIZE 158

// Put the max s and v line right of center because low saturation values
// don't show up as well...
#define MSI_XCENTER 36

// Cut off at purple
#define MSI_START_H 312

// Originally was 50 but couldn't get stuff that was dark enough, 
// lowered to 20
#define MSI_MIN_S 20
#define MSI_MIN_V 30
#endif
