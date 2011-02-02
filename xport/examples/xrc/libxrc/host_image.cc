#include <algorithm>
#include "image.h"

#include <assert.h>
#include <string>

void chomp(string &str)
{
  string::size_type crpos= str.find_first_of("\r\n");
  if (crpos != string::npos) str= str.substr(0, crpos);
}

void 
Image::get_hsv(int x, int y,
               unsigned short &h, unsigned char &s, unsigned char &v)  const 
{
  unsigned char r,g,b;
  get_rgb(x,y,r,g,b);
  rgb_to_hsv(r,g,b,h,s,v);
}

bool 
Image::read(const string &imagename) 
{
  if (filename_suffix(imagename) == "ppm") {
    return read_ppm(imagename);
  } else {
    fprintf(stderr, "Don't know how to read %s\n", imagename.c_str());
    return false;
  }
}

bool 
Image::read_ppm(const string &imagename) 
{
  FILE *in= fopen(imagename.c_str(), "r");
  if (!in) {
    fprintf(stderr, "Can't read image %s\n", imagename.c_str());
    return false;
  }
  string header= getline(in);
  chomp(header);
  if (header != "P6") {
    fprintf(stderr, "Wrong header '%s'\n", header.c_str());
    fclose(in);
    return false;
  }
  int new_nrows, new_ncols;
  string dims= getline(in);
  if (2 != sscanf(dims.c_str(), "%d %d", &new_ncols, &new_nrows)) {
    fprintf(stderr, "Can't read dims '%s'\n", dims.c_str());
    fclose(in);
    return false;
  }
  string maxval= getline(in);
  resize(new_nrows, new_ncols);
  unsigned char *buf= (unsigned char *) malloc(nrows * ncols * 3);
  int nread= fread(buf, 1, nrows*ncols*3, in);
  fclose(in);
  if (nread < nrows*ncols*3) {
    fprintf(stderr, "Could only read %d of %d bytes\n",
	    nread, nrows*ncols*3);
    return false;
  }
  for (int y= 0; y< nrows; y++) {
    for (int x= 0; x< ncols; x++) {
      set_rgb(x, y,
	      buf[3*(x+y*ncols)+0],
	      buf[3*(x+y*ncols)+1],
	      buf[3*(x+y*ncols)+2]);
    }
  }
  free((void*)buf);
  return true;
}
bool 
Image::write_ppm(FILE *out) 
{
  fprintf(out, "P6\n%d %d\n255\n", ncols, nrows);
  unsigned char *buf= (unsigned char *) malloc(nrows*ncols*3);

  for (int y= 0; y< nrows; y++) {
    for (int x= 0; x< ncols; x++) {
      get_rgb(x, y,
	      buf[3*(x+y*ncols)+0],
	      buf[3*(x+y*ncols)+1],
	      buf[3*(x+y*ncols)+2]);
    }
  }
  fwrite(buf, nrows*ncols*3, 1, out);
  return true;
}
  
