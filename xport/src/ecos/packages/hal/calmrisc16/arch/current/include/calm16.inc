#ifndef CYGONCE_HAL_CALM16_INC
#define CYGONCE_HAL_CALM16_INC

##=============================================================================
##
##	calm16.inc
##
##	CalmRISC16 assembler header file
##
##=============================================================================
#####ECOSGPLCOPYRIGHTBEGIN####
## -------------------------------------------
## This file is part of eCos, the Embedded Configurable Operating System.
## Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
##
## eCos is free software; you can redistribute it and/or modify it under
## the terms of the GNU General Public License as published by the Free
## Software Foundation; either version 2 or (at your option) any later version.
##
## eCos is distributed in the hope that it will be useful, but WITHOUT ANY
## WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with eCos; if not, write to the Free Software Foundation, Inc.,
## 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
##
## As a special exception, if other files instantiate templates or use macros
## or inline functions from this file, or you compile this file and link it
## with other works to produce a work based on this file, this file does not
## by itself cause the resulting work to be covered by the GNU General Public
## License. However the source code for this file must still be made available
## in accordance with section (3) of the GNU General Public License.
##
## This exception does not invalidate any other reasons why a work based on
## this file might be covered by the GNU General Public License.
##
## Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
## at http://sources.redhat.com/ecos/ecos-license/
## -------------------------------------------
#####ECOSGPLCOPYRIGHTEND####
##=============================================================================
#######DESCRIPTIONBEGIN####
##
## Author(s): 	msalter
## Contributors: msalter
## Date:	2001-02-12
## Purpose:	CalmRISC16 definitions.
## Description:	This file contains various definitions and macros that are
##              useful for writing assembly code for the MIPS CPU family.
## Usage:
##		#include <cyg/hal/calm16.inc>
##		...
##		
##
######DESCRIPTIONEND####
##
##=============================================================================


#------------------------------------------------------------------------------

#ifdef __USER_LABEL_PREFIX__
#define __GLUE(a,b) a##b
#define _GLUE(a,b) __GLUE(a,b)
#define SYM_NAME(x) _GLUE(_,x)

.macro FUNC_START name
        .type _\name,@function
        .globl _\name
_\name:
.endm

.macro FUNC_END name
_\name\(_end):
.endm
#else
#define SYM_NAME(x) x
.macro FUNC_START name
        .type \name,@function
        .globl \name
\name:
.endm

.macro FUNC_END name
\name\(_end):
.endm
#endif


#------------------------------------------------------------------------------
#endif // ifndef CYGONCE_HAL_CALM16_INC
# end of calm16.inc
