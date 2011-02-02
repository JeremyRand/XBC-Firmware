/*=============================================================================
//
//      hal_diag.c
//
//      HAL diagnostic output code
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
// Author(s):   nickg, gthomas
// Contributors:nickg, gthomas, rich@charmedlabs.com
// Date:        2002-09-02
// Purpose:     HAL diagnostic output
// Description: Implementations of HAL diagnostic output support.
//
//####DESCRIPTIONEND####
//
//===========================================================================*/

#include <pkgconf/hal.h>
#include <pkgconf/hal_arm_gbax.h>        // board specifics

#include <cyg/infra/cyg_type.h>         // base types
#include <cyg/infra/cyg_trac.h>         // tracing macros
#include <cyg/infra/cyg_ass.h>          // assertion macros

#include <cyg/hal/hal_arch.h>           // basic machine info
#include <cyg/hal/hal_intr.h>           // interrupt macros
#include <cyg/hal/hal_io.h>             // IO macros
#include <cyg/hal/hal_diag.h>
#include <cyg/hal/drv_api.h>
#include <cyg/hal/hal_if.h>             // interface API
#include <cyg/hal/hal_misc.h>           // Helper functions
#include <cyg/hal/gba.h>
#include <cyg/hal/xport.h>
#include <cyg/hal/text_disp.h>

/*---------------------------------------------------------------------------*/
/* From serial_16550.h */
#ifdef CYGHWR_GBAX_VERSION_1_1
#if CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD==9600
#define CYG_DEVICE_SERIAL_BAUD_LSB        0x00
#endif
#if CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD==38400
#define CYG_DEVICE_SERIAL_BAUD_LSB        0x01
#endif
#if CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD==57600
#define CYG_DEVICE_SERIAL_BAUD_LSB        0x02
#endif
#if CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL_BAUD==115200
#define CYG_DEVICE_SERIAL_BAUD_LSB        0x03
#endif

#ifndef CYG_DEVICE_SERIAL_BAUD_LSB
#error Missing/incorrect serial baud rate defined - CDL error?
#endif
#endif

//-----------------------------------------------------------------------------
typedef struct {
    cyg_int32 msec_timeout;
    int isr_vector;
} channel_data_t;

//-----------------------------------------------------------------------------

static void
cyg_hal_plf_serial_init_channel(void* __ch_data)
{
#ifdef CYGHWR_GBAX_VERSION_1_1
	GBA_REG_R &= 0x7fff;
//	GBA_REG_SCCNT_L = 0x3d84 | CYG_DEVICE_SERIAL_BAUD_LSB;	// cts handshaking
	GBA_REG_SCCNT_L = 0x3d80 | CYG_DEVICE_SERIAL_BAUD_LSB;  // no cts handshaking
#endif
}

static channel_data_t gbax_ser_channels[2] = {
    {1000, 0},
};

void (*mangler_orig)(void* __ch_data, cyg_uint8 c) = NULL;

void 
mangler_putc(void* __ch_data, cyg_uint8 c)
{
	if (mangler_orig)
		(*mangler_orig)(__ch_data, c);
	TD_PrintChar(c);
}

void
cyg_hal_plf_serial_putc(void *__ch_data, char c)
{
	hal_virtual_comm_table_t *comm;

	CYGARC_HAL_SAVE_GP();

#ifdef CYGDBG_HAL_DEBUG_LCD_OUTPUT
	comm = CYGACC_CALL_IF_CONSOLE_PROCS();
	// filter out console output from GDB mangler output
	if ((channel_data_t *)*comm[0]==gbax_ser_channels)
		TD_PrintChar(c);
#endif
	
#ifdef CYGHWR_GBAX_VERSION_1_1
	while(GBA_REG_SCCNT_L&0x0010);
	GBA_REG_SCCNT_H = (cyg_uint16)c;
#else
	while(XP_REG_CPORT_STAT&0x0002);
	XP_REG_CPORT_DATA = (cyg_uint16)c;
#endif
    CYGARC_HAL_RESTORE_GP();
}

static cyg_bool
cyg_hal_plf_serial_getc_nonblock(void* __ch_data, cyg_uint8* ch)
{
#ifdef CYGHWR_GBAX_VERSION_1_1 
	if (GBA_REG_SCCNT_L&0x0020)
		return false;
	
	*ch = (cyg_uint8)GBA_REG_SCCNT_H;
#else
	if (XP_REG_CPORT_STAT&0x0001)
		return false;

	*ch = (cyg_uint8)XP_REG_CPORT_DATA;
#endif
	return true;
}

cyg_uint8
cyg_hal_plf_serial_getc(void* __ch_data)
{
    cyg_uint8 ch;
    CYGARC_HAL_SAVE_GP();

    while(!cyg_hal_plf_serial_getc_nonblock(__ch_data, &ch));

    CYGARC_HAL_RESTORE_GP();
    return ch;
}


static void
cyg_hal_plf_serial_write(void* __ch_data, const cyg_uint8* __buf, 
                         cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        cyg_hal_plf_serial_putc(__ch_data, *__buf++);

    CYGARC_HAL_RESTORE_GP();
}

static void
cyg_hal_plf_serial_read(void* __ch_data, cyg_uint8* __buf, cyg_uint32 __len)
{
    CYGARC_HAL_SAVE_GP();

    while(__len-- > 0)
        *__buf++ = cyg_hal_plf_serial_getc(__ch_data);

    CYGARC_HAL_RESTORE_GP();
}

cyg_bool
cyg_hal_plf_serial_getc_timeout(void* __ch_data, cyg_uint8* ch)
{
    int delay_count;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    cyg_bool res;
    CYGARC_HAL_SAVE_GP();
    
    delay_count = chan->msec_timeout * 10; // delay in .1 ms steps

    for(;;) {
        res = cyg_hal_plf_serial_getc_nonblock(__ch_data, ch);
        if (res || 0 == delay_count--)
            break;
        
        CYGACC_CALL_IF_DELAY_US(100);
    }

    CYGARC_HAL_RESTORE_GP();
    return res;
}

void foo(void)
	{
	}

static int
cyg_hal_plf_serial_control(void *__ch_data, __comm_control_cmd_t __func, ...)
{
    static int irq_state = 0;
    channel_data_t* chan = (channel_data_t*)__ch_data;
    int ret = 0;
    CYGARC_HAL_SAVE_GP();

    switch (__func) {
    case __COMMCTL_IRQ_ENABLE:
        irq_state = 1;
        break;
    case __COMMCTL_IRQ_DISABLE:
        ret = irq_state;
        irq_state = 0;
        break;
    case __COMMCTL_DBG_ISR_VECTOR:
        ret = chan->isr_vector;
        break;
    case __COMMCTL_SET_TIMEOUT:
    {
        va_list ap;

        va_start(ap, __func);

        ret = chan->msec_timeout;
        chan->msec_timeout = va_arg(ap, cyg_uint32);

        va_end(ap);
		break;
    }        
    default:
		foo();
        break;
    }
    CYGARC_HAL_RESTORE_GP();
    return ret;
}

static int
cyg_hal_plf_serial_isr(void *__ch_data, int* __ctrlc, 
                       CYG_ADDRWORD __vector, CYG_ADDRWORD __data)
{
	return 0;
}

static void
cyg_hal_plf_serial_init(void)
{
    hal_virtual_comm_table_t* comm;
    int cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);

    // Init channels
    cyg_hal_plf_serial_init_channel(&gbax_ser_channels[0]);

    // Setup procs in the vector table

    // Set channel 0
    CYGACC_CALL_IF_SET_CONSOLE_COMM(0);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &gbax_ser_channels[0]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

    // Set channel 1
    CYGACC_CALL_IF_SET_CONSOLE_COMM(1);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
    CYGACC_COMM_IF_CH_DATA_SET(*comm, &gbax_ser_channels[1]);
    CYGACC_COMM_IF_WRITE_SET(*comm, cyg_hal_plf_serial_write);
    CYGACC_COMM_IF_READ_SET(*comm, cyg_hal_plf_serial_read);
    CYGACC_COMM_IF_PUTC_SET(*comm, cyg_hal_plf_serial_putc);
    CYGACC_COMM_IF_GETC_SET(*comm, cyg_hal_plf_serial_getc);
    CYGACC_COMM_IF_CONTROL_SET(*comm, cyg_hal_plf_serial_control);
    CYGACC_COMM_IF_DBG_ISR_SET(*comm, cyg_hal_plf_serial_isr);
    CYGACC_COMM_IF_GETC_TIMEOUT_SET(*comm, cyg_hal_plf_serial_getc_timeout);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);
}

void 
hal_diag_register_mangler(void)
{
#ifdef CYGDBG_HAL_DEBUG_GDB_LCD_OUTPUT
    hal_virtual_comm_table_t* comm;
    int cur;
	
	cur = CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_QUERY_CURRENT);
    CYGACC_CALL_IF_SET_CONSOLE_COMM(CYGNUM_CALL_IF_SET_COMM_ID_MANGLER);
    comm = CYGACC_CALL_IF_CONSOLE_PROCS();
	mangler_orig = (void (*)(void* __ch_data, cyg_uint8 c))(*comm)[CYGNUM_COMM_IF_PUTC];
    CYGACC_COMM_IF_PUTC_SET(*comm, mangler_putc);

    // Restore original console
    CYGACC_CALL_IF_SET_CONSOLE_COMM(cur);

#endif
#if defined(CYGDBG_HAL_DEBUG_LCD_OUTPUT) || defined(CYGDBG_HAL_DEBUG_GDB_LCD_OUTPUT)
	TD_Init();
#endif
}

void
cyg_hal_plf_comms_init(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

    cyg_hal_plf_serial_init();
}

/*---------------------------------------------------------------------------*/


//=============================================================================
// Compatibility with older stubs
//=============================================================================

#ifndef CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
#include <cyg/hal/hal_stub.h>           // cyg_hal_gdb_interrupt
#endif


static channel_data_t gbax_ser_channel = {
    0, 0
};

// Assumption: all diagnostic output must be GDB packetized unless this is a ROM (i.e.
// totally stand-alone) system.

#if defined(CYG_HAL_STARTUP_ROM) || !defined(CYGDBG_HAL_DIAG_TO_DEBUG_CHAN)
#define HAL_DIAG_USES_HARDWARE
#endif

#ifndef HAL_DIAG_USES_HARDWARE
#if (CYGNUM_HAL_VIRTUAL_VECTOR_CONSOLE_CHANNEL != CYGNUM_HAL_VIRTUAL_VECTOR_DEBUG_CHANNEL)
#define HAL_DIAG_USES_HARDWARE
#endif
#endif

#ifdef HAL_DIAG_USES_HARDWARE

void hal_diag_init(void)
{
    static int init = 0;
    char *msg = "\n\rARM eCos\n\r";

    if (init++) return;

    cyg_hal_plf_serial_init_channel(&gbax_ser_channel);

    while (*msg) cyg_hal_plf_serial_putc(&gbax_ser_channel, *msg++);
}

#ifdef DEBUG_DIAG
#if defined(CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS)
#define DIAG_BUFSIZE 32
#else
#define DIAG_BUFSIZE 2048
#endif
static char diag_buffer[DIAG_BUFSIZE];
static int diag_bp = 0;
#endif

void hal_diag_write_char(char c)
{
    hal_diag_init();

    cyg_hal_plf_serial_putc(&gbax_ser_channel, c);

#ifdef DEBUG_DIAG
    diag_buffer[diag_bp++] = c;
    if (diag_bp == DIAG_BUFSIZE) diag_bp = 0;
#endif
}

void hal_diag_read_char(char *c)
{
    *c = cyg_hal_plf_serial_getc(&gbax_ser_channel);
}

#else // HAL_DIAG relies on GDB

// Initialize diag port - assume GDB channel is already set up
void hal_diag_init(void)
{
    if (0) cyg_hal_plf_serial_init_channel(&gbax_ser_channel); // avoid warning
}

// Actually send character down the wire
static void
hal_diag_write_char_serial(char c)
{
    hal_diag_init();

    cyg_hal_plf_serial_putc(&gbax_ser_channel, c);
}

static bool
hal_diag_read_serial(char *c)
{
    long timeout = 1000000000;  // A long time...
    while (!cyg_hal_plf_serial_getc_nonblock(&gbax_ser_channel, c))
        if (0 == --timeout) return false;

    return true;
}

void 
hal_diag_read_char(char *c)
{
    while (!hal_diag_read_serial(c)) ;
}

void 
hal_diag_write_char(char c)
{
    static char line[100];
    static int pos = 0;

    // No need to send CRs
    if( c == '\r' ) return;

    line[pos++] = c;

    if( c == '\n' || pos == sizeof(line) )
    {
        CYG_INTERRUPT_STATE old;

        // Disable interrupts. This prevents GDB trying to interrupt us
        // while we are in the middle of sending a packet. The serial
        // receive interrupt will be seen when we re-enable interrupts
        // later.
        
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_ENTER_CRITICAL_IO_REGION(old);
#else
        HAL_DISABLE_INTERRUPTS(old);
#endif

        while(1)
        {
            static char hex[] = "0123456789ABCDEF";
            cyg_uint8 csum = 0;
            int i;
            char c1;
        
            hal_diag_write_char_serial('$');
            hal_diag_write_char_serial('O');
            csum += 'O';
            for( i = 0; i < pos; i++ )
            {
                char ch = line[i];
                char h = hex[(ch>>4)&0xF];
                char l = hex[ch&0xF];
                hal_diag_write_char_serial(h);
                hal_diag_write_char_serial(l);
                csum += h;
                csum += l;
            }
            hal_diag_write_char_serial('#');
            hal_diag_write_char_serial(hex[(csum>>4)&0xF]);
            hal_diag_write_char_serial(hex[csum&0xF]);

            // Wait for the ACK character '+' from GDB here and handle
            // receiving a ^C instead.  This is the reason for this clause
            // being a loop.
            if (!hal_diag_read_serial(&c1))
                continue;   // No response - try sending packet again

            if( c1 == '+' )
                break;              // a good acknowledge

#ifdef CYGDBG_HAL_DEBUG_GDB_BREAK_SUPPORT
            cyg_drv_interrupt_acknowledge(CYG_DEVICE_SERIAL_INT);
            if( c1 == 3 ) {
                // Ctrl-C: breakpoint.
                cyg_hal_gdb_interrupt ((target_register_t)__builtin_return_address(0));
                break;
            }
#endif
            // otherwise, loop round again
        }
        
        pos = 0;

        // And re-enable interrupts
#ifdef CYGDBG_HAL_DEBUG_GDB_INCLUDE_STUBS
        CYG_HAL_GDB_LEAVE_CRITICAL_IO_REGION(old);
#else
        HAL_RESTORE_INTERRUPTS(old);
#endif
        
    }
}
#endif

#endif // CYGSEM_HAL_VIRTUAL_VECTOR_DIAG

/*---------------------------------------------------------------------------*/
/* End of hal_diag.c */
