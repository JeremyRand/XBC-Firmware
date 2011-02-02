/*****************************************************************************\
                    DevKit Advance C Runtime Initialization
\*****************************************************************************/

/* DevKit Advance License:
 *
 * Copyright 2003, Jason Wilkins.  This source code is free for any use except
 * that this copyright notice and the following disclaimers remain intact when
 * the source is distributed.  Object code and binary distributions may be made
 * as if the code were in the public domain.
 *
 * THIS CODE WAS NOT MADE IN ASSOCIATION WITH NINTENDO AND DOES NOT MAKE USE OF
 * ANY INTELLECTUAL PROPERTY CLAIMED BY NINTENDO.
 *
 * GAMEBOY ADVANCE IS A TRADEMARK OF NINTENDO.
 *
 * THIS CODE HAS BEEN PROVIDED "AS-IS" WITHOUT A WARRANTY OF ANY KIND, EITHER
 * EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO IMPLIED WARRANTIES OF
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR USE.  THE ENTIRE RISK AS TO THE
 * QUALITY OR PERFORMANCE OF THE CODE IS WITH YOU.
 *
 * IN NO EVENT, UNLESS AGREED TO IN WRITING, WILL ANY COPYRIGHT HOLDER, OR ANY
 * OTHER PARTY, BE HELD LIABLE FOR ANY DAMAGES RESULTING FROM THE USE OR
 * INABILITY TO USE THIS CODE.
 *****************************************************************************/
#include "asm-macros.h"
#include "config-crt0.h"

.text

.align

/*****************************************************************************\
                                     Header
\*****************************************************************************/

#if HEADER
/*Nintendo Header:
 * A special header is required or the GBA will refuse to run your code.
 *
 * Please see $(DEVKITADV)/docs/Nintendo-Logo.txt for more information.
 *
 * Throughout the description of the header, the notation [00] is used to mean
 * a file offset, or alternatively, an offset from 0x0200000 or 0x08000000.
 *****************************************************************************/
    FUNCTION(_start)
	b	.Lreal_start			// [00] Entry Point
    END(_start)

    STATIC_OBJECT(__crt0_rom_header)
#if NINTENDO_LOGO
	.include "nintendo-logo.S"		// [04] Nintendo Logo
#else
	.fill	156, 1, 0			// [04] Nintendo Logo
#endif
	.ascii	GAME_TITLE			// [A0] Game Title
	.ascii	GAME_CODE			// [AC] Game Code
	.ascii  MAKER_CODE			// [B0] Maker Code
	.byte	0x96				// [B2] Fixed Value
	.byte	0				// [B3] Main Unit Code
	.byte	0				// [B4] Device Type
	.byte	0, 0, 0, 0, 0, 0, 0		// [B5] Reserved (7 Bytes)
	.byte	0				// [BC] Software Version No.
	.byte	COMPLEMENT_CHECK		// [BD] Complement Check
	.byte	0, 0				// [BE] Reserved
    END(__crt0_rom_header)

   .Lheader_end:
#else
    // even if a header is not requested, the _start symbol is still required
    FUNCTION(_start)
    END(_start)
#endif

#if MULTIBOOT_HEADER
/* Multiboot Header:
 * The following header is required if the code is meant for Multiboot.
 *
 * If the code has been downloaded through the serial port, then the GBA BIOS
 * will set [C0] depending on the boot method:
 *
 * 1 = JoyBus, 3 = Multiboot
 *
 * It remains 0 for cartridges.
 *
 * [C4] will be set to the GBA's assigned slave number 1-3.
 *
 * This header also defines the symbols __boot_method and __slave_number for
 * easy reference to these values.  Some libraries may depend on them whether
 * or not the code is meant for Multiboot.
 *****************************************************************************/
    STATIC_FUNCTION(__crt0_multiboot_start)
	b	.Lreal_start			// [C0] Multiboot Entry Point
    END(__crt0_multiboot_start)

    OBJECT(__boot_method)
	.byte	0				// [C4] Boot Method
    END(__boot_method)

    OBJECT(__slave_number)
	.byte	0				// [C5] Slave Number (1-3)
    END(__slave_number)

	.align
    STATIC_OBJECT(__crt0_mb_reserved)
	.word	0, 0, 0, 0, 0, 0		// [C8] Reserved (6 words)
    END(__crt0_mb_reserved)
#endif

#if JOYBUS_HEADER
    STATIC_FUNCTION(__crt0_joybus_start)
	b	.Lreal_start			// [E0] JoyBus Entry Point
    END(__crt0_joybus_start)
#endif

#if EXTENDED_HEADER
/* DevKit Advance Extended Header:
 * This header is located at either [C0], [E0], or [E4] depending on which
 * headers are present.  It is easy to tell the difference between an ARM
 * instruction used as an entry point and __crt0_footer_ptr.
 *
 * Just look for a word whose most significant byte is 0x02, 0x08, or 0x09 at
 * [C0], then [E0], then [E4].  An ARM instruction will start with 0xE.
 *****************************************************************************/
    STATIC_OBJECT(__crt0_footer_ptr)
 	.word	__load_start_footer		// [C0|E0|E4] Footer Pointer
    END(__crt0_footer_ptr)
#endif



   .Lreal_start:



/*****************************************************************************\
                           C Runtime Initialization
\*****************************************************************************/

#if INIT_STACKS
/* Initialize IRQ and USR Stack Pointers:
 * There are default definitions of __sp_irq_initial_value and __sp_usr_initial_value provided
 * below, but they can be overridden by user supplied definitions.
 *****************************************************************************/
    STATIC_FUNCTION(__crt0_init_stacks)

	mov	r0, #0x12
	msr	cpsr, r0			// switch to IRQ mode
	ldr	sp, =__sp_irq_initial_value
	ldr	sp, [sp]			// sp = *__sp_irq_initial_value

	mov	r0, #0x1F
	msr	cpsr, r0			// switch to USR mode
	ldr	sp, =__sp_usr_initial_value
	ldr	sp, [sp]			// sp = *__sp_usr_initial_value

    END(__crt0_init_stacks)
#endif

/* Switch to Thumb Mode:
 * The bulk of crt0 is written in thumb assembly language.
 *****************************************************************************/
    STATIC_FUNCTION(__crt0_bx_thumb)

	adr	r0, .Lthumb + 1
	bx	r0

	.thumb
   .Lthumb:

    END(__crt0_bx_thumb)

/* Reduce gameboy waitstates
 *****************************************************************************/
	STATIC_THUMB_FUNCTION(__reduce_waitstates)
	.equ __gba_wscnt, 0x4000204
	ldr     r1, =__gba_wscnt
	ldrh    r0, =0x4490
	strh    r0, [r1]
    END(__reduce_waitstates)

#if LOAD_MULTIBOOT
/* Load Multiboot Image from ROM into RAM:
 * Check to see if the image is meant for Multiboot or GamePak.  If it is for
 * Multiboot then check if it is currently running from EWRAM or from CARTROM.
 * If it is running from CARTROM, then it needs to be copied to EWRAM and
 * re-executed from the beginning.
 *
 * The reason for all this is to allow a program to be used "as-is" with a
 * flash-cart, emulator, or MBV2-style Multiboot cable without rebuilding.
 *
 * NOTE: Anything put above this code needs to be written so that it will not
 * crash if it relies on an absolute address.  Because execution will be
 * restarted, results are allowed to be temporarily incorrect.  Any branchs used
 * above this code need to be relative.
 *****************************************************************************/
    STATIC_THUMB_FUNCTION(__crt0_load_multiboot)
	ldr	r0, =_start		// 8000000h=GamePak 2000000h=Multiboot
	ldr	r1, =__memmap_rom_start

	cmp	r0, r1
	beq	.Lno_load_multiboot	// skip if GamePak

	mov	r3, pc
	cmp	r1, r3			// check program counter
	bhi	.Lno_load_multiboot	// skip if already running from EWRAM

	sub	r3, r1, r0		// difference between _start and CARTROM
	ldr	r2, =__load_break_appended_ptr
	add	r2, r2, r3		// adjust pointer into ROM
	ldr	r2, [r2]
	add	r2, r2, r3		// adjust pointer into ROM

	// Assuming that _start is 2000000h then:
	// r0:dst       = 2000000h (base of EWRAM)
	// r1:src_start = 8000000h (base of CARTROM)
	// r2:src_end   = *(__load_break_appended_ptr + 6000000h) + 6000000h
	bl	__crt0_move_memory

	// patch multiboot header
 	ldr	r0, =__boot_method
	mov	r1, #3
	str	r1, [r0]

	// remember that multiboot image came from GamePak
	ldr	r0, =__crt0_no_load_multiboot
	mov	r1, #0
	str	r1, [r0]

 	ldr	r0, =_start
	bx	r0			// restart

    END(__crt0_load_multiboot)

    OBJECT(__crt0_no_load_multiboot)
	.word -1
    END(__crt0_no_load_multiboot)

   .Lno_load_multiboot:
#endif

#if INIT_INTR_VECT
/* Initialize Interrupt Vector:
 * Set the interrupt vector.  Depending on how this file is compiled, the
 * function __interrupt_handler is expected to either be defined by the user
 * or by crt0.
 *****************************************************************************/
	.equ __interrupt_vector, 0x03007FFC

    STATIC_THUMB_FUNCTION(__crt0_init_intr_vect)
	ldr     r1, =__interrupt_vector
	ldr     r0, =__interrupt_handler
	str     r0, [r1]
    END(__crt0_init_intr_vect)
#endif

#if FIXUP_APPENDED_SECTION
/* Fix-Up Appended Assets:
 * Most of the time, appended assets are accessed where they are loaded.
 * However, In some cases it is convenient to move them.  The LMA and VMA of
 * of appended assets are passed from the footer (where an appender program may
 * have updated them) into __crt0_move_memory, which then decides what to do.
 * Assets are moved either because they overlap with the .bss section or to
 * free up space that has been allocated in the image for the appender program's
 * private use, but is not useful at runtime.  See APPENDED.TXT for more info.
 *****************************************************************************/
    STATIC_THUMB_FUNCTION(__crt0_fixup_appended)

	ldr	r0, =__appended_start

	ldr	r1, =__load_start_appended_ptr
	ldr	r1, [r1]

	ldr	r2, =__load_break_appended_ptr
	ldr	r2, [r2]

	// r0:dst       = where user program expects appended assets to be
	// r1:src_start = where appender program says appended assets start
	// r2:src_end   = where appender program says appended assets end
	bl	__crt0_move_memory

    END(__crt0_fixup_appended)
#endif

/* Initialize Standard Sections:
 * Variables declared and defined in the normal way are initialized here.
 *****************************************************************************/
#if INIT_STD_SECTIONS
    STATIC_THUMB_FUNCTION(__crt0_init_std_sections)

        // load initial values of variables like 'int foo = 42'
	ldr	r0, =__data_start
	ldr	r1, =__load_start_data
	ldr	r2, =__load_stop_data
	bl	__crt0_move_memory

	// clear memory for uninitialized variables like 'int bar'
	ldr	r0, =__bss_start
	ldr	r1, =__bss_end
	bl	__crt0_zero_memory

    END(__crt0_init_std_sections)
#endif

/* Initialize Special Game Boy Advance Sections:
 * Variables that were put into special RAM sections are initialized here.
 *****************************************************************************/
#if INIT_AGB_SECTIONS
    STATIC_THUMB_FUNCTION(__crt0_init_agb_sections)

 	// load variables and code into .ewram and .ewram0 sections
	ldr	r0, =__ewram_start
	ldr	r1, =__load_start_ewram
	ldr	r2, =__load_stop_ewram0
	bl	__crt0_move_memory

 	// load variables and code into .iwram and .iwram0 sections
	ldr	r0, =__iwram_start
	ldr	r1, =__load_start_iwram
	ldr	r2, =__load_stop_iwram0
	bl	__crt0_move_memory

    END(__crt0_init_agb_sections)
#endif

/* Branch to User Code in AgbMain:
 * Make a long-call to AgbMain.  If it returns, then restart.
 *****************************************************************************/
#ifdef ARM_NO_INTERWORK
    // If crt0.o is -marm -mno-thumb-interwork, then AgbMain should be the
    // same.  Expect AgbMain to return without exchanging intruction sets
    STATIC_THUMB_FUNCTION(__crt0_call_arm_AgbMain)
	adr	r0, .Larm
	bx	r0
	.arm
   .Larm:
	ldr	r0, =AgbMain
	bl	__crt0_call_via_r0
	ldr	r0, =__crt0_reset
    STATIC_FUNCTION(__crt0_call_via_r0)
	bx	r0
    END(__crt0_call_arm_AgbMain)
    END(__crt0_call_via_r0)
#else
    // Expect AgbMain to have interworking enabled or to be an ARM function,
    // therefore, AgbMain can return directly to _start
    STATIC_THUMB_FUNCTION(__crt0_call_AgbMain)
	ldr	r0, =AgbMain
	ldr	r1, =__crt0_reset
	mov	lr, r1
	bx	r0
    END(__crt0_call_AgbMain)
#endif

/* Reset:
 *****************************************************************************/
	.equ __soft_reset_direct, 0x03007FFA

    THUMB_FUNCTION(__crt0_reset)
	// disable interrupts
	ldr	r0, =0x04000208
 	mov	r1, #0
	strb	r1, [r0]

	// reset stack
	ldr	r0, =0x03007F00
	mov	sp, r0

#if FIXUP_APPENDED_SECTION
	ldr	r0, =__load_start_appended_ptr
	ldr	r0, [r0]

	ldr	r1, =__appended_start

	ldr	r2, =__load_break_appended_ptr
	ldr	r2, [r2]
	sub	r2, r2, r0
	add	r2, r1, r2

	bl	__crt0_move_memory
#endif

#if LOAD_MULTIBOOT
	ldr	r0, =__crt0_no_load_multiboot
	ldr	r0, [r0]
	cmp	r0, #0
	beq	.Lreset
#endif

	ldr	r0, =_start
	ldr	r1, =__memmap_rom_start
	sub	r0, r0, r1
	lsr	r0, r0, #24

   .Lreset:

	// soft reset (swi 0) parameter: where is _start
	ldr	r1, =__soft_reset_direct
	strb	r0, [r1]

	mov	r0, #254	// clear all but EWRAM

	swi	1
	swi	0

    END(__crt0_reset)



/* Literal Values Used By CRT Initialization:
 *****************************************************************************/
	.align
    STATIC_OBJECT(__crt0_literals)
	.pool
    END(__crt0_literals)



/*****************************************************************************\
                             CRT0 Library Functions
\*****************************************************************************/

/* __interrupt_handler:
 *****************************************************************************/
#if INIT_INTR_VECT
    .align
    FUNCTION(__interrupt_handler)
    adr    r0,1f + 1                
    bx     r0

   .thumb                          
1:
    push   {lr}
    bl     _gba_interrupt /* assume we are calling into THUMB */

	pop    {r0}
	bx     r0
	END(__interrupt_handler)
#endif

/* __crt0_zero_memory and __crt0_set_memory:
 *****************************************************************************/
#if SET_MEMORY
    THUMB_FUNCTION(__crt0_zero_memory)
	mov	r2, #0
	nop

    THUMB_FUNCTION(__crt0_set_memory)

	cmp	r0, r1
	bcs	.Lset_memory_return

    .Lset_memory_loop:
	stmia	r0!, {r2}
	cmp	r0, r1
	bcc	.Lset_memory_loop

    .Lset_memory_return:
	bx	lr

    END(__crt0_zero_memory)
    END(__crt0_set_memory)
#endif

/* __crt0_move_memory:
 *****************************************************************************/
#if MOVE_MEMORY
    THUMB_FUNCTION(__crt0_move_memory)

	cmp     r0, r1
	bcc     .Lforward_move	// if dst < src then forward copy
	bhi	.Lreverse_move	// if dst > src then reverse copy
	bx	lr		// else dst == src, nothing to do

   .Lforward_move:

	cmp     r1, r2
	bcs     .Lmove_memory_return

   .Lforward_move_loop:
	ldmia   r1!, {r3}
	stmia   r0!, {r3}
	cmp     r1, r2
	bcc     .Lforward_move_loop

	bx	lr

   .Lreverse_move:
	cmp     r2, r1
	bls     .Lmove_memory_return

        sub     r3, r2, r1
        add     r0, r0, r3

   .Lreverse_move_loop:
        sub     r2, r2, #4
        ldr     r3, [r2]
        sub     r0, r0, #4
        str     r3, [r0]
        cmp     r2, r1
        bhi     .Lreverse_move_loop

   .Lmove_memory_return:
	bx	lr

    END(__crt0_move_memory)
#endif

/* __FarFunction and __FarProcedure:
 *****************************************************************************/
#if FAR_FUNCTION
   THUMB_FUNCTION(__FarFunction)
   THUMB_FUNCTION(__FarProcedure)
        bx      r0
	nop
        nop
   END(__FarFunction)
   END(__FarProcedure)
#endif



/*****************************************************************************\
                            Interrupt Handler
\*****************************************************************************/

#if ISR_IN_CRT0
#include "isr.S"
#endif



/*****************************************************************************\
                                Miscellaneous
\*****************************************************************************/

/* Default Values for USR and IRQ stack pointers:
 *****************************************************************************/
#if WEAK_STACK_POINTERS
.section .cartrom.data, "a", %progbits

    WEAK_OBJECT(__sp_irq_initial_value)
        .word 0x03008000 - 0x60
    END(__sp_irq_initial_value)

    WEAK_OBJECT(__sp_usr_initial_value)
        .word 0x03008000 - 0x100
    END(__sp_usr_initial_value)
#endif

/* Weak Interrupt Handler:
 * This allows the user to override the definition of the interrupt handler
 * provided by crt0.  If no definition for __interrupt_handler is provided
 * anywhere, instead of saying '__interrupt_handler undefined', the linker will
 * automatically defined it as 0.
 *****************************************************************************/
#if WEAK_INTR_HANDLER
   .weak __interrupt_handler
   .weak _gba_interrupt
#endif

/* Footer For Use By External Tools:
 *****************************************************************************/
#if FOOTER
   .section .footer, "a", %progbits

    OBJECT(__load_break_appended_ptr)
	.word	__load_stop_appended	// pointer to end of binary image
    END(__load_break_appended_ptr)

    OBJECT(__load_start_appended_ptr)
	.word	__load_start_appended	// load address of appended assets
    END(__load_start_appended_ptr)

    OBJECT(__load_limit_appended_ptr)
	.word	__load_stop_appended	// max load address for appended assets
    END(__load_limit_appended_ptr)
#endif


/* __interrupt_check_flag = 0x03007FF8; */
/* __sound_buffer_ptr     = 0x03007FF0; */
/* __sp_svc = 0x03007FE0; */


