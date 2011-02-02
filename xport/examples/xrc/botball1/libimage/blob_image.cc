#include "blob_image.h"
#ifdef GBA
#include <textdisp.h>
#endif

//#define CHECK

const unsigned int raw_blob_colors[] = {0x00ff00, 0x0000ff, 0xff0000, 
					0xff00ff, 0x00ffff, 0xffff00, 
					0x7fff7f, 0x7f7fff, 0xff7f7f, 
					0x077f07, 0x07077f, 0x7f0707, 
					0x7f077f, 0x077f7f, 0x7f7f07, 
					0x3f7f3f, 0x3f3f7f, 0x7f3f3f};

const int num_colors = sizeof(raw_blob_colors)/sizeof(unsigned int);
TPix blob_colors[num_colors];
bool bcolors_converted = false;

void convert_bcolors()
{
  if(bcolors_converted) return;
  for(int i=0; i< num_colors; i++) {
#ifdef GBA
    unsigned char r = (raw_blob_colors[i] >> 16) & 0xff;
    unsigned char g = (raw_blob_colors[i] >> 8) & 0xff;
    unsigned char b = (raw_blob_colors[i]) & 0xff;
    
    blob_colors[i] = Image::make_rgb(r,g,b);
#else 
    blob_colors[i] = raw_blob_colors[i];
#endif
  }
  bcolors_converted=true;
}

void draw_blob(Image &dest, CBlob *blob, const TPix &color,
               int coord_shift, bool showseg, bool showell)
{
  //TPix white = 0xffffff;
  TPix accent_color = color;
  SMomentStats stats;
  blob->moments.GetStats(stats);

  if(showseg && CBlob::recordSegments) {
    SLinkedSegment *lseg= blob->firstSegment;
    while(lseg!=NULL) {
      dest.draw_line((int)lseg->segment.startCol,
                     (int)lseg->segment.row,
                     (int)lseg->segment.endCol,
                     (int)lseg->segment.row,
                     color);
      lseg = lseg->next;
    }

    // Calculate accent color to be inverse of segment color
    unsigned char r,g,b;
    Image::read_rgb(color,r,g,b);
    accent_color = Image::make_rgb(255-r, 255-g, 255-b);
  }
  if(showell) {
    float cscale = 2.0 * coord_shift;
    draw_ellipse(dest, stats.centroidX/cscale, 
		 stats.centroidY/cscale, 
		 stats.angle,
                 stats.majorDiameter/(2.0 + cscale), 
		 stats.minorDiameter/(2.0 + cscale),
                 accent_color);
  }
  dest.draw_cross((int)stats.centroidX>>coord_shift, 
		  (int)stats.centroidY>>coord_shift,
                  3, accent_color);

  short left, top, right, bottom;
  blob->getBBox(left, top, right, bottom);

  dest.draw_box((int)left>>coord_shift, (int)top>>coord_shift, 
		(int)right>>coord_shift, (int)bottom>>coord_shift, color);
}

void image_blobify(const Image &src, CBlobAssembler &bass, 
		   bool recordSegments, int row_lim, int col_lim)
{
  SSegment seg;
  // When not actively in a segment, set seg.row to norow.
  const short norow = 0x1ff;

  seg.row = norow;

  // Setup to record segments or not
  CBlob::recordSegments = recordSegments;
  SMoments::computeAxes= false;
  CBlob::testMoments= false;

  // Clear any existing blobs
  bass.Reset();

  // row_lim and col_lim allow processing of top left region of an image
  if(row_lim==0) {
    row_lim = src.nrows;
  }
  if(col_lim==0) {
    col_lim = src.ncols;
  }

  // Process the image into segments and feed to the blob assembler
  for (int y= 0; y< row_lim; y++) {
    for (int x= 0; x< col_lim; x++) {
      TPix val = src.pixel(x, y);

      // Do blob processing
      if(val!=0) {
        // This pixel is part of a segment
        if(seg.row == norow) {
          // Just starting a new segment
          seg.row = y;
          seg.startCol = x;
          seg.endCol = x;
        }
        else {
          // Continuing an existing segment
          seg.endCol = x;
        }
      }
      else {
        // This pixel is not part of a segment, check if one just ended
        if(seg.row != norow) {
          // Segment just ended, add to blob assembler
          bass.Add(seg);
          // Mark segment as invalid
          seg.row = norow;
        }
      }
    }
    // Row just ended, close off any active segments
    if(seg.row != norow) {
      // Add to blob assembler
      bass.Add(seg);
      // Mark segment as invalid
      seg.row = norow;
    }
  }

  // Tell the blob assembler we're done and process the finished blobs
  bass.EndFrame();
  return;
}

// Call this with a completed blob assembler
void show_blobs(Image &dest, CBlob *blob, 
		int minarea, int coord_shift, 
		bool showseg, bool showbars, bool showell, 
		bool showtext, bool showbox)
{
  SMomentStats stats;
  TPix white = Image::make_rgb(0xff, 0xff, 0xff);
  TPix color;
  int i=0;

  convert_bcolors();

  if(showtext) 
    printf("----\n"); 
  while(blob!=NULL) {
#ifdef CHECK
    printf("blob %x\n", blob);
#endif
    blob->moments.GetStats(stats);
    if(stats.area >= minarea) {
      if(i>=num_colors) {
	color = white;
      }
      else {
	color = blob_colors[i];
      }
      if(showtext) {
	printf("Blob %d: area %d, centroid (%f, %f)\n", 
	       i++, stats.area, stats.centroidX, stats.centroidY);
      }
      if (showbox)
	draw_blob(dest, blob, color, coord_shift, showseg, showell);
      
      if(showbars && i<=num_colors) {
	dest.draw_fillrect((i-1)*10, 0, i*10, 10, color);
      }
    }
#ifdef CHECK
    if (blob->next==blob)
      {
	printf("cycle found (show_blobs)\n");
	while(1);
      }
#endif
    blob = blob->next;
  }
}

//-------------------------------------------------------------------------
const short sintab[512] = {0, 3, 6, 9, 13, 16, 19, 
                          22, 25, 28, 31, 34, 38, 41, 
                          44, 47, 50, 53, 56, 59, 62, 
                          65, 68, 71, 74, 77, 80, 83, 
                          86, 89, 92, 95, 98, 101, 104, 
                          107, 109, 112, 115, 118, 121, 123, 
                          126, 129, 132, 134, 137, 140, 142, 
                          145, 147, 150, 152, 155, 157, 160, 
                          162, 165, 167, 170, 172, 174, 177, 
                          179, 181, 183, 185, 188, 190, 192, 
                          194, 196, 198, 200, 202, 204, 206, 
                          207, 209, 211, 213, 215, 216, 218, 
                          220, 221, 223, 224, 226, 227, 229, 
                          230, 231, 233, 234, 235, 237, 238, 
                          239, 240, 241, 242, 243, 244, 245, 
                          246, 247, 248, 248, 249, 250, 250, 
                          251, 252, 252, 253, 253, 254, 254, 
                          254, 255, 255, 255, 256, 256, 256, 
                          256, 256, 256, 256, 256, 256, 256, 
                          256, 255, 255, 255, 254, 254, 254, 
                          253, 253, 252, 252, 251, 250, 250, 
                          249, 248, 248, 247, 246, 245, 244, 
                          243, 242, 241, 240, 239, 238, 237, 
                          235, 234, 233, 231, 230, 229, 227, 
                          226, 224, 223, 221, 220, 218, 216, 
                          215, 213, 211, 209, 207, 206, 204, 
                          202, 200, 198, 196, 194, 192, 190, 
                          188, 185, 183, 181, 179, 177, 174, 
                          172, 170, 167, 165, 162, 160, 157, 
                          155, 152, 150, 147, 145, 142, 140, 
                          137, 134, 132, 129, 126, 123, 121, 
                          118, 115, 112, 109, 107, 104, 101, 
                          98, 95, 92, 89, 86, 83, 80, 
                          77, 74, 71, 68, 65, 62, 59, 
                          56, 53, 50, 47, 44, 41, 38, 
                          34, 31, 28, 25, 22, 19, 16, 
                          13, 9, 6, 3, 0, -3, -6, 
                          -9, -13, -16, -19, -22, -25, -28, 
                          -31, -34, -38, -41, -44, -47, -50, 
                          -53, -56, -59, -62, -65, -68, -71, 
                          -74, -77, -80, -83, -86, -89, -92, 
                          -95, -98, -101, -104, -107, -109, -112, 
                          -115, -118, -121, -123, -126, -129, -132, 
                          -134, -137, -140, -142, -145, -147, -150, 
                          -152, -155, -157, -160, -162, -165, -167, 
                          -170, -172, -174, -177, -179, -181, -183, 
                          -185, -188, -190, -192, -194, -196, -198, 
                          -200, -202, -204, -206, -207, -209, -211, 
                          -213, -215, -216, -218, -220, -221, -223, 
                          -224, -226, -227, -229, -230, -231, -233, 
                          -234, -235, -237, -238, -239, -240, -241, 
                          -242, -243, -244, -245, -246, -247, -248, 
                          -248, -249, -250, -250, -251, -252, -252, 
                          -253, -253, -254, -254, -254, -255, -255, 
                          -255, -256, -256, -256, -256, -256, -256, 
                          -256, -256, -256, -256, -256, -255, -255, 
                          -255, -254, -254, -254, -253, -253, -252, 
                          -252, -251, -250, -250, -249, -248, -248, 
                          -247, -246, -245, -244, -243, -242, -241, 
                          -240, -239, -238, -237, -235, -234, -233, 
                          -231, -230, -229, -227, -226, -224, -223, 
                          -221, -220, -218, -216, -215, -213, -211, 
                          -209, -207, -206, -204, -202, -200, -198, 
                          -196, -194, -192, -190, -188, -185, -183, 
                          -181, -179, -177, -174, -172, -170, -167, 
                          -165, -162, -160, -157, -155, -152, -150, 
                          -147, -145, -142, -140, -137, -134, -132, 
                          -129, -126, -123, -121, -118, -115, -112, 
                          -109, -107, -104, -101, -98, -95, -92, 
                          -89, -86, -83, -80, -77, -74, -71, 
                          -68, -65, -62, -59, -56, -53, -50, 
                          -47, -44, -41, -38, -34, -31, -28, 
                          -25, -22, -19, -16, -13, -9, -6, 
                          -3};

const short costab[512] = {256, 256, 256, 256, 256, 256, 255, 
                           255, 255, 254, 254, 254, 253, 253, 
                           252, 252, 251, 250, 250, 249, 248, 
                           248, 247, 246, 245, 244, 243, 242, 
                           241, 240, 239, 238, 237, 235, 234, 
                           233, 231, 230, 229, 227, 226, 224, 
                           223, 221, 220, 218, 216, 215, 213, 
                           211, 209, 207, 206, 204, 202, 200, 
                           198, 196, 194, 192, 190, 188, 185, 
                           183, 181, 179, 177, 174, 172, 170, 
                           167, 165, 162, 160, 157, 155, 152, 
                           150, 147, 145, 142, 140, 137, 134, 
                           132, 129, 126, 123, 121, 118, 115, 
                           112, 109, 107, 104, 101, 98, 95, 
                           92, 89, 86, 83, 80, 77, 74, 
                           71, 68, 65, 62, 59, 56, 53, 
                           50, 47, 44, 41, 38, 34, 31, 
                           28, 25, 22, 19, 16, 13, 9, 
                           6, 3, 0, -3, -6, -9, -13, 
                           -16, -19, -22, -25, -28, -31, -34, 
                           -38, -41, -44, -47, -50, -53, -56, 
                           -59, -62, -65, -68, -71, -74, -77, 
                           -80, -83, -86, -89, -92, -95, -98, 
                           -101, -104, -107, -109, -112, -115, -118, 
                           -121, -123, -126, -129, -132, -134, -137, 
                           -140, -142, -145, -147, -150, -152, -155, 
                           -157, -160, -162, -165, -167, -170, -172, 
                           -174, -177, -179, -181, -183, -185, -188, 
                           -190, -192, -194, -196, -198, -200, -202, 
                           -204, -206, -207, -209, -211, -213, -215, 
                           -216, -218, -220, -221, -223, -224, -226, 
                           -227, -229, -230, -231, -233, -234, -235, 
                           -237, -238, -239, -240, -241, -242, -243, 
                           -244, -245, -246, -247, -248, -248, -249, 
                           -250, -250, -251, -252, -252, -253, -253, 
                           -254, -254, -254, -255, -255, -255, -256, 
                           -256, -256, -256, -256, -256, -256, -256, 
                           -256, -256, -256, -255, -255, -255, -254, 
                           -254, -254, -253, -253, -252, -252, -251, 
                           -250, -250, -249, -248, -248, -247, -246, 
                           -245, -244, -243, -242, -241, -240, -239, 
                           -238, -237, -235, -234, -233, -231, -230, 
                           -229, -227, -226, -224, -223, -221, -220, 
                           -218, -216, -215, -213, -211, -209, -207, 
                           -206, -204, -202, -200, -198, -196, -194, 
                           -192, -190, -188, -185, -183, -181, -179, 
                           -177, -174, -172, -170, -167, -165, -162, 
                           -160, -157, -155, -152, -150, -147, -145, 
                           -142, -140, -137, -134, -132, -129, -126, 
                           -123, -121, -118, -115, -112, -109, -107, 
                           -104, -101, -98, -95, -92, -89, -86, 
                           -83, -80, -77, -74, -71, -68, -65, 
                           -62, -59, -56, -53, -50, -47, -44, 
                           -41, -38, -34, -31, -28, -25, -22, 
                           -19, -16, -13, -9, -6, -3, 0, 
                           3, 6, 9, 13, 16, 19, 22, 
                           25, 28, 31, 34, 38, 41, 44, 
                           47, 50, 53, 56, 59, 62, 65, 
                           68, 71, 74, 77, 80, 83, 86, 
                           89, 92, 95, 98, 101, 104, 107, 
                           109, 112, 115, 118, 121, 123, 126, 
                           129, 132, 134, 137, 140, 142, 145, 
                           147, 150, 152, 155, 157, 160, 162, 
                           165, 167, 170, 172, 174, 177, 179, 
                           181, 183, 185, 188, 190, 192, 194, 
                           196, 198, 200, 202, 204, 206, 207, 
                           209, 211, 213, 215, 216, 218, 220, 
                           221, 223, 224, 226, 227, 229, 230, 
                           231, 233, 234, 235, 237, 238, 239, 
                           240, 241, 242, 243, 244, 245, 246, 
                           247, 248, 248, 249, 250, 250, 251, 
                           252, 252, 253, 253, 254, 254, 254, 
                           255, 255, 255, 256, 256, 256, 256, 
                           256};

void draw_ellipse(Image &dest, float x, float y, float angle,
                  float major_axis, float minor_axis,
                  const TPix &color)
{
  int step=1;
  int ix = (int)x, iy = (int)y;

  int a = (int)((angle/(2.0*M_PI))*512.0);
  if(a<0) {
    a+=512;
  }
  for(int p=0; p<512; p+=step) {
    // This is the x,y location at this angle around the ellipse if
    // the ellipse were centered at zero and had theta=0;
    int ce_x = (int)(major_axis * costab[p]);
    int ce_y = (int)(minor_axis * sintab[p]);

    // This calculates the actual x,y location by rotating ellipse_p
    // by theta, then adding the center x,y
    int p_x = ix + ((ce_x * costab[a] - ce_y * sintab[a])>>16);
    int p_y = iy + ((ce_y * costab[a] + ce_x * sintab[a])>>16);

    if (p_x >= 0 && p_x < dest.ncols && p_y >= 0 && p_y < dest.nrows)
      dest.pixel(p_x, p_y) = color;
  }
}
