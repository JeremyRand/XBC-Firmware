#ifndef PCODE_H
#define PCODE_H
/**
 ** pcode.h
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


/*-------------------------------------------------------------------------*/
/* Public Constants                                                        */

#ifdef COMPILER
#define HOST 1
#endif

#ifdef SIMULATOR
#define HOST 1
#define PCODE_DEFINE_ERRORS
#endif

/******************************/
/***                        ***/
/*** 6811 MEMORY MANAGEMENT ***/
/***                        ***/
/******************************/

/* default start of Pcode object; builds upwards */
#define PCODE_ORIGIN 0x200

/*  start of global stack in RAM; builds downwards */
#define GLOBALS_ORIGIN 0xBFC0

/* 6811 interrupt vectors */
/* #define SPECIAL_VECTORS 0xBFC0 */  /* define in Makefile */

/* pcode jumptable; must be aligned on page boundary */
#define JUMPTABLE 0xC000

/* string variable space */
/* temp buffer for composing nums; 16 bytes long */
#define STRING_TEMP_BUFFER 0xC100
/* buffer for printed chars */
#define PRINT_BUFFER 0xC110
#define PRINT_BUFFER_SIZE 0x50

/* downloaded pcode buffer */
#define UI_PROCESS_BUFFER 0xC200
#define UI_PROCESS_BUFFER_END 0xC2FF

/* scheduler table start */
#define PROCESS_TABLE 0xC300
#define PROCESS_SLOT_LENGTH 16
#define MAX_PROCESSES 32

/* start of pcode interpreter */
#define MAIN_CODE 0xC500

/* start of pcode stack (grows downward) */
#define PCODE_STACK_END 0xE400
#define PCODE_STACK_BEGIN 0xFE00

/* machine stack  */
#define MACHINE_STACK 0xFEFF

/* the zero length array.  there is a "0" word stored here */
/* incredibly, this location is also used to store a
   checksum of memory; if this word is NOT zero, then a
   checksum is stored there, and it should be the checksum
   of memory.
*/   
#define THE_ZERO_ARRAY 0xFF02

/* version number */
#define VERSION_NUMBER 0xFF00

/**********************/
/***                ***/
/*** PROCESS STATUS ***/
/***                ***/
/**********************/


/*****************************************************************/
/* SCHEDULER TABLE                                               */
/*                                                               */
/* Format:                                                       */
/*                                                               */
/* Process 0 is "UI" process.                                    */
/* Process 1 is "main" process.                                  */
/* Process 2 through N are processes spawned by other processes. */
/*                                                               */
/* Table format:                                                 */
/* SCHEDULER_TABLE EQU                                           */
/* (Process 0)	current PC	2 bytes 	P_PC		 */
/* 		current SP	2 bytes         P_SP             */
/* 		stack origin	2 bytes         P_STACK_ORG      */
/* 		stack limit	2 bytes         P_STACK_LIM      */
/* 		ticks		1 byte          P_TICKS          */
/* 		status		1 byte 		P_STATUS	 */
/*		process ID	2 bytes		P_ID		 */
/*	        prev_proc_addr  2 byte		P_PREV		 */
/*	        next_proc_addr  2 byte		P_NEXT		 */
/*                                                               */
/* (Process 1)			16 bytes			 */
/* (Process 2)                  16 bytes			 */
/* ...                                                           */
/* ...                                                           */
/* ...                                                           */
/* ...                                                           */
/* ...                                                           */
/* (Process N)                  16 bytes			 */
/*****************************************************************/

/* scheduler table offset defs */
#define P_PC 0
#define P_SP 2
#define P_STACK_ORG 4
#define P_STACK_LIM 6
#define P_TICKS 8
#define P_STATUS 9
#define P_ID 10
#define P_PREV 12
#define P_NEXT 14

/* process defaults */
#define PSTACK_DEFAULT_SIZE 256
#define PROC_DEFAULT_TICKS 5
/* UI process defaults */
#define UI_STACK_SIZE 256
#define UI_TICKS 10

/* process status codes */
#define PSTAT_RUNNING 0

#ifdef PCODE_INTERPRETER
#define PSTAT_ERROR(token, number, desc)token EQU number
#endif
  
#ifdef HOST
#define PSTAT_ERROR(token, number, desc) { number, desc },
#ifndef PCODE_DEFINE_ERRORS
#define INHIBIT_ERRORS
#endif  
#endif


#ifndef INHIBIT_ERRORS
#ifdef HOST  
struct {int id; char *name;} system_errors[]= {
#endif

PSTAT_ERROR(PSTAT_NO_ERROR,        0, "System is peachy-keen")  
PSTAT_ERROR(PSTAT_NONEWSTACKSPACE, 1, "Not enough stack memory for new process")
PSTAT_ERROR(PSTAT_NOPROCSPACE,     2, "Too many processes")
PSTAT_ERROR(PSTAT_ARRAYBOUNDS,     3, "Array reference out of bounds")
PSTAT_ERROR(PSTAT_STACKOVERFLOW,   4, "Out of stack space")
PSTAT_ERROR(PSTAT_NULLPOINTER,     5, "Use of uninitialized or NULL pointer")
PSTAT_ERROR(OVFERR,                6, "Floating point overflow")
PSTAT_ERROR(UNFERR,                7, "Floating point underflow")
PSTAT_ERROR(DIV0ERR,               8, "Division by zero (floating point)")
PSTAT_ERROR(TOLGSMER,              9,
	    "Floating point number too large or small for conversion to integer")
PSTAT_ERROR(NSQRTERR,             10, "Squareroot of negative number")
PSTAT_ERROR(TAN90ERR,             11, "Tangent of illegal angle")
PSTAT_ERROR(LNNEGERR,             12, "Logarithm of negative number")

  /* 13 - 16 reserved for floating point library */
  
PSTAT_ERROR(PSTAT_INTDIVZERO,     16, "Division by zero (integer)")

PSTAT_ERROR(PSTAT_UNKNOWN_ERROR,  31, "!!UNKNOWN ERROR!!")
#ifdef HOST
};
#endif
#endif /* #ifndef INHIBIT_ERRORS */

/* error codes are from 0 to 31 */
#define PSTAT_NO_ERROR 0
#define PSTAT_NONEWSTACKSPACE 1
#define PSTAT_NOPROCSPACE 2
#define PSTAT_ARRAYBOUNDS 3
#define PSTAT_STACKOVERFLOW 4
#define PSTAT_NULLPOINTER 5
#define OVFERR 6
#define UNFERR 7
#define DIV0ERR 8
#define TOLGSMER 9
#define NSQRTERR 10
#define TAN90ERR 11
#define LNNEGERR 12

#define PSTAT_INTDIVZERO 16

#define PSTAT_UNKNOWN_ERROR 31

#define PSTAT_HALTED 32
#define PSTAT_DEAD 64
// Flag set and reset by host
#define PSTAT_DISABLED 128

/* system status codes */
#define SYSSTAT_NOERROR 0
#define SYSSTAT_IOWAITING 1
#define SYSSTAT_ERRORHALT 2
#define SYSSTAT_HALTNOTIFY 4
#define SYSSTAT_IGNOREUI 8
#define SYSSTAT_NOLCD 16


/* lcd command and status/phase codes */
/* load into A register and call driver routine */
#define LCDCMD_QUERYBUSY 4
	/*  if C flag set on return, LCD was busy  */
#define LCDCMD_ASCII 2
#define LCDCMD_CMD 0

/* print character normally */
#define LCDSTAT_PRINTCHAR 0
/* print character, but clear screen first.  Saves char t/b printed.  */
#define LCDSTAT_PRINTCHARCLS 1
/* random codes until finish cls */
/* first of these must be numbered '2' */
#define LCDSTAT_CLS2 2
#define LCDSTAT_CLS3 4
#define LCDSTAT_CLS4 6
#define LCDSTAT_CLS5 8
#define LCDSTAT_FINISHCLS 10

#define PCODE_PROCESS_TICKS 10
#define PCODE_SYSTEM_STATUS 11

#define PCODE_SYSTEM_TIME_HI 18
#define PCODE_SYSTEM_TIME_LO 20
#endif
