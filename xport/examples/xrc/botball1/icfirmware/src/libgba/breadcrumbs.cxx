#include <stdlib.h>
#include "breadcrumbs.h"
#include "textdisp.h"

// Put the breadcrumbs at the start of internal work ram.  The stack
// is in this RAM, but build from the high addresses (CRUMB_START +
// 32K) towards the beginning of the block (CRUMB_START)
#define CRUMB_START 0x3004000

//#define CRUMB_START 0x2030000
#define USE_MALLOC

unsigned short *crumbs;
int crumb_length;
int crumb_idx;

void init_crumbs(int length, bool dump) {
  if (crumbs) return;
  crumb_length= length*2;
  int nbytes= crumb_length*sizeof(unsigned short);
#ifdef USE_MALLOC
  crumbs= (unsigned short *)malloc(nbytes);
#else
  crumbs= (unsigned short *)CRUMB_START;
#endif
  printf("Using crumb buffer %x\n   length %d\n",
	 (int)crumbs, nbytes);
  if (dump) dump_crumbs();
  for (int i= 0; i< crumb_length; i++) crumbs[i]=(unsigned short)-1;
}

void dump_crumbs() {
  if (!crumbs) {
    printf("crumbs undefined\n");
    return;
  }
  int start= -1;
  for (int i= 0; i< crumb_length; i+=2) {
    int next= (i+2)%crumb_length;
    if (crumbs[i]== (unsigned short)-1 && 
	crumbs[next] != (unsigned short)-1) {
      start= next;
      break;
    }
  }
  if (start == -1) {
    printf("Couldn't find crumb start:\n");
    for (int i= 0; i<4; i++) {
      printf("  %x: %04X: %d [0x%x]\n", ((int)&(crumbs[i])), 
	     crumbs[i], crumbs[i+1], crumbs[i+1]);
    }
    return;
  }
  printf("crumbs start:\n");
  for (int i= start; crumbs[i] != (unsigned short)-1; i=(i+2)%crumb_length) {
    printf("%04X: %d [0x%x]\n", crumbs[i], crumbs[i+1], crumbs[i+1]);
  }
  printf("crumbs end\n");
}
