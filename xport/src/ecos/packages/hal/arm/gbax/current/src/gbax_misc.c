//==========================================================================
//
//      gbax_misc.c
//
//      HAL misc board support code for Game Boy Advance
//
//==========================================================================
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
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, rich@charmedlabs.com
// Date:         2002-09-02
// Purpose:      HAL board support
// Description:  Implementations of HAL board interfaces
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_arch.h>           // Register state info
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/hal_intr.h>           // necessary?
#include <cyg/hal/hal_if.h>             // calling interface
#include <cyg/hal/gba.h>
#include <cyg/hal/xport.h>
#include <cyg/hal/text_disp.h>

#if 0
static char hexchars[16] = "0123456789abcdef";

#define hexdigit(r) temp = num/r; buf[i++] = hexchars[temp]; num -= r * temp;

void printhexnum(unsigned long num)
	{
	unsigned long temp, i = 0;
	char buf[16];

	hexdigit(0x10000000);
	hexdigit(0x1000000);
	hexdigit(0x100000);
	hexdigit(0x10000);
	hexdigit(0x1000);
	hexdigit(0x100);
	hexdigit(0x10);
	hexdigit(0x1);
	buf[i++] = 10;
	buf[i++] = 0;
	TD_Print(buf);
	}
#endif

void hal_clock_initialize(cyg_uint32 period)
{
	GBA_REG_TM0D = period>0xffff ? 0 : 0xffff-period;
	GBA_REG_TM0CNT = 0x00c3;
}

void hal_clock_reset(cyg_uint32 vector, cyg_uint32 period)
{
}

void hal_clock_read(cyg_uint32 *pvalue)
{
	*pvalue = 0;
}

// -------------------------------------------------------------------------
//
// Delay for some number of micro-seconds
//
void hal_delay_us(cyg_int32 usecs)
{
    cyg_uint32 outer, ticks = usecs/10 + 1;
	volatile cyg_uint32 inner;

	for (outer=0; outer<ticks; outer++)
		for (inner=0; inner<4; inner++);
}

// -------------------------------------------------------------------------

void hal_patch_vector_code(void);

void hal_hardware_init(void)
{
#ifndef CYGHWR_GBAX_VERSION_1_1 
	volatile unsigned long *psrc, *pdest;
	int i;
#endif

	// clear flags, set master interrupt enable
	GBA_REG_IF = 0xffff;
	GBA_REG_IE = 0;
	GBA_REG_IME = 1;

    // Set up eCos/ROM interfaces
    hal_if_init();
	hal_diag_register_mangler();

#if !defined(CYGHWR_GBAX_VERSION_1_1) && defined(CYG_HAL_STARTUP_ROM) 
	// copy patch vector code for illegal instruction into RAM
	pdest = (volatile unsigned long *)XP_BRAM_START;
	psrc = (volatile unsigned long *)hal_patch_vector_code;
	for (i=0; i<128; i++)
		pdest[i] = psrc[i];
#if 0
	for (i=0; i<128; i++)
		if (pdest[i] != psrc[i])
			TD_Print("Error: RAM not present.");
#endif
#endif
}

//
// This routine is called to respond to a hardware interrupt (IRQ).  It
// should interrogate the hardware and return the IRQ vector number.

int hal_IRQ_handler(void)
{
    int vector;
	cyg_uint16 irq_status;

	irq_status = GBA_REG_IF;
    for (vector=CYGNUM_HAL_ISR_MIN; vector<=CYGNUM_HAL_ISR_MAX; vector++) 
		{
        if (irq_status & (1<<vector)) 
			return vector;
    }
    return CYGNUM_HAL_INTERRUPT_NONE; // This shouldn't happen!
}

//
// Interrupt control
//

void hal_interrupt_mask(int vector)
{
	GBA_REG_IE &= ~(1<<vector);
}

void hal_interrupt_unmask(int vector)
{
	GBA_REG_IE |= 1<<vector;
}

void hal_interrupt_acknowledge(int vector)
{
	GBA_REG_IF = 1<<vector;	
}

void hal_interrupt_configure(int vector, int level, int up)
{
    //diag_init();  diag_printf("%s(%d,%d,%d)\n", __PRETTY_FUNCTION__, vector, level, up);
}

void hal_interrupt_set_level(int vector, int level)
{
    //diag_init();  diag_printf("%s(%d,%d)\n", __PRETTY_FUNCTION__, vector, level);
}

void hal_show_IRQ(int vector, int data, int handler)
{
    //    diag_printf("IRQ - vector: %x, data: %x, handler: %x\n", vector, data, handler);
}
 
/*---------------------------------------------------------------------------*/
/* End of hal_misc.c */
