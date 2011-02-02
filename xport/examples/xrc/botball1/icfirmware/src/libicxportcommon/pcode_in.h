/**
 ** pcode_in.h
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

#ifndef PCODE_IN_H
#define PCODE_IN_H

/*  lets C and assembler share the opcode definitions  */
#ifdef PCODE_INTERPRETER

#define FIRST_OPCODE(x)/**/ 	FDB	x
#define OPCODE(x)/**/ 	FDB	x
#define LAST_OPCODE(x)/**/ 	FDB	x
 
#endif

#ifdef COMPILER  


typedef int Opcode;

#define old_pcode 0

#define Padd2         0 
#define Psub2         1 
#define Pmult2        2 
#define Pdiv2         3 
#define Pbitand2      4 
#define Pbitor2       5 
#define Pbitxor2      6 
#define Pequal2       7 
#define Plt2          8 
#define Pgt2          9 
#define Plshift       10 
#define Plognot2      11 
#define Plogidn2      12 
#define Pbitnot2      13 
#define Pneg2         14 
#define Padd4         15 
#define Psub4         16 
#define Pmult4        17 
#define Plt4          18 
#define Pgt4          19 
#define Pequal4       20 
#define Pneg4         21 
#define Ppush2        22 
#define Ppush4        23 
#define Ppushblock    24 
#define Ppop2         25 
#define Ppop4         26 
#define Ppeeki1       27 
#define Ppeeki2       28 
#define Ppeeki4       29 
#define Ppeek1        30 
#define Ppeek2        31 
#define Ppeek4        32 
#define Pspeek2       33 
#define Pspeek4       34 
#define Ppokei1       35 
#define Ppokei2       36 
#define Ppokei4       37 
#define Ppoke1        38 
#define Ppoke2        39 
#define Ppoke4        40 
#define Ppoke1nopop   41 
#define Ppoke2nopop   42 
#define Ppoke4nopop   43 
#define Pbitset       44 
#define Pbitclr       45 
#define Pspoke2       46 
#define Pspoke4       47 
#define Pjumpi        48 
#define Pjump         49 
#define Pjfalse       50 
#define Pjtrue        51 
#define Pjpfalse      52 
#define Pjptrue       53 
#define Pmret0        54 
#define Pmret2        55 
#define Pmret4        56 
#define Paref1        57 
#define Paref2        58 
#define Paref4        59 
#define Paref_arb     60 
#define Paddsp        61
#define Psprel        62
#define Pcheckstack   63
#define Psetsp        64
#define Pfl2int       65
#define Pint2fl       66
#define Pfl2lng       67
#define Plng2fl       68
#define Pfadd         69
#define Pfsub         70
#define Pfmult        71
#define Pfdiv         72
#define Pfequal       73
#define Pflt          74
#define Pfgt          75
#define Pfneg         76
#define Pfsqrt        77
#define Pfexp         78
#define Pf10tx        79
#define Pfx2y         80
#define Pfln          81
#define Pflog         82
#define Pfatan        83
#define Pfsin         84
#define Pfcos         85
#define Pftan         86
#define Pfl2ascii     87
#define Pprintlcd2    88
#define Pprintf       89
#define Pprintstring  90
#define Pprintchar    91
#define Pstartprocess 92
#define Phaltnotify   93
#define Pkillprocess  94
#define Pdefer        95
#define Psystime      96
#define Ploadreg      97
#define Pfetchreg     98
#define Pbench        99
#define Pcallml       100
#define Pinitint      101
#define Pcallml2      102
#define Pcallml3      103
#define Pdiv4	      104
#define Pbitand4	  105
#define Pbitor4		  106
#define Pbitxor4	  107
#define Plognot4	  108
#define Plogidn4	  109
#define Pbitnot4	  110

#define Pundefined    111


#else

#ifdef SIMULATOR

#define FIRST_OPCODE(x) typedef enum { x ,  
#define OPCODE(x) x ,
#define LAST_OPCODE(x) x } Opcode;

#endif

#ifdef LIST_INSTRUCTIONS

#define FIRST_OPCODE(x) LIST_OPCODE x
#define OPCODE(x) LIST_OPCODE x
#define LAST_OPCODE(x) LIST_OPCODE x

#endif
  
/*************************/
/*** 16-BIT OPERATIONS ***/
/*************************/

/*********************/
/* binary 16-bit ops */
/*********************/

FIRST_OPCODE(Padd2)
OPCODE(Psub2)
OPCODE(Pmult2)
OPCODE(Pdiv2)
OPCODE(Pbitand2)
OPCODE(Pbitor2)
OPCODE(Pbitxor2)
OPCODE(Pequal2)
OPCODE(Plt2)
OPCODE(Pgt2)

OPCODE(Plshift)         /* shifts next-to-top-of-stack left by top-of-stack
			   bits.  shifts to the right if top-of-stack is
			   negative */

/*  unary 16-bit ops  */
OPCODE(Plognot2)	/* logical not:  0 -> 1
			   		else -> 0
			*/
OPCODE(Plogidn2)	/* logical identity:  0 -> 0
			   		else -> 1
			*/
OPCODE(Pbitnot2)	/* bitwise not */
OPCODE(Pneg2)     	/* two's complement */


/*************************/
/*** 32-BIT OPERATIONS ***/
/*************************/

OPCODE(Padd4)
OPCODE(Psub4)
OPCODE(Pmult4)
OPCODE(Plt4)
OPCODE(Pgt4)
OPCODE(Pequal4)

OPCODE(Pneg4)     	/* two's complement */

/****************************/
/*** MEMORY AND STACK OPS ***/
/****************************/

/**********************************************************/
/* the following instructions take an inline arg and push */
/* it onto the stack                                      */
/**********************************************************/
OPCODE(Ppush2)
OPCODE(Ppush4)

/*************************************/
/* takes inline byte of count;       */
/* inline data of bytes to be pushed */
/*************************************/
OPCODE(Ppushblock)

/****************************************************************/
/* the following instructions args pop and discard 2 or 4 bytes */
/****************************************************************/
OPCODE(Ppop2)
OPCODE(Ppop4)

/******************************************************************/
/* these take inline 16-bit address arg and peek 1, 2, or 4 bytes */
/******************************************************************/
OPCODE(Ppeeki1)     /*  puts byte on stack as word  */
OPCODE(Ppeeki2)
OPCODE(Ppeeki4)

/*************************/
/* pops                  */
/* absolute address      */
/* pushes                */
/* 1, 2, or 4 byte value */
/*************************/
OPCODE(Ppeek1)     /*  puts byte on stack as word  */
OPCODE(Ppeek2)
OPCODE(Ppeek4)

/***********************************************/
/* takes 8-bit arg as offset from top of stack */
/* add documentation for how this really works */
/***********************************************/
OPCODE(Pspeek2)
OPCODE(Pspeek4)

/******************************************************/
/* pops 2 or 4 byte arg off of stack, and pokes it to */
/* address specified by 16-bit inline arg             */
/******************************************************/
OPCODE(Ppokei1)     /*  coerces word to byte when poking  */
OPCODE(Ppokei2)
OPCODE(Ppokei4)

/******************************/
/* pops                       */
/* a) 2 or 4 byte arg         */
/* b) absolute address        */
/* and pokes it where you say */
/******************************/
OPCODE(Ppoke1)     /*  coerces word to byte when poking  */
OPCODE(Ppoke2)
OPCODE(Ppoke4)

/******************************/
/* pops                       */
/* a) 2 or 4 byte arg         */
/* b) absolute address        */
/* and pokes it where you say */
/* pushes (a) back on stack   */
/******************************/
OPCODE(Ppoke1nopop)     /*  coerces word to byte when poking  */
OPCODE(Ppoke2nopop)
OPCODE(Ppoke4nopop)

/*****************************************/
/* pops                                  */
/* a) 2 byte arg                         */
/* b) absolute address                   */
/* and sets or clears bits where you say */
/*****************************************/
OPCODE(Pbitset)
OPCODE(Pbitclr)

/*********************************************************/
/* pops 2 or 4 byte arg off of stack, pokes it back into */
/* stack based on 8-bit offset (inline arg)              */
/*********************************************************/
OPCODE(Pspoke2)
OPCODE(Pspoke4)


/***********************/
/*** FLOW OF CONTROL ***/
/***********************/

/*******************************/
/* takes 16-bit inline address */
/*******************************/
OPCODE(Pjumpi)

/**********************************/
/* takes 16-bit address off stack */
/**********************************/
OPCODE(Pjump)

/***********************************************/
/* takes 8-bit inline address offset (signed?) */
/***********************************************/
/*  #define Pjumprel  */

/*************************************************/
/* takes 16-bit number off stack as conditional, */
/* jumps to 16-bit inline address                */
/* true := anything but 0                        */
/* false := zero                                 */
/*************************************************/
OPCODE(Pjfalse)
OPCODE(Pjtrue)

/**************************************************/
/* takes 16-bit number off stack as conditional,  */
/* jumps to 16-bit inline address                 */
/* true := anything but 0                         */
/* false := zero                                  */
/* if the jump is taken, Pjpfalse leaves a 16 bit */
/* 0 on the stack, and Pjptrue leaves a 16 bit 1  */
/* on the stack                                   */
/**************************************************/
OPCODE(Pjpfalse)
OPCODE(Pjptrue)

/********************/
/*** MAGIC RETURN ***/
/********************/

/**************************************/
/* inline and random amount of stack  */
/* no further documentation available */
/**************************************/

/*********************************************************************/
/* DUE TO THE FREEDOM OF INFORMATION ACT,                            */
/* THE NSA HAS DISCLOSED THE FOLLOWING DOCUMENTATION                 */
/* FOR THE MRETx OPCODE:                                             */
/* a) reads (not pops) and saves the return value (0, 2, or 4 bytes) */
/* b) pops [inline word] bytes off the stack                         */
/* c) pops return address and places in pc                           */
/* d) pushes saved return value onto stack                           */
/* If return address was 0, execution is terminated                  */
/*********************************************************************/

OPCODE(Pmret0)
OPCODE(Pmret2)
OPCODE(Pmret4)

/*******************/
/*** ARRAY STUFF ***/
/*******************/

/********************************************/
/* pops                                     */
/* index (2-byte positive offset)           */
/* array base (2-byte absolute address) and */
/* off of stack, and                        */
/* pushes                                   */
/* absolute address of element              */
/* back on stack (or gives bounds error)    */
/********************************************/

OPCODE(Paref1)
OPCODE(Paref2)
OPCODE(Paref4)
OPCODE(Paref_arb)



/******************************/
/*** STACK POINTER FROBBING ***/
/******************************/

/***********************************************************/
/* the following modifies the SP by a signed 16-bit amount */
/* (inline arg)                                            */
/***********************************************************/
OPCODE(Paddsp)

/**********************************************************/
/* pushes value of current SP plus a 2-byte inline number */
/* used to push base+offset for a local array reference   */
/**********************************************************/
OPCODE(Psprel)

/***********************************************************************/
/* signals error if not at least <inline 2-byte integer> bytes left in */
/* stack of currently executing process                                */
/***********************************************************************/
OPCODE(Pcheckstack)

/*********************************************************/
/* pops a word off the stack and sets the SP equal to it */
/*********************************************************/
OPCODE(Psetsp)

/****************************/
/*** FLOATING POINT STUFF ***/
/****************************/

/*  TYPE COERCION  */
/*  to coerce
      a char into an Int:  use Ppush1(0)
      an Int into a char:  use Ppop1
*/
OPCODE(Pfl2int)   /*  make a float an Int, bud  */
OPCODE(Pint2fl)   /*  make a (signed) Int into a float  */
OPCODE(Pfl2lng)   /*  make a float a long, bud  */
OPCODE(Plng2fl)   /*  make a (signed) long into a float  */

/*  take two four-byte floating point nums off of stack
    and return answer as floatnum on stack  */
OPCODE(Pfadd)
OPCODE(Pfsub)
OPCODE(Pfmult)
OPCODE(Pfdiv)
OPCODE(Pfequal)
OPCODE(Pflt)
OPCODE(Pfgt)
OPCODE(Pfneg)
OPCODE(Pfsqrt)	/*  square root  */
OPCODE(Pfexp)   /*  e to the x  */
OPCODE(Pf10tx)  /*  10 to the x  */
OPCODE(Pfx2y)   /*  x to the y  */
OPCODE(Pfln)    /*  log base e  */
OPCODE(Pflog)   /*  log base 10 */
OPCODE(Pfatan)  /*  arc tan  */
OPCODE(Pfsin)   /*  sin  */
OPCODE(Pfcos)   /*  cos  */
OPCODE(Pftan)   /*  tan  */


/*****************************/
/*** LCD print debug stuff ***/
/*****************************/
OPCODE(Pfl2ascii)	/* takes flnum, string pointer
			   returns string pointer */
OPCODE(Pprintlcd2)	/* pops & prints word to LCD (hex) */
OPCODE(Pprintf)		/* takes fmt string, args & prints it baby */
OPCODE(Pprintstring)  /* pops addr ptr to null-terminated string */
OPCODE(Pprintchar)	/* pops word and prints low byte as char */


/********************/
/*** Task control ***/
/********************/

OPCODE(Pstartprocess)
OPCODE(Phaltnotify)
OPCODE(Pkillprocess)

OPCODE(Pdefer)

OPCODE(Psystime)

/***********************/
/*** Pcode registers ***/
/***********************/
/*  registers are numbered from 0 to 0 */

OPCODE(Ploadreg)	/*  1-inline byte specifying reg # */
			/*  stack word -> reg */
OPCODE(Pfetchreg)	/*  1-inline byte specifying reg # */
			/*  reg -> stack word */

/*************************/
/*** Special functions ***/
/*************************/
OPCODE(Pbench)		/*  return integer indicating how many machine
			    cycles were able to execute in one millisec
			    of real time */
OPCODE(Pcallml)         /* call machine language subroutine.
			   pops 2 bytes, puts in D
			   pops address, JSR's to this address
			   pushes D onto stack */

OPCODE(Pinitint)	/* initializes 6811 interrupts to
			   pcode defaults, de-installing any
			   binary module routines */

OPCODE(Pcallml2)
OPCODE(Pcallml3)
  

/****************************/
/*** New 32-bit functions ***/
/****************************/

OPCODE(Pdiv4)
OPCODE(Pbitand4)
OPCODE(Pbitor4)
OPCODE(Pbitxor4)
OPCODE(Plognot4)
OPCODE(Plogidn4)
OPCODE(Pbitnot4)

LAST_OPCODE(Pundefined)

#endif

#endif

