#ifndef BLOB_IMAGE_H
#define BLOB_IMAGE_H

#include "image.h"
#include "blob.h"

//////////////////////////////////////////////////////////////////
// Blob display

// In the following functions, coord_shift controls the power-of-two
// scaling between the blob coordinates and the image coordinates.
// This will be 0 if the screen coordinates are the same as the blob
// coordinates, which happens if you are using image_blobify on an
// image that has the same scaling as the sceen.  This will be 1 if
// you are using the segment queue, which has twice the vertical and
// horizontal resolution of the image display.

// Draw all blobs
void show_blobs(Image &dest, CBlob *blob, 
		int minarea, int coord_shift,
		bool showseg, bool showbars, bool showell, 
		bool showtext=true, bool showbox=true);

// Draw a single blob
void draw_blob(Image &dest, CBlob *blob, const TPix &color,
               int coord_shift, bool showseg, bool showell);

// Draw an ellipse
void draw_ellipse(Image &dest, float x, float y, float angle,
                  float major_axis, float minor_axis,
                  const TPix &color);

//////////////////////////////////////////////////////////////////
// Blob processing from Image input

// The following takes a thresholded image, such as is written to the
// screen when the processing mode is set to processed, converts it to
// line segments, and processes the line segments through the blob
// assembler passed in.  The row_lim and col_lim args allow processing
// the top left section of an image.  If these are not set, or if set
// to 0, the whole image is processed.
void image_blobify(const Image &src, CBlobAssembler &bass, 
		   bool recordSegments, 
		   int row_lim=0, int col_lim=0);


#endif
