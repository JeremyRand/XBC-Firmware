#include "image.h"
#include "file_utils.h"

int main(int argc, char **argv)
{
  string infile="";
  string outfile="";
  string varname="";

  for (int i=1; i < argc; ++i) {
    if (!strcmp (argv[i], "-o")) {
      if (++i < argc) outfile = argv[i];
    }
    else if (!strcmp (argv[i], "-n")) {
      if (++i < argc) varname = argv[i];
    }
    else {
      infile = argv[i];
    }
  }

  Image img; 

  if(!img.read_ppm(infile)) {
    fprintf(stderr, "Could not read input file %s\n", infile.c_str());
  }
  
  if(outfile == "") {
    outfile = filename_sans_directory(filename_sans_suffix(infile)) + ".cxx";
  }

  if(varname == "") {
    varname = filename_sans_directory(filename_sans_suffix(infile));
  }

  printf("Writing input image %s to %s\n", infile.c_str(), outfile.c_str());
  printf("  naming variables %s\n", varname.c_str());
	 
  // Generate header name
  string hdrfile = filename_sans_suffix(outfile) + ".h";

  // Generate argstr for comments
  string argstr=argv[0];

  for (int i=1; i < argc; ++i) {
    argstr += " ";
    argstr += argv[i];
  }

  // Generate header file
  FILE *hdr= fopen(hdrfile.c_str(), "w");

  fprintf(hdr, "#ifndef __%s_HDR__\n", varname.c_str());
  fprintf(hdr, "#define __%s_HDR__\n\n", varname.c_str());
  
  fprintf(hdr, "// GBA image generated by:\n");
  fprintf(hdr, "//   %s\n\n", argstr.c_str());

  fprintf(hdr, "extern const int %s_nrows;\n", varname.c_str());
  fprintf(hdr, "extern const int %s_ncols;\n", varname.c_str());
  fprintf(hdr, "extern const unsigned short %s[%d][%d];\n\n", 
	  varname.c_str(), img.nrows, img.ncols);

  fprintf(hdr, "#endif\n");

  fclose(hdr);

  // Generate source file
  FILE *out= fopen(outfile.c_str(), "w");
  
  fprintf(out, "// GBA image generated by:\n");
  fprintf(out, "//   %s\n\n", argstr.c_str());

  fprintf(out, "#include \"%s\"\n\n", hdrfile.c_str());

  fprintf(out, "const int %s_nrows = %d;\n", varname.c_str(), img.nrows);
  fprintf(out, "const int %s_ncols = %d;\n", varname.c_str(), img.ncols);
  fprintf(out, "const unsigned short %s[%d][%d] = {\n", varname.c_str(), 
	  img.nrows, img.ncols);

  for (int row= 0; row< img.nrows; row++) {
    fprintf(out, "  {\t// Row %d\n    ", row);
    for (int col= 0; col< img.ncols; col++) {
      unsigned char r, g, b;
      img.get_rgb(col, row, r, g, b);
      // Convert the 8-bit RGB values to 5 bit values and shift them 
      // to be compatible with the 16-bit GBA RGB pixel format
      int gbaPix = 
	(((b>>3)&0x1f)<<10)+
	(((g>>3)&0x1f)<<5)+
	((r>>3)&0x1f);
      //printf("r=%3d, g=%3d, b=%3d: gbaPix=0x%4x\n", r, g, b, gbaPix);
      if(col % 8 == 7) {
	fprintf(out, "\n    ");
      }
      fprintf(out, "0x%x", gbaPix);
      if(col == img.ncols-1) {
	fprintf(out, "\n");
      }
      else {
	fprintf(out, ", ");
      }
    }
    if(row != img.nrows-1) {
      fprintf(out, "  },\n\n");
    }
  }
  fprintf(out, "  }\n};\n");
  fclose(out);
}