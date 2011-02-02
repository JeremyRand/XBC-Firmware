#ifndef CYGONCE_HAL_PLATFORM_SETUP_H
#define CYGONCE_HAL_PLATFORM_SETUP_H

/*=============================================================================
//
//      hal_platform_setup.h
//
//      Platform specific support for HAL (assembly code)
//
//=============================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, rich@charmedlabs.com
// Date:        2002-08-13
// Purpose:     Game Boy Advance specific support routines
// Description: 
// Usage:       #include <cyg/hal/hal_platform_setup.h>
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#define PLATFORM_SETUP1				_platform_setup
#define PLATFORM_PREAMBLE			_platform_preamble
#define PLATFORM_VECTORS			_platform_vectors

#define ARM_MODE(_r_, _l_)                       \
        ldr     _r_,=_l_ ## f                   ;\
        bx      _r_                             ;\
        .pool                                   ;\
        .code   32                              ;\
 _l_:


//
// _platform_setup
//
        .macro  _platform_setup
// setup gba interrupt address
        ldr     r1, .int_addr					  
        adr     r0, GBA_IRQ
        str     r0, [r1]
		.endm

		
//
// _platform_preamble
//
        .macro  _platform_preamble
#ifdef CYG_HAL_STARTUP_RAM
// RAM startup (we assume that we are already in thumb mode)
        .ALIGN
        .code 16
        ARM_MODE(r0,10)
        b       reset_vector

#endif
#ifdef CYG_HAL_STARTUP_ROM 
// ROM requires ROM header
        .ALIGN
        .code 32
rom_header:
        b       rom_header_end

// Nintendo Logo Character Data (8000004)
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00

// Software Titles (80000A0)
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        .byte   0x00,0x00,0x00,0x00

// Initial Code (80000AC)
        .byte   0x00,0x00,0x00,0x00

// Maker Code (80000B0)
        .byte   0x30,0x31

// Fixed Value (80000B2)
        .byte   0x96

// Main Unit Code (80000B3)
        .byte   0x00

// Device Type (80000B4)
        .byte   0x00

// Unused Data (7Byte) (80000B5)
        .byte   0x00,0x00,0x00,0x00,0x00,0x00,0x00

// Software Version No (80000BC)
        .byte   0x00

// Complement Check (80000BD)
        .byte   0xf0

// Check Sum (80000BE)
        .byte   0x00,0x00

rom_header_end:
        b       reset_vector

        .byte   0       // boot method 
	    .byte   0       // slave number
        .byte   0       // reserved
        .byte   0       // reserved
        .word   0       // reserved
        .word   0       // reserved
        .word   0       // reserved
        .word   0       // reserved
        .word   0       // reserved
        .word   0       // reserved
#endif			
		.endm                                        


//
// _platform_vectors
//
        .macro  _platform_vectors
#if defined(CYG_HAL_STARTUP_ROM)
//
// "Patch vectors" - used to patch illegal instruction into debug stub code
//  This code gets mapped to 0x9ffc000 (mirrored at 0x83fc000 for 4mb flash)
//
#if defined(CYGHWR_GBAX_VERSION_1_1) 
        .section ".patch_vectors", "ax"
#else
		.text
#endif
		.code   32

hal_patch_vector_code:
		.globl	hal_patch_vector_code
		// restore registers and return to user code (not bios, which returns incorrectly)
		ldr		r13, .pv_cl1
		ldmfd	r13!, {r12,r14}
		msr		spsr, r12
		ldmfd	r13!, {r12,r14}
        ldr		pc, .pv_cl0  
.pv_cl0:
		.word	undefined_instruction
.pv_cl1:
		.word	0x3007fe0
#endif
		.text
GBA_IRQ:
		// restores registers and stack from GBA bios
		ldmfd   r13!,{r0-r3,r12,r14}
		b		IRQ

.int_addr:
		.word	0x3007ffc
#if defined(CYG_HAL_STARTUP_ROM)
.hal_patch_vector_code: 
		.word	hal_patch_vector_code
#endif
        .endm                                        

#undef ARM_MODE(_r_, _l_) 

/*---------------------------------------------------------------------------*/
/* end of hal_platform_setup.h                                               */
#endif /* CYGONCE_HAL_PLATFORM_SETUP_H */
