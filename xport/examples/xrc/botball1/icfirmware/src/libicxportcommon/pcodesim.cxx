/**
** pcodesim.c
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

/**
** pcodesim.c  Randy Sargent
**
** v1.0 June 1991 created.  works only on machines allowing word fetch at odd
**                          memory locations
** v1.1 Tue Aug 27 11:59:49 1991 modified to allow word fetch at odd memory
**                          locations for processors which do not support it
** v1.5 Wed Jul 28 15:12:29 2004 Kyle Machulis - Fixed simulator to work with 
**							new versions of IC. Nothing like 13 years between
**							log entries.
**/

#include "ICRobot.h"
#ifndef USE_OWN_CLASS

//#include CONFIG
#include "pcodesim.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "CPrintBuffer.h"
#include "ICRobotPBufContext.h"
#include "simptimer.h"

#define SIMULATOR
#include "pcode.h"
#include "pcode_in.h"

//#include "icrobot.h"

int mseconds = 0;
int pcodesim_debug= 0;
static char buf[100];

#define BOARDSIM 1

#if BOARDSIM

int pcodesim_major_version= 2 ;
int pcodesim_minor_version= 81;

int print_reset_on_newline = 0;

int16 pcode_regs[MAX_REGS];

char *pc, *sp;
int err;

char *mem= NULL;

void pprintf(char *fmt);

void execute(void);
void proc_remove(intp proc_ptr);
int proc_start(void);
int proc_kill(int16 pid_to_kill);

#define PROC_PC(p)        MEMP((p) + P_PC)
#define PROC_SP(p)        MEMP((p) + P_SP)
#define PROC_STACK_ORG(p) MEMP((p) + P_STACK_ORG)
#define PROC_STACK_LIM(p) MEMP((p) + P_STACK_LIM)
#define PROC_TICKS(p)     MEM1((p) + P_TICKS)
#define PROC_STATUS(p)    MEM1((p) + P_STATUS)
#define PROC_ID(p)        MEM2((p) + P_ID)
#define PROC_PREV(p)      MEMP((p) + P_PREV)
#define PROC_NEXT(p)      MEMP((p) + P_NEXT)

#define SET_PROC_PC(p,x)        SET_MEMP((p) + P_PC, (x))
#define SET_PROC_SP(p,x)        SET_MEMP((p) + P_SP, (x))
#define SET_PROC_STACK_ORG(p,x) SET_MEMP((p) + P_STACK_ORG, (x))
#define SET_PROC_STACK_LIM(p,x) SET_MEMP((p) + P_STACK_LIM, (x))
#define SET_PROC_TICKS(p,x)     SET_MEM1((p) + P_TICKS, (x))
#define SET_PROC_STATUS(p,x)    SET_MEM1((p) + P_STATUS, (x))
#define SET_PROC_ID(p,x)        SET_MEM2((p) + P_ID, (x))
#define SET_PROC_PREV(p,x)      SET_MEMP((p) + P_PREV, (x))
#define SET_PROC_NEXT(p,x)      SET_MEMP((p) + P_NEXT, (x))

#define INSTRUCTIONS_PER_MSEC 16
#define BOARDSIM 1

#if BOARDSIM
extern char *opcode_name(int opcode);

CSimpTimer timer;
ICRobot* mlTranslator;

void setMLTranslator(ICRobot* robotTranslator)
{
	if(robotTranslator) mlTranslator = robotTranslator;
}

void execute(void)
{
  for (int i= 0; i< 50 && PROC_STATUS(proc_current) == PSTAT_NO_ERROR; i++) {
	uchar op= (uchar) FETCH1;
	int16 a,b;
	int32 c,d;
	float32 f,g;
	intp  p;

	/*
	if (1)
	{
		PrintToBuffer("proc: %04lX pc: %04lX, op: %02X, sp: %04lX ...\n", (int)proc_current, (int)((pc-mem)-1), op, (int)(sp-mem));	
	}
	*/
	switch (op) 
	{
	case 2*Padd2    : b=POP2; a=POP2; PUSH2(a +  b); break;
	case 2*Psub2    : b=POP2; a=POP2; PUSH2( a -  b); break;
	case 2*Pmult2   : b=POP2; a=POP2; PUSH2( a *  b); break;
	case 2*Pdiv2    : 
		{
			b=POP2; a=POP2; 
			if(b==0) 
			{
				SET_PROC_STATUS(proc_current, PSTAT_INTDIVZERO);
				break;
			}
			PUSH2( a /  b); break;
		}
	case 2*Pbitand2 : b=POP2; a=POP2; PUSH2( a &  b); break;
	case 2*Pbitor2  : b=POP2; a=POP2; PUSH2( a |  b); break;
	case 2*Pbitxor2 : b=POP2; a=POP2; PUSH2( a ^  b); break;
	case 2*Pequal2  : b=POP2; a=POP2; PUSH2( a == b); break;
	case 2*Plt2     : b=POP2; a=POP2; PUSH2( a <  b); break;
	case 2*Pgt2     : b=POP2; a=POP2; PUSH2( a >  b); break;
	case 2*Plshift  : b=POP2; a=POP2; PUSH2((short)((b>0) ? a << b : a >> (-b))); break;
	case 2*Plognot2 : a=POP2; PUSH2( !a);  break;
	case 2*Plogidn2 : a=POP2; PUSH2( !!a); break;
	case 2*Pbitnot2 : a=POP2; PUSH2( ~a);  break;
	case 2*Pneg2    : a=POP2; PUSH2( -a);  break;

	case 2*Padd4    : d=POP4; c=POP4; PUSH4( c +  d); break;
	case 2*Psub4    : d=POP4; c=POP4; PUSH4( c -  d); break;
	case 2*Pmult4   : d=POP4; c=POP4; PUSH4( c *  d); break;
	case 2*Pequal4  : d=POP4; c=POP4; PUSH2( c == d); break;
	case 2*Plt4     : d=POP4; c=POP4; PUSH2( c <  d); break;
	case 2*Pgt4     : d=POP4; c=POP4; PUSH2( c >  d); break;
	case 2*Pneg4    : c=POP4; PUSH4( -c);  break;


	case 2*Pdiv4    : d=POP4; c=POP4; PUSH4( c /  d); break;

	case 2*Pbitand4 : d=POP4; c=POP4; PUSH4( c &  d); break;
	case 2*Pbitor4  : d=POP4; c=POP4; PUSH4( c |  d); break;
	case 2*Pbitxor4 : d=POP4; c=POP4; PUSH4( c ^  d); break;
	case 2*Plognot4 : c=POP4; PUSH4( !c);  break;
	case 2*Plogidn4 : c=POP4; PUSH4( !!c); break;
	case 2*Pbitnot4 : c=POP4; PUSH4( ~c);  break;


	case 2*Ppush2: PUSH2( FETCH2); break;
	case 2*Ppush4: PUSH4( FETCH4); break;

	case 2*Ppushblock:
		a=(uchar)FETCH1; GROW_STACK(a); memcpy(sp, pc, a); pc += a;
		break;

	case 2*Ppop2: POP2; break;
	case 2*Ppop4: POP4; break;

	case 2*Ppeeki1: PUSH2( (uchar) MEM1(FETCHP)); break;
	case 2*Ppeeki2: PUSH2( MEM2(FETCHP)); break;
	case 2*Ppeeki4: PUSH4( MEM4(FETCHP)); break;

	case 2*Ppeek1: p= POPP; PUSH2( (uchar) MEM1(p)); break;
	case 2*Ppeek2: p= POPP; PUSH2( MEM2(p)); break;
	case 2*Ppeek4: p= POPP; PUSH4( MEM4(p)); break;

	case 2*Pspeek2: a= STACK_OFFSET2(FETCH1); PUSH2( a); break;
	case 2*Pspeek4: c= STACK_OFFSET4(FETCH1); PUSH4( c); break;

	case 2*Ppokei1: SET_MEM1(FETCHP, (char) POP2); break;
	case 2*Ppokei2: SET_MEM2(FETCHP, POP2);        break;
	case 2*Ppokei4: SET_MEM4(FETCHP, POP4);        break;

	case 2*Ppoke1: a=POP2; SET_MEM1(POPP, (char) a); break;
	case 2*Ppoke2: a=POP2; SET_MEM2(POPP, a);        break;
	case 2*Ppoke4: c=POP4; SET_MEM4(POPP, c);        break;

	case 2*Ppoke1nopop: a=POP2; SET_MEM1(POPP, (char) a); PUSH2(a); break;
	case 2*Ppoke2nopop: a=POP2; SET_MEM2(POPP, a);        PUSH2(a); break;
	case 2*Ppoke4nopop: c=POP4; SET_MEM4(POPP, c);        PUSH4(c); break;

	case 2*Pbitset: a=POP2; p=POPP; SET_MEM1(p, MEM1(p) | (char) a);  break;
	case 2*Pbitclr: a=POP2; p=POPP; SET_MEM1(p, MEM1(p) & (char) ~a);  break;

	case 2*Pspoke2: a=POP2; SET_STACK_OFFSET2((uchar)(FETCH1-2), a); break;
	case 2*Pspoke4: c=POP4; SET_STACK_OFFSET4((uchar)(FETCH1-4), c); break;

	case 2*Pjumpi:  pc= mem + READP(pc); break;
	case 2*Pjump:   pc= mem + POPP; break;
	case 2*Pjfalse: if (!POP2) pc= mem + READP(pc); else FETCHP; break;
	case 2*Pjtrue:  if (POP2)  pc= mem + READP(pc); else FETCHP; break;
	case 2*Pjpfalse: if (!POP2) {pc= mem + READP(pc); PUSH2( 0); }
					else FETCHP; break;
	case 2*Pjptrue:  if (POP2) {pc= mem + READP(pc); PUSH2( 1); }
					else FETCHP; break;

	case 2*Pmret0:
		GROW_STACK(-(FETCH2)); pc= mem + POPP;
		if (pc == mem) {SET_PROC_STATUS(proc_current, PSTAT_DEAD);
		proc_remove(proc_current);}
		break;
	case 2*Pmret2:
		a= POP2; GROW_STACK(-(FETCH2-2)); pc= mem + POPP;
		PUSH2( a);
		if (pc == mem) {SET_PROC_STATUS(proc_current, PSTAT_DEAD);
		proc_remove(proc_current);}
		break;
	case 2*Pmret4:
		c= POP4; GROW_STACK(-(FETCH2-2)); pc= mem + POPP;
		PUSH4( c);
		if (pc == mem) {SET_PROC_STATUS(proc_current, PSTAT_DEAD);
		proc_remove(proc_current);}
		break;
	case 2*Paref1: a= POP2; p= POPP;
		if (a < 0 || a >= MEM2(p)) SET_PROC_STATUS(proc_current, PSTAT_ARRAYBOUNDS);
		else PUSHP( 2 + p + a);
		break;
	case 2*Paref2: a= POP2; p= POPP;
		if (a < 0 || a >= MEM2(p)) SET_PROC_STATUS(proc_current, PSTAT_ARRAYBOUNDS);
		else PUSHP( 2 + p + a*2);
		break;
	case 2*Paref4: a= POP2; p= POPP;
		if (a < 0 || a >= MEM2(p)) SET_PROC_STATUS(proc_current, PSTAT_ARRAYBOUNDS);
		else PUSHP( 2 + p + a*4);
		break;
	case 2*Paref_arb: a= POP2; p= POPP;
		if (a < 0 || a >= MEM2(p)) SET_PROC_STATUS(proc_current, PSTAT_ARRAYBOUNDS);
		else PUSHP( 2 + p + a*FETCH1);
		break;

	case 2*Paddsp: GROW_STACK(-FETCH2); break;
	case 2*Psprel: p= FETCH2 + (sp-mem); PUSHP( p); break;
	case 2*Pcheckstack: FETCH2; break; /* TODO: implement stack bounds checking */
	case 2*Psetsp: p= POPP; sp= mem+p; break;

	case 2*Pfl2int: f= POPF; PUSH2( (int16) f);      break;
	case 2*Pint2fl: a= POP2; PUSHF( (float32) a);    break;
	case 2*Pfl2lng: f= POPF; PUSH4( (int32) f);      break;
	case 2*Plng2fl: c= POP4; PUSHF( (float32) c);    break;

	case 2*Pfadd:   g=POPF; f=POPF; PUSHF(f +  g);   break;
	case 2*Pfsub:   g=POPF; f=POPF; PUSHF(f -  g);   break;
	case 2*Pfmult:  g=POPF; f=POPF; PUSHF(f *  g);   break;
	case 2*Pfdiv:
		{
			g=POPF; f=POPF; 
			if(g < .000001 && g > -.000001) 
			{
				SET_PROC_STATUS(proc_current, DIV0ERR);
				break;
			}
			PUSHF(f /  g);   break;
		}
	case 2*Pfx2y:   g=POPF; f=POPF; PUSHF((float)pow(f,g)); break;

	case 2*Pfequal: g=POPF; f=POPF; PUSH2(f == g);   break;
	case 2*Pflt:    g=POPF; f=POPF; PUSH2(f <  g);   break;
	case 2*Pfgt:    g=POPF; f=POPF; PUSH2(f >  g);   break;

	case 2*Pfneg:   f=POPF; PUSHF( -f);              break;
	case 2*Pf10tx:  f=POPF; PUSHF( (float)pow((float)10.0, f));    break;
	case 2*Pfsqrt:  f=POPF; PUSHF( (float)sqrt(f));         break;
	case 2*Pfexp:   f=POPF; PUSHF( (float)exp(f));          break;
	case 2*Pfln:    f=POPF; PUSHF( (float)log(f));          break;
	case 2*Pflog:   f=POPF; PUSHF( (float)log10(f));        break;
	case 2*Pfatan:  f=POPF; PUSHF( (float)atan(f));         break;
	case 2*Pfsin:   f=POPF; PUSHF( (float)sin(f));          break;
	case 2*Pfcos:   f=POPF; PUSHF( (float)cos(f));          break;
	case 2*Pftan:   f=POPF; PUSHF( (float)tan(f));          break;

	case 2*Pfl2ascii:
		sprintf(buf, "%f", POPF); buf[13]= 0;
		p= FETCHP; strcpy(mem + p, buf);
		break;

	case 2*Pprintlcd2:   POP2; pprintf("  %x"); break;
	case 2*Pprintf:
		a=POP2;	GROW_STACK(-a);
		p= POPP; pprintf(mem + p + 2);
		break;
	case 2*Pprintstring: POPP; pprintf("  %s"); break;
	case 2*Pprintchar:   POP2; pprintf("  %c"); break;

	case 2*Pstartprocess:
		a= proc_start(); PUSH2( a); break;
	case 2*Phaltnotify:	proc_error(PSTAT_HALTED);  break; 
	case 2*Pkillprocess: PUSH2( proc_kill(POP2)); break;

	case 2*Pdefer: SET_MEM1(PCODE_PROCESS_TICKS,0); //die(("defer unimplemented")); 
		break;
	case 2*Psystime:
		//PUSH4(TimerSingleton.GetMillisecondsSinceBoot());
		long long unsigned count;
		timer.GetCount(&count);
		PUSH4(count/1000);
		break;

	case 2*Ploadreg:  pcode_regs[(uchar)FETCH1]= POP2;  break;
	case 2*Pfetchreg: PUSH2( pcode_regs[(uchar)FETCH1]); break;

	case 2*Pbench:  PUSH2( 1000); break;
	case 2*Pinitint:  /* NOP for us */ break;
	case 2*Pcallml:  b=POP2; a=POP2; PUSH2(mlTranslator ? mlTranslator->CallML1Translator(a,b) : 0); break;
	case 2*Pcallml2: c=POP2; b=POP2; a=POP2;  PUSH2(mlTranslator ? mlTranslator->CallML2Translator(a,b,c) : 0); break;
	case 2*Pcallml3: d=POP2; c=POP2; b=POP2; a=POP2;  PUSH2(mlTranslator ? mlTranslator->CallML3Translator(a,b,c,d) : 0); break;
	default:
		PrintToBuffer("Illegal opcode %02X\n", op);
		break;
	}
	//if (pcodesim_debug)tdSim.PrintToBuffer"sp: %04lX\n", (int)(sp-mem));
  }
}

int proc_start(void)
{
	intp proc_ptr= PROCESS_TABLE;
	intp best_fit= 0;
	int  best_error= 0x7fff;
	intp stack_size_requested;
	intp new_pc;
	int16 stack_bytes_to_copy;
	int16 ticks;
	int  count= MAX_PROCESSES;

	stack_size_requested= POPP;
	ticks= POP2;
	new_pc= POPP;
	stack_bytes_to_copy= POP2;
	GROW_STACK(- stack_bytes_to_copy);
	/* Find stack space.  Processes are in process table in order of their
	stack locations.  Look at spaces between processes to find unused stack. */
	while (1) 
	{
		intp next_ptr= PROC_NEXT(proc_ptr);
		intp stack_begin, stack_end, stack_size, idproc;

		idproc = PROC_ID(proc_ptr);
		if (!count--) {
			PrintToBuffer("Corrupt process table in proc_start\n");
			return 0;
		}

		stack_begin= PROC_STACK_LIM(proc_ptr);

		if (next_ptr == PROCESS_TABLE) {
			/* Looking at the last process;  we have the rest of the total stack */
			stack_end= PCODE_STACK_END;
		}
		else {
			stack_end= PROC_STACK_ORG(next_ptr);
		}

		if (stack_begin < stack_end) {
			PrintToBuffer("Process table corrupt -- stack_begin < stack_end\n");
			stack_size= 0;
		}
		else stack_size= stack_begin - stack_end;

		if (stack_size >= stack_size_requested) {
			int this_error= stack_size_requested - stack_size;
			if (this_error < best_error) {
				best_error= this_error;
				best_fit= proc_ptr;
			}
		}
		if (next_ptr == PROCESS_TABLE) break;
		proc_ptr = next_ptr;
	}

	/* Finished with the list of processes */
	if (best_error == 0x7fff) {
		/* Couldn't find a stack space big enough */
		proc_error(PSTAT_NONEWSTACKSPACE);
		return 0;
	}

	/* Now find a process slot.  Look thru processes to find one that is
	PSTAT_DEAD */
	proc_ptr= PROCESS_TABLE + PROCESS_SLOT_LENGTH; /* skip UI process */

	while (1) {
		if (PROC_STATUS(proc_ptr) == PSTAT_DEAD) break;
		proc_ptr += PROCESS_SLOT_LENGTH;
		if (proc_ptr >= PROCESS_TABLE + PROCESS_SLOT_LENGTH * MAX_PROCESSES) {
			/* No process slots available */
			proc_error(PSTAT_NOPROCSPACE);
			return 0;
		}
	}

	/* copy initial vals into new proc slot */

	/* install new slot into proc list */
	SET_PROC_NEXT(proc_ptr, PROC_NEXT(best_fit));
	SET_PROC_PREV(proc_ptr, best_fit);

	SET_PROC_PREV(PROC_NEXT(proc_ptr), proc_ptr);
	SET_PROC_NEXT(best_fit, proc_ptr);

	/* copy initial vals into new proc slot */

	SET_PROC_STACK_ORG(proc_ptr, PROC_STACK_LIM(best_fit) - 1);
	SET_PROC_STACK_LIM(proc_ptr, PROC_STACK_ORG(proc_ptr) + 1
		- stack_size_requested);
	/* the following line allows for pids to get re-used */
	if(PROC_ID(proc_ptr) <= 1 || PROC_ID(proc_ptr) > proc_counter)  
		SET_PROC_ID(proc_ptr, ++proc_counter);
	SET_PROC_PC(proc_ptr, new_pc);
	SET_PROC_STATUS(proc_ptr, PSTAT_RUNNING);
	SET_PROC_TICKS(proc_ptr, (char) ticks);
	/* 3. copy stack into new proc stack */
	memcpy(mem + PROC_STACK_ORG(proc_ptr) - stack_bytes_to_copy,
		sp - stack_bytes_to_copy,
		stack_bytes_to_copy);
	SET_PROC_SP(proc_ptr, PROC_STACK_ORG(proc_ptr) - stack_bytes_to_copy);

	return PROC_ID(proc_ptr);
}

int proc_kill(int16 pid_to_kill)
{
	intp process_ptr= PROCESS_TABLE;
	int count= MAX_PROCESSES;

	while (count--) {
		int16 pid= PROC_ID(process_ptr);
		if (pid == pid_to_kill) {
			SET_PROC_STATUS(process_ptr, PSTAT_DEAD);
			/*
			if (process_ptr == proc_current) {
				// Suicide 
				proc_error(PSTAT_DEAD);
			}
		*/
			proc_remove(process_ptr);
			return 0;
		}
		process_ptr= PROC_NEXT(process_ptr);
		if (process_ptr == PROCESS_TABLE) return 1;
	}
	PrintToBuffer("Corrupt process table; did not kill process\n");
	return 0;
}

void proc_check(intp proc_ptr)
{
	int offset= proc_ptr - PROCESS_TABLE;

	if ((offset % PROCESS_SLOT_LENGTH) != 0 ||
		offset < 0 ||
		(offset / PROCESS_SLOT_LENGTH) >= MAX_PROCESSES) {
			PrintToBuffer("Illegal process %04X\n", proc_ptr);
		}
}

void proc_remove(intp proc_ptr)
{
	intp prev, next;
	proc_check(proc_ptr);

	prev= PROC_PREV(proc_ptr);
	next= PROC_NEXT(proc_ptr);

	proc_check(prev);
	proc_check(next);

	SET_PROC_NEXT(prev, next);
	SET_PROC_PREV(next, prev);
}

void sprintf_binary_byte(char* cstrBuffer, int16 b)
{
	int i;
	for (i= 0; i< 8; i++) {
		if (b & 128) sprintf(cstrBuffer, "1"); 
		else sprintf(cstrBuffer, "0"); 
		++cstrBuffer;
		b <<= 1;
	}
}

void pprintf(char *format)
{
	int c, idx;
	idx = 0;
	char* ptr= sp-sizeof(intp);
	char pbuf[PRINT_BUFFER_SIZE+4];
	char* buffer = pbuf;
	*buffer = 0;

	while ((c= *format++)) {
//		if(strlen(pbuf) >= PRINT_BUFFER_SIZE) continue;
		if (c != '%') {
			sprintf(buffer, "%c", c);
		}
		else {
			switch (c= *format++) {
		  case 0: goto done;
		  case '%': sprintf(buffer, "%c", '%'); break;
		  case 'd': ptr -= 2; sprintf(buffer,
						"%ld", (long)READ2(ptr)); break;
		  case 'c': ptr -= 2; sprintf(buffer,
						"%c", (int)READ2(ptr)); break;
		  case 'x': ptr -= 2; sprintf(buffer,
						"%04lX",0xffff & (unsigned long)READ2(ptr)); break;
		  case 'b': ptr -= 2; 
			  sprintf_binary_byte(buffer,	READ2(ptr)); 
			  break;
		  case 'f': ptr -= 4; sprintf(buffer,
						"%f", READF(ptr)); break;
		  case 'l': ptr -= 4; sprintf(buffer,
						"%ld", READ4(ptr)); break;
		  case 's': ptr -= 2; sprintf(buffer,
						"%s", mem +2+ READP(ptr)); break;
		  default: break;
			}
		}
		buffer= pbuf+strlen(pbuf);

	}
done:	
	g_printBuffer->PrintfIC("%s", pbuf);

	// This accomplishes showing the user the IC print buffer the
	// first time something is printed because otherwise the
	// novice user may not realized how to find what was
	// printed...
	static bool firstPrint=true;
	if(mlTranslator != NULL && firstPrint) {
	  g_printBuffer->ShowICPrintBuffer();
	  ICRobotPBufContext::Push(*g_printBuffer, *mlTranslator);
	  firstPrint=false;
	}

	PUSH2(0);
}

intp proc_current;
int16 proc_counter;
int proc_error_flag;
int simulator_is_running = 0;

void pcodesim_init(void)
{
	if (!mem) mem= (char*)malloc(65535);
	memset(mem, 0, 65535);
}

INLINE uchar read1(char *p)
{
	return (uchar) *p;
}

INLINE int16 read2(char *p)
{
	return
		(((int16)(uchar)READ1(p+0)) <<  8) +
		(((int16)(uchar)READ1(p+1)) <<  0);
}

INLINE int32 read4(char *p)
{
	return
		(((int32)(uchar)READ1(p+0)) << 24) +
		(((int32)(uchar)READ1(p+1)) << 16) +
		(((int32)(uchar)READ1(p+2)) <<  8) +
		(((int32)(uchar)READ1(p+3)) <<  0);
}

INLINE float readf(char *p)
{
	int32 ret= read4(p);
	return *(float*)&ret;
}

INLINE void write1(char *p, uchar x)
{
	*p = (char) x;
}

INLINE void write2(char *p, int16 x)
{
	WRITE1(p+0, (char) (x >>  8));
	WRITE1(p+1, (char) (x >>  0));
}

INLINE void write4(char *p, int32 x)
{
	WRITE1(p+0, (char) (x >> 24));
	WRITE1(p+1, (char) (x >> 16));
	WRITE1(p+2, (char) (x >>  8));
	WRITE1(p+3, (char) (x >>  0));
}

INLINE uchar fetch1(void)
{
	uchar ret= read1(pc);
	pc += 1;
	return ret;
}

INLINE int16 fetch2(void)
{
	int16 ret= read2(pc);
	pc += 2;
	return ret;
}

INLINE int32 fetch4(void)
{
	int32 ret= read4(pc);
	pc += 4;
	return ret;
}

INLINE void push1(uchar x)
{
	sp -= 1;
	write1(sp, x);
}

INLINE void push2(int16 x)
{
	sp -= 2;
	write2(sp, x);
}

INLINE void push4(int32 x)
{
	sp -= 4;
	write4(sp, x);
}

INLINE void pushf(float x)
{
	push4(*(int32*)&x);
}

INLINE uchar pop1(void)
{
	uchar ret= read1(sp);
	sp += 1;
	return ret;
}

INLINE int16 pop2(void)
{
	int16 ret= read2(sp);
	sp += 2;
	return ret;
}

INLINE int32 pop4(void)
{
	int32 ret= read4(sp);
	sp += 4;
	return ret;
}

INLINE float popf(void)
{
	int32 ret= pop4();
	return *(float*)&ret;
}

void pcodesim_wakeup(void)
{
	proc_current= PROCESS_TABLE;
	SET_MEM1(PCODE_PROCESS_TICKS, PROC_TICKS(proc_current));
	pc= mem+PROC_PC(proc_current);
	sp= mem+PROC_SP(proc_current);
}

/* returns 1 if single process halted, 2 if all processes halted */
int pcodesim_execute_some(void)
{
	int count= 0;
	int timeToRun = 0;
	proc_error_flag= 0;
	intp p_check = proc_current;
	CSimpTimer timer;
	long long unsigned startTime, currentTime;
	while (PROC_STATUS(proc_current) != PSTAT_NO_ERROR)
	{
	  //SET_PROC_PC(proc_current, pc-mem);
	  //SET_PROC_SP(proc_current, sp-mem);
		proc_current= PROC_NEXT(proc_current);
		SET_MEM1(PCODE_PROCESS_TICKS, PROC_TICKS(proc_current));
		pc= mem+PROC_PC(proc_current);
		sp= mem+PROC_SP(proc_current);
		//If we've fully looped, nothing is running, we're done
		if(p_check == proc_current)
		{
			return 2;
		}		
	}
	//PrintToBuffer("Proc %d\n", PROC_ID(proc_current));
	while (MEM1(PCODE_PROCESS_TICKS) > 0) 
	{
		if(pc == 0) return 1;
		if (PROC_STATUS(proc_current) == PSTAT_HALTED || PROC_STATUS(proc_current) == PSTAT_DEAD)
		{
			proc_error_flag = 1;
//			//PrintToBuffer("process halted\n");
			break;
		}
		if (PROC_STATUS(proc_current) != PSTAT_NO_ERROR)
		{
			proc_error(PROC_STATUS(proc_current));
//			//PrintToBuffer("breaking on process error\n");
			break;
		}
		timer.GetCount(&startTime);
		timer.GetCount(&currentTime);
		while(currentTime - startTime < 1000 && (MEM1(PCODE_PROCESS_TICKS) > 0) && PROC_STATUS(proc_current) == PSTAT_NO_ERROR)
		{
			timer.GetCount(&currentTime);
			execute();
		}
		timeToRun = ((currentTime - startTime)/1000);
		count += timeToRun;
		if (MEM1(PCODE_PROCESS_TICKS) > timeToRun) //count == 10 &&
		{
			SET_MEM1(PCODE_PROCESS_TICKS, MEM1(PCODE_PROCESS_TICKS) - timeToRun);
		}
		else
		{
			SET_MEM1(PCODE_PROCESS_TICKS, 0);
		}
		//We increment count once per 
		if(count > 30)
		{
			return 0;
		}
	}
	SET_PROC_PC(proc_current, pc-mem);
	SET_PROC_SP(proc_current, sp-mem);
	proc_current= PROC_NEXT(proc_current);
	SET_MEM1(PCODE_PROCESS_TICKS, PROC_TICKS(proc_current));
	pc= mem+PROC_PC(proc_current);
	sp= mem+PROC_SP(proc_current);
	if (proc_error_flag) return 1; else return 0;
}

void pcodesim_reset(void)
{
	int i;
	for (i= 0; i< MAX_PROCESSES; i++) 
	{
		SET_PROC_STATUS(PROCESS_TABLE + i*PROCESS_SLOT_LENGTH, PSTAT_DEAD);
	}
	SET_PROC_PC(PROCESS_TABLE, UI_PROCESS_BUFFER);
	SET_PROC_STACK_ORG(PROCESS_TABLE, PCODE_STACK_BEGIN);
	SET_PROC_SP(PROCESS_TABLE, PCODE_STACK_BEGIN + 1);
	SET_PROC_STACK_LIM(PROCESS_TABLE, PCODE_STACK_BEGIN - UI_STACK_SIZE + 1);
	SET_PROC_PREV(PROCESS_TABLE, PROCESS_TABLE);
	SET_PROC_NEXT(PROCESS_TABLE, PROCESS_TABLE);
	SET_PROC_TICKS(PROCESS_TABLE, UI_TICKS);
	SET_PROC_STATUS(PROCESS_TABLE, PSTAT_HALTED);

	proc_counter= 1;
	SET_PROC_ID(PROCESS_TABLE, 1);
	pcodesim_wakeup();
}

void proc_error(int16 err)
{
	char buff[1000];
	const char* name = "NONE";
	for (int i= 0; system_errors[i].name; i++) {
		if (system_errors[i].id == err) {
			name= system_errors[i].name;
			break;
		}
	}
	sprintf(buff, "%s while executing (process #%d)\n", name, PROC_ID(proc_current));
//	if (pcodesim_debug) debugf("set proc status to %ld\n", (int) err);
	if(err != 64 && err != 32) pprintf(buff);
	SET_PROC_STATUS(proc_current, (char)err);
	proc_error_flag= 1;
}
#endif


#endif

#endif /* #ifndef USE_OWN_CLASS */