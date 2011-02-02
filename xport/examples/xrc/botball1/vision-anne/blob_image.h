#ifndef BLOB_IMAGE_H
#define BLOB_IMAGE_H

#include "xrc_image.h"
#include "blob.h"

class CVision;

void image_blobify(const Image &src, CBlobAssembler &bass, 
		   bool recordSegments, 
		   int row_lim, int col_lim, CVision *pVision);

void show_blobs(Image &dest, CBlob *blob, 
		int minarea, bool showseg,
		bool showbars, bool showell, 
		bool showtext=true);


#endif
