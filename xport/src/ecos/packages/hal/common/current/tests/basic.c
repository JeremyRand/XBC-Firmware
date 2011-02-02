/*=================================================================
//
//        intr.c
//
//        HAL Interrupt API test
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     nickg
// Contributors:  nickg
// Date:          1998-10-08
//####DESCRIPTIONEND####
*/

#include <pkgconf/hal.h>

#include <cyg/infra/testcase.h>

#include <cyg/hal/hal_intr.h>
#include <cyg/hal/drv_api.h>

// Include HAL/Platform specifics
#include CYGBLD_HAL_PLATFORM_H

#ifdef CYGPKG_KERNEL
#include <pkgconf/kernel.h>             // Need to look for the RTC config.
#endif

// Fallback defaults (in case HAL didn't define these)
#ifndef CYGNUM_HAL_RTC_NUMERATOR       
#define CYGNUM_HAL_RTC_NUMERATOR     1000000000
#define CYGNUM_HAL_RTC_DENOMINATOR   100
#define CYGNUM_HAL_RTC_PERIOD        9999
#endif

// -------------------------------------------------------------------------

#define ISR_DATA        0xAAAA1234

// -------------------------------------------------------------------------

volatile cyg_count32 ticks = 0;
static cyg_interrupt intr;
static cyg_handle_t  intr_handle;

// -------------------------------------------------------------------------

cyg_uint32 isr( cyg_uint32 vector, CYG_ADDRWORD data )
{
    CYG_TEST_CHECK( ISR_DATA == data , "Bad data passed to ISR");
    CYG_TEST_CHECK( CYGNUM_HAL_INTERRUPT_RTC == vector ,
                    "Bad vector passed to ISR");

    HAL_CLOCK_RESET( vector, CYGNUM_HAL_RTC_PERIOD );

    HAL_INTERRUPT_ACKNOWLEDGE( vector );
    
    ticks++;

    return CYG_ISR_HANDLED;
}

// -------------------------------------------------------------------------


void intr_main( void )
{
    CYG_INTERRUPT_STATE oldints;

#if 1
	cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_RTC, 1,
                             ISR_DATA, isr, NULL, &intr_handle, &intr);
    cyg_drv_interrupt_attach(intr_handle);
    HAL_CLOCK_INITIALIZE( CYGNUM_HAL_RTC_PERIOD );
    cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_RTC);

    HAL_ENABLE_INTERRUPTS();
#endif

    while(ticks < 500000000 )
    {
        diag_printf("ticks %d\n", ticks);
    }

    HAL_DISABLE_INTERRUPTS(oldints);

    CYG_TEST_PASS_FINISH("HAL interrupt test");
}


// -------------------------------------------------------------------------

externC void
cyg_start( void )
{
    CYG_TEST_INIT();

    intr_main();


    CYG_TEST_NA("Cannot override kernel real-time clock.");
}


// -------------------------------------------------------------------------
// EOF intr.c
