#include <unistd.h>
#include <stdio.h>
#include <string>
#include "sleep_us.h"
#include "blob.h"
#include "image.h"
#include <vector>

#define DV_MODELS					3
#define CHECK 1

using namespace std;
static bool do_print=true;

#define PRINTF(x...) if(do_print) printf(x)

void usage()
{
  printf("Usage:\n");
  printf("bloblog <logfile>\n");
  exit(1);
}

// Call this with a completed blob assembler
int print_blobs(CBlob *blob)
{
  SMomentStats stats;
  int i=0;

  while(blob!=NULL) {
#ifdef CHECK
    PRINTF("blob %x\n", (unsigned int)blob);
#endif
    blob->moments.GetStats(stats);
    PRINTF("\tBlob %d: area %d, centroid (%f, %f)\n", 
	   i, stats.area, stats.centroidX, stats.centroidY);
    i++;

#ifdef CHECK
    if (blob->next==blob)
      {
	printf("cycle found (show_blobs)\n");
	return(-1);
      }
#endif
    blob = blob->next;
  }
  return(i);
}


int main(int argc, char **argv)
{
  char **argptr= argv+1;
  string fname = "";
  string outdir = "";
  bool do_debug = true;
  vector<unsigned long> segvec;

  while (*argptr) {
    char *arg= *argptr++;
    if (!strcmp(arg, "-debug")) {
      do_debug=true;
    } else if (!strcmp(arg, "-help")) {
      usage();
    } else if (!strcmp(arg, "-q")) {
      do_print=false;
    } else if (!strcmp(arg, "-o") && *argptr) {
      arg= *argptr++;
      outdir = arg;
    } else {
      fname = arg;
    }
  }

  FILE *in= fopen(fname.c_str(), "r");
  if (!in) {
    fprintf(stderr, "Failed to open file %s\n", fname.c_str());
    return(-1);
  }

  CBlobAssembler m_blobAssembler[DV_MODELS];
  string line="";
  bool in_frame = false;
  int i=0;
  utime_t starttime, endtime;

  while (!feof(in)) {
    line = getline(in);
    if(line.find_first_of(">") != string::npos) {
      printf("-------------------------\nGot frame start\n");
      for (i=0; i<DV_MODELS; i++)
	m_blobAssembler[i].Reset();
      in_frame = true;
      segvec.clear();
    }
    else if(line.find_first_of("<") == 0) {
      int blobcount=0;
      printf("Got frame end:\n");

      starttime = ustime();
      for(size_t j=0; j<segvec.size(); j++) {
	unsigned long compSegment = segvec[j];
	SSegment segment;
	unsigned char model;
	
	model = compSegment&0x07;
	compSegment >>= 3;
	segment.row = compSegment&0x1ff;
	compSegment >>= 9;
	segment.startCol = compSegment&0x3ff;
	compSegment >>= 10;
	segment.endCol = compSegment&0x3ff;
	
	if (model>=DV_MODELS || segment.startCol>355 || segment.endCol>355 || segment.row>290)
	  {
	    printf("error (BuildBlobs): Model %d row %d, %d -> %d\n", 
		   model, segment.row, segment.startCol, segment.endCol);
	    continue;
	  }
	
	PRINTF("Processing 0x%s\tModel %d row %d, %d -> %d\n", 
	       line.c_str(), 
	       model, segment.row, segment.startCol, segment.endCol);
	
	m_blobAssembler[model].Add(segment);
      }
	
      for (i=0; i<DV_MODELS; i++) {
	m_blobAssembler[i].EndFrame();
	PRINTF("Model %d:\n", i);
	blobcount += print_blobs(m_blobAssembler[i].finishedBlobs);
      }
      endtime = ustime();
      int elapsed = endtime-starttime;
      printf("Time = %d usec, %d segments, %d blobs\n", 
	     elapsed, segvec.size(), blobcount);
      printf("-------------------------\n");
      in_frame=false;
    }      
    else if(in_frame) {
      unsigned long compSegment = strtol(line.c_str(),NULL,16);
      segvec.push_back(compSegment);
    } else {
      PRINTF("Skipping '%s'\n", line.c_str());
    }
  }
  fclose(in);
}
