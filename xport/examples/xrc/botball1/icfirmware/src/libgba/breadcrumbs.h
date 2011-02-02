#ifndef _BREADCRUMBS_H
#define _BREADCRUMBS_H

extern unsigned short *crumbs;
extern int crumb_idx;
extern int crumb_length;

void init_crumbs(int length, bool dump);
void dump_crumbs();

#define ADD_DEBUG_CRUMB(token, val) \
  if (crumbs) { \
    int next_idx= crumb_idx+2; \
    if (next_idx == crumb_length) next_idx= 0; \
    crumbs[next_idx]= (unsigned short)-1; \
    crumbs[crumb_idx++]= token; \
    crumbs[crumb_idx]= val; \
    crumb_idx= next_idx; \
  } else {}
  
#define ADD_CRUMB(filetoken)  ADD_DEBUG_CRUMB(filetoken, __LINE__)

#endif
