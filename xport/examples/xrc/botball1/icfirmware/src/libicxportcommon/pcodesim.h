/**
 ** pcodesim.h
 **
 ** Copyright 1990, 1991 by Randy Sargent.
 **
 ** The author hereby grants to MIT permission to use this software.
 ** The author also grants to MIT permission to distribute this software
 ** to schools for non-commercial educational use only.
 **
 ** The author hereby grants to other individuals or organizations
 ** permission to use this software for non-commercial
 ** educational use only.  This software may not be distributed to others
 ** except by MIT, under the conditions above.
 **
 ** Other than these cases, no part of this software may be used or
 ** distributed without written permission of the author.
 **
 ** Neither the author nor MIT make any representations about the 
 ** suitability of this software for any purpose.  It is provided 
 ** "as is" without express or implied warranty.
 **
 ** Randy Sargent
 ** Research Specialist
 ** MIT Media Lab
 ** 20 Ames St.  E15-301
 ** Cambridge, MA  02139
 ** E-mail:  rsargent@athena.mit.edu
 **
 **/


#ifndef PCODESIM_H
#define PCODESIM_H

#include "ICRobot.h"
#ifndef USE_OWN_CLASS

#include "config.h"

extern int mseconds;

typedef unsigned char uchar;

typedef short          int16;
typedef unsigned short intp;
typedef long           int32;
typedef float          float32;

typedef unsigned short uint16;
typedef unsigned long  uint32;

extern intp proc_current;
extern int16 proc_counter;
extern int proc_error_flag;


extern int pcodesim_debug;
extern int pcodesim_major_version;
extern int pcodesim_minor_version;


extern char *pc, *sp;
extern int err;
extern char *mem;
extern int simulator_is_running;

#ifndef _WINDOWS_
#define CANT_COERCE_LVALUE
#endif


#define MAX_REGS 1

#define STACK_OFFSET2(x) READ2(sp+(x))
#define STACK_OFFSET4(x) READ4(sp+(x))
#define SET_STACK_OFFSET2(x,y) WRITE2(sp+(x), (y))
#define SET_STACK_OFFSET4(x,y) WRITE4(sp+(x), (y))
#define GROW_STACK(x)  (sp -= (x))

#define MEM1(x) READ1(x+mem)
#define MEM2(x) READ2(x+mem)
#define MEM4(x) READ4(x+mem)
#define MEMP(x) READP(x+mem)

#define SET_MEM1(x,y) WRITE1((x)+mem,(y))
#define SET_MEM2(x,y) WRITE2((x)+mem,(y))
#define SET_MEM4(x,y) WRITE4((x)+mem,(y))
#define SET_MEMP(x,y) WRITEP((x)+mem,(y))

#ifndef CANT_COERCE_LVALUE
#define FETCH1 ( *(  ((char *)pc)++) )
#define READ1(x) (*(char *)(x))
#define WRITE1(x,y) ((*(char *)(x))=(y))
#else
#define FETCH1 fetch1()
#define READ1(x) read1(x)
#define WRITE1(x,y) write1(x,y)
#endif

/* Pointer operations */
#define FETCHP      ((intp)FETCH2)
#define READP(x)    ((intp)READ2(x))
#define WRITEP(x,y) (WRITE2(x,(int16)(y)))
#define PUSHP(x)    PUSH2((int16)(x))
#define POPP        (intp)POP2

#if defined(ODD_ADDRESSES_OK) && !defined(CANT_COERCE_LVALUE)

#define FETCH2 ( *(  ((int16*)pc)++) )
#define FETCH4 ( *(  ((int32*)pc)++) )

#define PUSH2(x)  ( *(--((int16  *)sp) = (x)) )
#define PUSH4(x)  ( *(--((int32  *)sp) = (x)) )
#define PUSHF(x)  ( *(--((float32*)sp) = (x)) )

#define POP2   ( *(  ((int16  *)sp)++) )
#define POP4   ( *(  ((int32  *)sp)++) )
#define POPF   ( *(  ((float32*)sp)++) )

#define READ2(x) (*(int16*)(x))
#define READ4(x) (*(int32*)(x))
#define READF(x) (*(float32*)(x))

#define WRITE2(x,y) ((*(int16*)(x))=(y))
#define WRITE4(x,y) ((*(int32*)(x))=(y))

#else

#define FETCH2 fetch2()
#define FETCH4 fetch4()

#define PUSH2(x) push2(x)
#define PUSH4(x) push4(x)
#define PUSHF(x) pushf(x)

#define POP2 pop2()
#define POP4 pop4()
#define POPF popf()

#define READ2(x) read2(x)
#define READ4(x) read4(x)
#define READF(x) readf(x)

#define WRITE2(x,y) write2(x,y)
#define WRITE4(x,y) write4(x,y)

#endif

extern int invalid;

class ICRobot;

void execute (void);
int proc_start (void);
int proc_kill (int16 pid_to_kill);
void proc_check (intp proc_ptr);
void proc_remove (intp proc_ptr);
void sPrintToBuffer_binary_byte (char* cstrBuffer, int16 b);
void pPrintToBuffer (char *format);
void pcodesim_init (void);
INLINE uchar read1 (char *p);
INLINE int16 read2 (char *p);
INLINE int32 read4 (char *p);
INLINE float readf (char *p);
INLINE void write1 (char *p, uchar x);
INLINE void write2 (char *p, int16 x);
INLINE void write4 (char *p, int32 x);
INLINE uchar fetch1 (void);
INLINE int16 fetch2 (void);
INLINE int32 fetch4 (void);
INLINE void push1 (uchar x);
INLINE void push2 (int16 x);
INLINE void push4 (int32 x);
INLINE void pushf (float x);
INLINE uchar pop1 (void);
INLINE int16 pop2 (void);
INLINE int32 pop4 (void);
INLINE float popf (void);
void pcodesim_wakeup (void);
int pcodesim_execute_some (void);
void pcodesim_reset (void);
void proc_error (int16 err);
void setMLTranslator(ICRobot* robotTranslator);
#endif /* #ifndef USE_OWN_CLASS */
#endif /* #ifndef PCODESIM_H */

