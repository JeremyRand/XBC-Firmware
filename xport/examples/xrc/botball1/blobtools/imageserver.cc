// TODO:
// find game board boundaries
// do blob stats

// Test blobification with synthetic images
// Single circular blob
//   http://localhost:9001/filter.html;oneblob.ppm;blobify,showseg=1
// Left fringe
//   http://localhost:9001/filter.html;diamond_test3.ppm;blobify,showseg=1
// Top fringe
//   http://localhost:9001/filter.html;broken_ball2.ppm;blobify,showseg=1
// Blob Merging
//   http://localhost:9001/filter.html;lumpyblob.ppm;blobify,showseg=1
// Blob Separation
//   http://localhost:9001/filter.html;three_blobs.ppm;blobify,showseg=1

// Generate colorref for normal and xrc modes
// http://localhost:9001/filter.html;xrc1.ppm;colorref,xrc=0,fname=colorref.ppm
// http://localhost:9001/filter.html;xrc1.ppm;colorref,xrc=1,fname=xrc_colorref.ppm

// Test ellipse drawing, angle in degrees
//   http://localhost:9001/filter.html;xrc1.ppm;ellipse,minor_axis=50,major_axis=100,angle=45

// System includes
#include <vector>
#include <map>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>

// Local includes
#include "image.h"
#include "tipc.h"
#include "file_utils.h"
#include "string_printf.h"
#include "blob.h"

#ifndef NO_XRC
#include "xrc_lut.h"
#include "test_xrc_lut.h"
#endif

#include "hsv.h"
#include "blob_image.h"

using namespace std;

void
sleep_us (unsigned int usec)
{
  // This isn't accurate since it may be interrupted, but it should wait a 
  // little bit at least...
  struct timeval tval;
  tval.tv_sec = 0;
  tval.tv_usec = usec;
  select (0, NULL, NULL, NULL, &tval); /* will be interrupted */
  return;
}

void split(const string &in, const string &delims, vector<string> &out)
{
  string::size_type pos= 0;
  out.clear();
  while (pos < in.length()) {
    pos= in.find_first_not_of(delims, pos);
    if (pos >= in.length()) return;
    string::size_type endpos= in.find_first_of(delims, pos);
    if (endpos == string::npos) endpos= in.length();
    out.push_back(in.substr(pos, endpos-pos));
    pos= endpos;
  }
}

void get_named_args(const vector<string> &in, map<string,string> &out)
{
  vector<string> tmp;
  out.clear();
  for(unsigned int i=0; i<in.size(); i++) {
    split(in[i], "=", tmp);
    if(tmp.size()==2) {
      out[tmp[0]] = tmp[1];
    }
  }
}

void get_named_args(const vector<string> &in, map<string,int> &out)
{
  map<string, string> strmap;
  map<string, string>::const_iterator it;
  get_named_args(in, strmap);
  for(it=strmap.begin(); it != strmap.end(); it++) {
    out[it->first] = atoi(it->second.c_str());
  }
}

// This modifies in to strip off names, if any, and files names into
// the output names vector
void strip_named_args(vector<string> &in, vector<string> &names)
{
  vector<string> tmp;
  names.clear();
  names.resize(in.size());
  for(unsigned int i=0; i<in.size(); i++) {
    split(in[i], "=", tmp);
    if(tmp.size()==2) {
      names[i]=tmp[0];
      in[i]=tmp[1];
    }
    else {
      names[i]="";
    }
  }
}

// If name is in amap, return corresponding value.  Otherwise return defval.
template<class T>
T getnarg(const map<string,T> &amap, const string &name,
           const T &defval)
{
  T val;
  typename map<string, T>::const_iterator it = amap.find(name);
  if (it == amap.end()) {
    val=defval;
  }
  else {
    val=it->second;
  }
  return(val);
}
string getnarg(const map<string,string> &amap, const string &name,
               const string &defval)
{
  string val;
  map<string, string>::const_iterator it = amap.find(name);
  if (it == amap.end()) {
    val=defval;
  }
  else {
    val=it->second;
  }
  return(val);
}

class TIPC_Web_Server : public TIPC_Server {
  virtual void serve_one(int fd) {
    //
    // Receive request
    //
    string request;
    TIPC::getline(fd, request, _debug);
    //
    // Get URL
    //
    vector<string> tokens;
    split(request, " \t\r\n", tokens);
    if (tokens.size() < 2 || tokens[0] != "GET") {
      fprintf(stderr, "Malformed request: >%s<\n", request.c_str());
      close(fd);
      return;
    }
    string &url= tokens[1];
    serve_url(fd, url);
  }
  virtual void serve_url(int fd, const string &url) {
    FILE *out= fdopen(fd, "w");
    output_html_header(out);
    fprintf(out, "<b>Try overloading serve_url</b>\n");
    fclose(out);
  }
public:
  void output_html_header(FILE *out) {
    fprintf(out, "HTTP/1.0 200 OK\n");
    fprintf(out, "Content-Type: text/html\n\n");
  }
  void output_html_message(FILE *out, const string &msg) {
    output_html_header(out);
    fprintf(out, "%s\n", msg.c_str());
  }
};

void image_invert(const Image &src, Image &dest)
{
  dest.resize(src.nrows, src.ncols);
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      unsigned char r,g,b;
      src.get_rgb(x,y,r,g,b);
      r= 255-r;
      g= 255-g;
      b= 255-b;
      dest.set_rgb(x,y,r,g,b);
    }
  }
}

// The basic idea of the colorref image is to generate an image where
// the hue values go from 0 at the left to 1 at the right, and
// distributes saturation/value pairs of that hue along the top to the
// bottom.  In general, the pattern is as follows, with some modifications
// depending on whether histogram mode is used or not
//
//             h=0                h=1
//              ___________________
//   s=0, v=1  |                   |
//             |                   |
//   s=1, v=1  |                   |
//             |                   |
//   s-1, v=0  |___________________|
//

void image_create_colorref(const Image &src, Image &dest,
                           bool use_xrc, bool do_hist)
{
  int rnum=src.nrows, cnum=src.ncols;
  int rhalf=(rnum/2)-1;
  unsigned short hmax=360;
  unsigned char svmax=(use_xrc)?(224):(255);
  unsigned short h;
  unsigned char s, v;
  unsigned char r,g,b;

  // hscale * x = h
  float hscale = (float)(hmax)/(float)(cnum-1);
  // In upper half, sscale * y = s, v=svmax
  float sscale = (float)(svmax)/(float)(rhalf);
  // In lower half, vscale * ((rnum-1)-y) = v, s=svmax
  float vscale = (float)(svmax)/(float)(rhalf);

  dest.resize(src.nrows, src.ncols);

  // If we're doing a histogram, directly convert input image into
  // output image without intermediary by doing (h,s,v)->(dx,dy)
  // mapping and putting the rgb pixel values from the src image
  // into the resulting pixel location in the dest image
  if(do_hist) {
    dest.fill(0);

    for (int x= 0; x< cnum; x++) {
      for (int y= 0; y< rnum; y++) {
        src.get_rgb(x,y,r,g,b);
        // Convert rgb to hsv
#ifndef NO_XRC
        if(use_xrc) {
          RgbToHsvOV6620(r,g,b, h, s, v);
        }
        else 
#endif
	  {
          rgb_to_hsv(r,g,b, h, s, v);
        }

        if(h>hmax) {
          // Means the pixel was invalid
          continue;
        }

        // Map h, s, v to dest x, y
        int dx, dy;
        // dx is based on hue, dx = h/hscale
        dx = (int)(((float)h)/hscale);
        if(s>svmax/2) {
          // Calculate where this is in the lower half (S=1)
          dy = rnum - 1 - (int)(((float)(v))/vscale);
          dest.set_rgb(dx,dy,r,g,b);
        }

        if(v>svmax/2) {
          // Calculate where this is in the upper half (V=1)
          dy = (int)(((float)s)/sscale);
          dest.set_rgb(dx,dy,r,g,b);
        }
      }
    }
    return;
  }

  // If we're not doing a histogram, loop over all the x and y values
  // of the dest image, compute the appropriate hsv value for that
  // location, and convert that to the RGB value
  for (int x= 0; x< cnum; x++) {
    h = (unsigned short)(((float)(x))*hscale);
    for (int y= 0; y< rnum; y++) {
      if(y<rhalf) {
        s = (unsigned char)(((float)(y))*sscale);
        v = svmax;
      }
      else if(y==rhalf) {
        s = svmax;
        v = svmax;
      }
      else {
        s = svmax;
        v = (unsigned char)(((float)(rnum-1-y))*vscale);
      }
#ifndef NO_XRC
      if(use_xrc) {
        HsvToRgbOV6620(h,s,v,r,g,b);
      }
      else 
#endif
	{
        hsv_to_rgb(h,s,v, r,g,b);
      }
      dest.set_rgb(x,y,r,g,b);
    }
  }
}

void image_threshold_rgb_cube(const Image &src, Image &dest,
                              int min_r, int max_r,
                              int min_g, int max_g,
                              int min_b, int max_b)
{
  printf("Executing image_threshold_rgb_cube with args:\n");
  printf("\trmin=%d,rmax=%d,gmin=%d,gmax=%d,bmin=%d,bmax=%d\n",
         min_r, max_r, min_g, max_g, min_b, max_b);

  dest.resize(src.nrows, src.ncols);
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      unsigned char r,g,b;
      src.get_rgb(x,y,r,g,b);
      bool ok= (min_r <= r && r <= max_r &&
                min_g <= g && g <= max_g &&
                min_b <= b && b <= max_b);
      if (ok) {
        dest.set_rgb(x, y, r/2+128, g/2+128, b/2+128);
      } else {
        dest.pixel(x,y)=0;
      }
    }
  }
}

void image_truncate_rgb(const Image &src, Image &dest,
                        int r_bits, int g_bits, int b_bits)
{
  printf("Executing image_truncate_rgb with args:\n");
  printf("\trbits=%d,gbits=%d,bbits=%d\n",
         r_bits, g_bits, b_bits);
  // Mask to zero the bottom 8-n_bits bits
  unsigned char r_mask= (unsigned char)(0xff << (8-r_bits));
  unsigned char g_mask= (unsigned char)(0xff << (8-g_bits));
  unsigned char b_mask= (unsigned char)(0xff << (8-b_bits));

  dest.resize(src.nrows, src.ncols);
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      unsigned char r,g,b;
      src.get_rgb(x,y,r,g,b);
      dest.set_rgb(x,y,r&r_mask,g&g_mask,b&b_mask);
    }
  }
}

void image_threshold_hsv_cube(const Image &src, Image &dest,
                              int min_h, int max_h,
                              int min_s, int max_s,
                              int min_v, int max_v)
{
  printf("Executing image_threshold_hsv_cube with args:\n");
  printf("\thmin=%d,hmax=%d,smin=%d,smax=%d,vmin=%d,vmax=%d\n",
         min_h, max_h, min_s, max_s, min_v, max_v);

  dest.resize(src.nrows, src.ncols);
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      unsigned char r,g,b,s,v;
      unsigned short h;
      src.get_rgb(x,y,r,g,b);
      rgb_to_hsv(r,g,b,h,s,v);
      bool ok= in_hsv_cube(h,s,v, min_h,max_h, min_s,max_s, min_v,max_v);
      if (ok) {
        dest.set_rgb(x, y, r/2+128, g/2+128, b/2+128);
      } else {
        dest.pixel(x,y)=0;
      }
    }
  }
}

void image_threshold_hsv_cube_rb_lookup(const Image &src, Image &dest,
                                        int min_h, int max_h,
                                        int min_s, int max_s,
                                        int min_v, int max_v,
                                        int r_bits, int g_bits, int b_bits)
{
  int n_r= 1<<r_bits, n_g= 1<<g_bits, n_b= 1<<b_bits;

  // Create and fill min_g and max_g lookup tables
  // min_g <= g < max_g
  // Caution: min_g entries are g_bits wide (represent 0...n_g-1),
  //          max_g entries are g_bits+1 wide (represent 0...n_g)
  unsigned char *min_g= new unsigned char[n_r*n_b];
  unsigned short *max_g= new unsigned short[n_r*n_b];
  for (int r= 0; r< n_r; r++) {
    for (int b= 0; b< n_b; b++) {
      bool prev=0;
      unsigned char this_min_g= 0;  // start with inclusive range
      unsigned short this_max_g= (unsigned short) n_g;
      for (int g= 0; g< n_g; g++) {
        unsigned short h; unsigned char s,v;
        rgb_to_hsv(r<<(8-r_bits), g<<(8-g_bits), b<<(8-b_bits), h, s, v);
        bool ok= in_hsv_cube(h,s,v, min_h,max_h, min_s,max_s, min_v,max_v);
        if (g && ok != prev) {
          if (ok && this_min_g == 0) {
            // 0 to 1 transition (rising): min
            // If more than one rising edge, take the first
            this_min_g= g; // min is inclusive
          } else if (!ok) {
            // 1 to 0 transition (falling): max
            // If more than one falling edge, take the last
            this_max_g= g; // max is exclusive
          }
        }
        prev= ok;
      }
      if (this_min_g == 0 && this_max_g == n_g && !prev) {
        // All zeros.  Empty the range
        this_max_g= 0;
      }
      int lut_idx= (r<<b_bits) | b;
      min_g[lut_idx]= this_min_g;
      max_g[lut_idx]= this_max_g;
    }
  }

  dest.resize(src.nrows, src.ncols);
  // Now filter the pixels using the LUT
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      unsigned char r8,g8,b8;
      src.get_rgb(x,y,r8,g8,b8);
      unsigned char r = r8>>(8-r_bits);
      unsigned char g = g8>>(8-g_bits);
      unsigned char b = b8>>(8-b_bits);
      int lut_idx= (r<<g_bits) | b;
      bool ok= (min_g[lut_idx] <= max_g[lut_idx]) ^
               (min_g[lut_idx] <= g) ^
               (g < max_g[lut_idx]);
      if (ok) {
        dest.set_rgb(x, y, r8/2+128, g8/2+128, b8/2+128);
      } else {
        dest.pixel(x,y)=0;
      }
    }
  }
}


int dx[4]={1,0,-1,0};
int dy[4]={0,1,0,-1};

void segment_hsv_label(const Image &src, int labelno,
                       int x, int y,
                       int max_delta_h, int max_delta_s, int max_delta_v,
                       Image &labels)
{
  labels.pixel(x,y)=labelno;
  unsigned short h; unsigned char s,v;
  src.get_hsv(x,y,h,s,v);
  for (int dir= 0; dir< 4; dir++) {
    int newx= x+dx[dir], newy= y+dy[dir];
    if (0 <= newx && newx < src.ncols &&
        0 <= newy && newy < src.nrows &&
        !labels.pixel(newx,newy)) {
      unsigned short newh; unsigned char news, newv;
      src.get_hsv(newx,newy,newh,news,newv);
      if (abs((int)newh-h) <= max_delta_h &&
          abs(news-s) <= max_delta_s &&
          abs(newv-v) <= max_delta_v) {
        segment_hsv_label(src, labelno, newx, newy,
                          max_delta_h, max_delta_s, max_delta_v,
                          labels);
      }
    }
  }
}

void convert_labels(const Image &src, const Image &labels,
                    Image &dest)
{
  dest.resize(src.nrows, src.ncols);
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      if (labels.pixel(x,y) == -1) {
        dest.pixel(x,y)= 0;
      } else {
        bool border= false;
        for (int dir= 0; dir< 4; dir++) {
          int newx= x+dx[dir], newy= y+dy[dir];
          if (0 <= newx && newx < src.ncols &&
              0 <= newy && newy < src.nrows &&
              labels.pixel(newx,newy) != labels.pixel(x,y)) {
            border= true;
            break;
          }
        }
        if (border) dest.set_rgb(x,y,255,255,255);
        else {
          unsigned char r,g,b;
          src.get_rgb(x,y,r,g,b);
          dest.set_rgb(x,y,r/4+128, g/4+128, b/4+128);
        }
      }
    }
  }
}

void image_segment_hsv(const Image &src, Image &dest,
                       int max_delta_h, int max_delta_s, int max_delta_v,
                       int min_s, int min_abs_s, int min_v)
{
  printf("Executing image_segment_hsv with args:\n");
  printf("\tmax_delta_h=%d,max_delta_s=%d,max_delta_v=%d,min_s=%d,min_abs_s=%d,min_v=%d\n",
         max_delta_h, max_delta_s, max_delta_v,
         min_s, min_abs_s, min_v);

  Image labels(src.nrows, src.ncols);
  int labelno= 1;
  labels.fill(0);
  for (int y= 0; y< src.nrows; y++) {
    for (int x= 0; x< src.ncols; x++) {
      if (!labels.pixel(x,y)) {
        unsigned char r,g,b,s,v;
        unsigned short h;
        src.get_rgb(x,y,r,g,b);
        rgb_to_hsv(r,g,b,h,s,v);
        int abs_s= max(max(r,g),b) - min(min(r,g),b);
        if ((s < min_s && abs_s < min_abs_s) || v < min_v) {
          labels.pixel(x,y)=-1;
        } else {
          segment_hsv_label(src, labelno++, x, y,
                            max_delta_h, max_delta_s, max_delta_v, labels);
        }
      }
    }
  }
  convert_labels(src, labels, dest);
  printf("Found %d labels\n", labelno-1);
}

const TPix blob_colors[] = {0xff0000, 0x00ff00, 0x0000ff,
                             0xffff00, 0xff00ff, 0x00ffff,
                             0xff7f7f, 0x7fff7f, 0x7f7fff,
                             0x7f0707, 0x077f07, 0x07077f,
                             0x7f7f07, 0x7f077f, 0x077f7f,
                             0x7f3f3f, 0x3f7f3f, 0x3f3f7f,

};
const int num_colors = sizeof(blob_colors)/sizeof(TPix);

// void draw_ellipse(Image &dest, float x, float y, float angle,
//                   float major_axis, float minor_axis,
//                   TPix color)
// {
//   for(float theta = 0; theta < 2 * M_PI; theta += (M_PI/320.0)) {
//     // This is the x,y location at this angle around the ellipse if
//     // the ellipse were centered at zero and had theta=0;
//     float ce_x = major_axis * cos(theta);
//     float ce_y = minor_axis * sin(theta);

//     // This calculates the actual x,y location by rotating ellipse_p
//     // by theta, then adding the center x,y
//     int p_x = (int)(x + ce_x * cos(angle) - ce_y * sin(angle));
//     int p_y = (int)(y + ce_y * cos(angle) + ce_x * sin(angle));

//     if (p_x >= 0 && p_x < dest.ncols && p_y >= 0 && p_y < dest.nrows)
//       dest.pixel(p_x, p_y) = color;
//   }
// }

// short sintab[512], costab[512];
// bool tabinit_done=false;

// void draw_ellipse(Image &dest, float x, float y, float angle,
//                   float major_axis, float minor_axis,
//                   TPix color)
// {
//   if(!tabinit_done) {
//     for(int i=0; i< 512; i++) {
//       sintab[i] = (short)(round(sin((2.0*M_PI*i)/512.0)*256.0));
//       costab[i] = (short)(round(cos((2.0*M_PI*i)/512.0)*256.0));
//     }

// //     printf("const short costab[512] = {");
// //     int ncols = 5;
// //     int col=0;
// //     for(int i=0; i< 512; i++) {
// //       printf("%d, ", costab[i]);
// //       if(col++>ncols) {
// // 	printf("\n                           ");
// // 	col=0;
// //       }
// //     }
// //     printf("};\n\n");
//     tabinit_done=true;
//   }

//   int step=1;
//   int ix = (int)x, iy = (int)y;

//   int a = (int)((angle/(2.0*M_PI))*512.0);
//   if(a<0) {
//     a+=512;
//   }
//   for(int p=0; p<512; p+=step) {
//     // This is the x,y location at this angle around the ellipse if
//     // the ellipse were centered at zero and had theta=0;
//     int ce_x = (int)(major_axis * costab[p]);
//     int ce_y = (int)(minor_axis * sintab[p]);

//     // This calculates the actual x,y location by rotating ellipse_p
//     // by theta, then adding the center x,y
//     int p_x = ix + ((ce_x * costab[a] - ce_y * sintab[a])>>16);
//     int p_y = iy + ((ce_y * costab[a] + ce_x * sintab[a])>>16);

//     if (p_x >= 0 && p_x < dest.ncols && p_y >= 0 && p_y < dest.nrows)
//       dest.pixel(p_x, p_y) = color;
//   }
// }

// void draw_blob(Image &dest, CBlob *blob, TPix color,
//                bool showseg, bool showell)
// {
//   //TPix white = 0xffffff;
//   TPix accent_color = color;
//   SMomentStats stats;
//   blob->moments.GetStats(stats);

//   if(showseg) {
//     SLinkedSegment *lseg= blob->firstSegment;
//     while(lseg!=NULL) {
//       dest.draw_line((int)lseg->segment.startCol,
//                      (int)lseg->segment.row,
//                      (int)lseg->segment.endCol,
//                      (int)lseg->segment.row,
//                      color);
//       lseg = lseg->next;
//     }

//     // Calculate accent color to be inverse of segment color
//     unsigned char r,g,b;
//     Image::read_rgb(color,r,g,b);
//     accent_color = Image::make_rgb(255-r, 255-g, 255-b);
//   }
//   if(showell && SMoments::computeAxes) {
//     draw_ellipse(dest, stats.centroidX, stats.centroidY, stats.angle,
//                  stats.majorDiameter/2.0, stats.minorDiameter/2.0,
//                  accent_color);
//   }
//   dest.draw_cross((int)stats.centroidX, (int)stats.centroidY,
//                   3, accent_color);

//   short left, top, right, bottom;
//   blob->getBBox(left, top, right, bottom);

//   dest.draw_box((int)left, (int)top, (int)right, (int)bottom, color);
// }

// void image_blobify(const Image &src, CBlobAssembler &bass)
// {
//   SSegment seg;
//   // When not actively in a segment, set seg.row to norow.
//   const short norow = 0x1ff;

//   seg.model = 0;
//   seg.row = norow;

//   // Clear any existing blobs
//   bass.Reset();

//   // Process the image into segments and feed to the blob assembler
//   for (int y= 0; y< src.nrows; y++) {
//     for (int x= 0; x< src.ncols; x++) {
//       TPix val = src.pixel(x, y);

//       // Do blob processing
//       if(val!=0) {
//         // This pixel is part of a segment
//         if(seg.row == norow) {
//           // Just starting a new segment
//           seg.row = y;
//           seg.startCol = x;
//           seg.endCol = x;
//         }
//         else {
//           // Continuing an existing segment
//           seg.endCol = x;
//         }
//       }
//       else {
//         // This pixel is not part of a segment, check if one just ended
//         if(seg.row != norow) {
//           // Segment just ended, add to blob assembler
//           bass.Add(seg);
//           // Mark segment as invalid
//           seg.row = norow;
//         }
//       }
//     }
//     // Row just ended, close off any active segments
//     if(seg.row != norow) {
//       // Add to blob assembler
//       bass.Add(seg);
//       // Mark segment as invalid
//       seg.row = norow;
//     }
//   }

//   // Tell the blob assembler we're done and process the finished blobs
//   bass.EndFrame();
//   return;
// }

// Call this with a completed blob assembler
// void show_blobs(Image &dest, CBlob *blob, 
// 		int minarea, bool showseg,
// 		bool showbars, bool showell, 
// 		bool showtext)
// {
//   SMomentStats stats;
//   TPix white = Image::make_rgb(0xff, 0xff, 0xff);
//   TPix color;
//   int i=0;

//   if(showtext) {
//     printf("---------------------------------------------------------\n");
//     printf("Blob stats:\n");
//   }

//   while(blob!=NULL) {
//     blob->moments.GetStats(stats);
//     if(stats.area >= minarea) {
//       if(i>=num_colors) {
//         color = white;
//       }
//       else {
//         color = blob_colors[i];
//       }
//       if(showtext) {
// 	printf("\tBlob %d: area %d, centroid (%f, %f)\n", 
// 	       i++, stats.area, stats.centroidX, stats.centroidY);
// 	printf("                axis %f, aspect %f\n", 
// 	       stats.angle*180.0/M_PI, 
// 	       stats.minorDiameter / stats.majorDiameter);
//       }
//       draw_blob(dest, blob, color, showseg, showell);

//       if(showbars && i<=num_colors) {
//         dest.draw_fillrect((i-1)*10, 0, i*10, 10, color);
//       }
//     }
//     blob = blob->next;
//   }
// }

void show_image_blobify(const Image &src, Image &dest, 
			int minarea, bool showseg,
			bool showbars, bool showell, 
			bool do_sort)
{
  CBlobAssembler bass;

  // Setup to record segments or not
  CBlob::recordSegments = showseg;
  SMoments::computeAxes= true;

  dest = src;

  image_blobify(src, bass, true);
  if(do_sort) {
    bass.SortFinished();
  }
  show_blobs(dest, bass.finishedBlobs, 
	     minarea, 0, showseg, showbars, showell, true);
}

// void image_blobify(const Image &src, Image &dest, int minarea, bool showseg,
//                    bool showbars, bool showell)
// {
//   CBlobAssembler bass;
//   SSegment seg;
//   // When not actively in a segment, set seg.row to norow.
//   const short norow = 0x1ff;

//   seg.model = 0;
//   seg.row = norow;

//   // Setup to record segments or not
//   CBlob::recordSegments = showseg;
//   SMoments::computeAxes= true;
//   CBlob::testMoments= true;

//   dest.resize(src.nrows, src.ncols);
//   for (int y= 0; y< src.nrows; y++) {
//     for (int x= 0; x< src.ncols; x++) {
//       TPix val = src.pixel(x, y);
//       // Copy same pixel to destination
//       dest.pixel(x, y) = val;

//       // Do blob processing
//       if(val!=0) {
//         // This pixel is part of a segment
//         if(seg.row == norow) {
//           // Just starting a new segment
//           seg.row = y;
//           seg.startCol = x;
//           seg.endCol = x;
//         }
//         else {
//           // Continuing an existing segment
//           seg.endCol = x;
//         }
//       }
//       else {
//         // This pixel is not part of a segment, check if one just ended
//         if(seg.row != norow) {
//           // Segment just ended, add to blob assembler
//           bass.Add(seg);
//           // Mark segment as invalid
//           seg.row = norow;
//         }
//       }
//     }
//     // Row just ended, close off any active segments
//     if(seg.row != norow) {
//       // Add to blob assembler
//       bass.Add(seg);
//       // Mark segment as invalid
//       seg.row = norow;
//     }
//   }

//   // Tell the blob assembler we're done and process the finished blobs
//   bass.EndFrame();
//   bass.SortFinished();
//   // Test that blobs are in fact sorted.  Don't do this in production code
//   bass.AssertFinishedSorted();
  
//   CBlob *blob = bass.finishedBlobs;
//   SMomentStats stats;
//   TPix white = Image::make_rgb(0xff, 0xff, 0xff);
//   TPix color;
//   int i=0;

//   cout << "---------------------------------------------------------" << endl
//        << "Blob stats:" << endl;

//   while(blob!=NULL) {
//     blob->moments.GetStats(stats);
//     if(stats.area >= minarea) {
//       if(i>=num_colors) {
//         color = white;
//       }
//       else {
//         color = blob_colors[i];
//       }
//       cout << "\tBlob " << i++ << ": area " << stats.area
//            << ", centroid (" << stats.centroidX
//            << ", " << stats.centroidY << "), " << endl
//            << "                axis " << stats.angle*180.0/M_PI
//            << ", aspect " << stats.minorDiameter / stats.majorDiameter << endl;

//       draw_blob(dest, blob, color, showseg, showell);

//       if(showbars && i<=num_colors) {
//         dest.draw_fillrect((i-1)*10, 0, i*10, 10, color);
//       }
//     }
//     blob = blob->next;
//   }
// }

bool filter_image(const Image &src, const string &operation,
                  Image &dest)
{
  vector<string> args;
  map<string,int> nmap;
  map<string,string> nsmap;

  split(operation, ",", args);
  if (args.size() < 1) { return false; }
  const string &cmd= args[0];
  get_named_args(args, nmap);
  get_named_args(args, nsmap);

  // Check for disable flag for this filter
  if(getnarg(nmap, "disable", 0)) {
    printf("Disabling %s filter\n", cmd.c_str());
    dest=src;
    return(true);
  }

  if (cmd == "invert")
    image_invert(src, dest);
  else if (cmd == "threshold_rgb_cube") {
    image_threshold_rgb_cube(src, dest,
                             getnarg(nmap, "rmin", 0),
                             getnarg(nmap, "rmax", 255),
                             getnarg(nmap, "gmin", 0),
                             getnarg(nmap, "gmax", 255),
                             getnarg(nmap, "bmin", 0),
                             getnarg(nmap, "bmax", 255));
  }
  else if (cmd == "threshold_hsv_cube")
    image_threshold_hsv_cube(src, dest,
                             getnarg(nmap, "hmin", 0),
                             getnarg(nmap, "hmax", 360),
                             getnarg(nmap, "smin", 0),
                             getnarg(nmap, "smax", 255),
                             getnarg(nmap, "vmin", 0),
                             getnarg(nmap, "vmax", 255));
  else if (cmd == "threshold_hsv_cube_rb_lookup")
    image_threshold_hsv_cube_rb_lookup(src, dest,
                                       getnarg(nmap, "hmin", 0),
                                       getnarg(nmap, "hmax", 360),
                                       getnarg(nmap, "smin", 0),
                                       getnarg(nmap, "smax", 255),
                                       getnarg(nmap, "vmin", 0),
                                       getnarg(nmap, "vmax", 255),
                                       getnarg(nmap, "rbits", 8),
                                       getnarg(nmap, "gbits", 8),
                                       getnarg(nmap, "bbits", 8));
#ifndef NO_XRC
  else if (cmd == "threshold_hsv_cube_xrc") {
    image_threshold_hsv_cube_xrc(src, dest,
                                 getnarg(nmap, "hmin", 0),
                                 getnarg(nmap, "hmax", 360),
                                 getnarg(nmap, "smin", 0),
                                 getnarg(nmap, "smax", 255),
                                 getnarg(nmap, "vmin", 0),
                                 getnarg(nmap, "vmax", 255));
  }
  else if (cmd == "threshold_hsv_cube_xrc3") {
    int i;
    Image thresh_img[3];
    CBlobAssembler bass[3];

    SMoments::computeAxes= true;

    for(i=0; i<3; i++) {
      string suffix = string_printf("_%d", i);
      
      int hmin = getnarg(nmap, "hmin" + suffix, 0);
      int hmax = getnarg(nmap, "hmax" + suffix, 360);
      int smin = getnarg(nmap, "smin" + suffix, 0);
      int smax = getnarg(nmap, "smax" + suffix, 255);
      int vmin = getnarg(nmap, "vmin" + suffix, 0);
      int vmax = getnarg(nmap, "vmax" + suffix, 255);

      image_threshold_hsv_cube_xrc(src, thresh_img[i], hmin, hmax, 
				   smin, smax, vmin, vmax);
      image_blobify(thresh_img[i], bass[i], true);
    }

    dest.resize(src.nrows, src.ncols);

    // Go over the three images and generate a composite output image
    // with r, g, b set based on value of thresh_img 0, 1, and 2 respectively
    for (int y= 0; y< src.nrows; y++) {
      for (int x= 0; x< src.ncols; x++) {
	unsigned char r,g,b;
	if(thresh_img[0].pixel(x, y) == 0) {
	  r=0;
	}
	else {
	  r=255;
	}
	if(thresh_img[1].pixel(x, y) == 0) {
	  g=0;
	}
	else {
	  g=255;
	}
	if(thresh_img[2].pixel(x, y) == 0) {
	  b=0;
	}
	else {
	  b=255;
	}

	dest.set_rgb(x, y, r, g, b);
      }
    }

    if(getnarg(nmap, "blobify", 0)) {
      for(i=0; i<3; i++) {
	show_blobs(dest, bass[i].finishedBlobs, 
		   getnarg(nmap, "minarea", 0),
		   (bool)getnarg(nmap, "showseg", 0),
		   (bool)getnarg(nmap, "showbars",1),
		   (bool)getnarg(nmap, "showell",1), 
		   true);
      }
    }

  }
#endif
  else if (cmd == "segment_hsv")
    image_segment_hsv(src, dest,
                      getnarg(nmap, "max_delta_h", 360),
                      getnarg(nmap, "max_delta_s", 255),
                      getnarg(nmap, "max_delta_v", 255),
                      getnarg(nmap, "min_s", 0),
                      getnarg(nmap, "min_abs_s", 0),
                      getnarg(nmap, "min_v", 0));
  else if (cmd == "truncate_rgb")
    image_truncate_rgb(src, dest,
                       getnarg(nmap, "rbits", 8),
                       getnarg(nmap, "gbits", 8),
                       getnarg(nmap, "bbits", 8));
  else if(cmd == "blobify") {
    show_image_blobify(src, dest,
		       getnarg(nmap, "minarea", 0),
		       (bool)getnarg(nmap, "showseg", 0),
		       (bool)getnarg(nmap, "showbars",1),
		       (bool)getnarg(nmap, "showell",1), 
		       (bool)getnarg(nmap, "sort",1));

  }
  else if (cmd == "colorref") {
    image_create_colorref(src, dest,
                          (bool)getnarg(nmap, "xrc", 0),
                          (bool)getnarg(nmap, "hist", 0));
  }
  else if(cmd == "ellipse") {
    dest.resize(src.nrows, src.ncols);
    dest.fill(0);
    draw_ellipse(dest, (float)src.ncols/2.0, (float)src.nrows/2.0,
                 (float)(getnarg(nmap, "angle", 0))*(M_PI/180.0),
                 getnarg(nmap, "major_axis", src.ncols/4),
                 getnarg(nmap, "minor_axis", src.nrows/4),
                 0xffffff);
  }
  else {
    fprintf(stderr, "Bad operation >%s<\n", operation.c_str());
    return false;
  }

  // Check if we should write the dest image out
  string fname=getnarg(nsmap, "fname", "");
  if(fname!="") {
    cout << "Writing result of " << cmd << " filter to file "
         << fname << endl;
    FILE *out= fopen(fname.c_str(), "w");
    dest.write_ppm(out);
    fclose(out);
  }
  return true;
}

class Imageserver_Web_Server: public TIPC_Web_Server {
  void serve_root_url(FILE *out) {
    output_html_message(out, "<b>Image server</b>\n");
  }
  void serve_bad_addr(FILE *out, const string &url) {
    output_html_header(out);
    fprintf(out, "Bad address: %s\n", url.c_str());
  }
  void serve_html(FILE *out, const vector<string> &args) {
    const string &filename= args[0];
    output_html_header(out);
    FILE *in= fopen(filename.c_str(), "r");
    if (!in) {
      fprintf(out, "Couldn't open html %s\n", filename.c_str());
      return;
    }
    // Dump args, if any
    if (args.size() > 1) {
      string sargs="args=[", snames="argnames=[";
      fprintf(out, "<script>\n");
      for (unsigned int i= 1; i< args.size(); i++) {
        vector<string> filter_args;
        vector<string> argnames;
        split(args[i], ",", filter_args);
        strip_named_args(filter_args, argnames);
        if (i > 1) {
          sargs+= ",   \n";
          snames+= ",   \n";
        }
        sargs += "[";
        snames += "[";
        for (unsigned int j= 0; j< filter_args.size(); j++) {
          if (j > 0) {
            sargs +=",";
            snames +=",";
          }
          sargs += "'" + filter_args[j] + "'";
          snames += "'" + argnames[j] + "'";
        }
        sargs += "]";
        snames += "]";
      }
      sargs += "];\n";
      snames += "];\n";

      fprintf(out, "%s%s", sargs.c_str(), snames.c_str());
      fprintf(out, "</script>\n");
    }
    char buf[1024];
    while (!feof(in)) {
      int nread= fread(buf, 1, sizeof(buf), in);
      if (nread > 0) fwrite(buf, 1, nread, out);
    }
  }

  void serve_picture(FILE *out, const vector<string> &args) {
    const string &picturename= args[0];
    Image image;
    if (!image.read(picturename)) {
      output_html_header(out);
      fprintf(out, "Couldn't read image %s\n", picturename.c_str());
      return;
    }
    for (unsigned int i= 1; i< args.size(); i++) {
      Image dest;
      if (!filter_image(image, args[i], dest)) {
        fprintf(out, "Couldn't filter image with %s\n", args[i].c_str());
      }
      image= dest;
    }
    output_image(out, image);
  }
  void output_image(FILE *out, Image &image) {
    bool use_png= false;
    fprintf(out, "HTTP/1.0 200 OK\n");
    if (use_png) {
      fprintf(out, "Content-Type: image/png\n\n");
    } else {
      fprintf(out, "Content-Type: image/jpeg\n\n");
    }
    fflush(out);
    // Temporarily redirect stdout to out
    int save_stdout= dup(fileno(stdout));
    dup2(fileno(out), fileno(stdout));
    string cmd;
    if (use_png) cmd= "pnmtopng";
    else {
      if (filename_exists("/sw/bin/cjpeg")) cmd= "/sw/bin/cjpeg";
      else cmd= "cjpeg";
      cmd += " -q 95";
    }
    FILE *cjpeg=popen(cmd.c_str(),"w");
    image.write_ppm(cjpeg);
    pclose(cjpeg);
    // Stop redirecting
    sleep_us(1000);
    dup2(save_stdout, fileno(stdout));
    close(save_stdout);
  }
  virtual void serve_url(int fd, const string &url) {
    char cwd[2048];
    getcwd(cwd, 2048);
    string new_url = string(cwd) + url;
    FILE *out= fdopen(fd, "w");
    vector<string> args;
    split(new_url, ";", args);
    assert(new_url.size()>0);
    string &addr= args[0];
    if (addr == "/") {
      serve_root_url(out);
    }
    else if (filename_suffix(addr) == "ppm") {
      serve_picture(out, args);
    }
    else if (filename_suffix(addr) == "html") {
      serve_html(out, args);
    }
    else {
      serve_bad_addr(out, new_url);
    }
    fclose(out);
  }
};

int iabs(int a) { return a > 0 ? a : -a; }

void test_hsv_to_rgb(unsigned short h, unsigned char s, unsigned char v)
{
  unsigned short htest;
  unsigned char stest, vtest;
  unsigned char r,g,b;
  hsv_to_rgb(h,s,v, r,g,b);
  printf("STD h:%d s:%d v:%d -> r:%d g:%d b:%d\n",
         h,s,v,r,g,b);

  rgb_to_hsv(r,g,b, htest, stest, vtest);
  printf("->  h:%d s:%d v:%d", htest, stest, vtest);
  if (iabs(h-htest)>1 || iabs(s-stest)>1 || iabs(v-vtest)>1) printf("***");
  printf("\n");

#ifndef NO_XRC
  s = s * 224 / 255;
  v = v * 224 / 255;
  HsvToRgbOV6620(h,s,v,r,g,b);
  printf("OVT h:%d s:%d v:%d -> r:%d g:%d b:%d\n",
         h,s,v,r,g,b);
  RgbToHsvOV6620(r,g,b, htest, stest, vtest);
  printf("->  h:%d s:%d v:%d", htest, stest, vtest);
  if (iabs(h-htest)>1 || iabs(s-stest)>1 || iabs(v-vtest)>1) printf("***");
  printf("\n");
#endif
}

void usage()
{
  printf("Usage:\n");
  printf("imageserver [-port portnum]\n");
  exit(1);
}

int main(int argc, char **argv)
{
  // When running under gdb, SIGPIPE by default will not be ignored
  // To ignore under gdb, type
  // handle SIGPIPE nostop
  signal(SIGPIPE, SIG_IGN);

  char **argptr= argv+1;
  int port= 9001;

  while (*argptr) {
    char *arg= *argptr++;
    if (!strcmp(arg, "-port") && *argptr) {
      arg= *argptr++;
      port= atoi(arg);
    } else {
      printf("Don't know flag '%s'\n", arg);
      usage();
    }
  }

#if 0
  for (unsigned short hue= 0; hue < 360; hue+=30) {
    test_hsv_to_rgb(hue,255,0);
    test_hsv_to_rgb(hue,255,128);
    test_hsv_to_rgb(hue,255,255);
    test_hsv_to_rgb(hue,128,255);
    test_hsv_to_rgb(hue,0,255);
    printf("\n");
  }
#endif

  Imageserver_Web_Server webserver;
  string result;
  printf("To connect to this server, please browse to:\n");
  printf("http://localhost:%d/index.html\n\n", port);
  webserver.serve(port, result);
}

