//===========================================================================
//
// MLT linker script for MN10300
// adapted from packages\hal\mn10300\stdeval1\v1_1\src\stdeval1.ld
//
//===========================================================================
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
//===========================================================================

#include <pkgconf/system.h>
STARTUP(vectors.o)
ENTRY(reset_vector)
#ifdef EXTRAS
INPUT(extras.o)
#endif
GROUP(libtarget.a libgcc.a)


#define ALIGN_LMA 4
#define FOLLOWING(_section_) AT ((LOADADDR (_section_) + SIZEOF (_section_) + ALIGN_LMA - 1) & ~ (ALIGN_LMA - 1))
#define LMA_EQ_VMA
#define FORCE_OUTPUT . = .

#define SECTIONS_BEGIN

#define SECTION_rom_vectors(_region_, _vma_, _lma_) \
    .rom_vectors _vma_ : _lma_ \
    { FORCE_OUTPUT; KEEP (*(.vectors)) } \
    > _region_

#define SECTION_text(_region_, _vma_, _lma_) \
    .text _vma_ : _lma_ \
    { __stext = ABSOLUTE(.); \
    *(.text*) *(.gnu.warning) *(.gnu.linkonce*) *(.init) } \
    > _region_ \
    __etext = .; PROVIDE (etext = .);

#define SECTION_fini(_region_, _vma_, _lma_) \
    .fini _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fini) } \
    > _region_

#define SECTION_rodata(_region_, _vma_, _lma_) \
    .rodata _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata*) } \
    > _region_

#define SECTION_rodata1(_region_, _vma_, _lma_) \
    .rodata1 _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.rodata1) } \
    > _region_

#define SECTION_fixup(_region_, _vma_, _lma_) \
    .fixup _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.fixup) } \
    > _region_

#define SECTION_gcc_except_table(_region_, _vma_, _lma_) \
    .gcc_except_table _vma_ : _lma_ \
    { FORCE_OUTPUT; *(.gcc_except_table) } \
    > _region_

#define SECTION_data(_region_, _vma_, _lma_) \
    .data _vma_ : _lma_ \
    { __ram_data_start = ABSOLUTE (.); *(.data*) *(.data1) \
    . = ALIGN(4); \
    __s2ram = ABSOLUTE(.); \
      *(.2ram*) \
    __e2ram = ABSOLUTE(.); \
    . = ALIGN(4); \
    _GOT1_START_ = ABSOLUTE (.); *(.got1) _GOT1_END_ = ABSOLUTE (.); \
    _GOT2_START_ = ABSOLUTE (.); *(.got2) _GOT2_END_ = ABSOLUTE (.); \
    . = ALIGN (4); \
    KEEP(*( SORT (.ecos.table.*))) ; \
    . = ALIGN(4); \
    __CTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.ctors*))) __CTOR_END__ = ABSOLUTE (.); \
    __DTOR_LIST__ = ABSOLUTE (.); KEEP (*(SORT (.dtors*))) __DTOR_END__ = ABSOLUTE (.); \
    _GOT_START = ABSOLUTE (.); _GLOBAL_OFFSET_TABLE_ = ABSOLUTE (.) + 32768; _SDA_BASE_ = ABSOLUTE (.); \
    *(.got.plt) *(.got) _GOT_END_ = ABSOLUTE (.); \
    *(.dynamic) *(.sdata*) *(.sbss*) } \
    > _region_ \
    __rom_data_start = LOADADDR (.data); \
    __ram_data_end = .; PROVIDE (__ram_data_end = .); _edata = .; PROVIDE (edata = .);

#define SECTION_bss(_region_, _vma_, _lma_) \
    .bss _vma_ : _lma_ \
    { __bss_start = ABSOLUTE (.); \
    *(.scommon) *(.dynbss) *(.bss) *(COMMON) \
    __bss_end = ABSOLUTE (.); } \
    > _region_

#define SECTIONS_END . = ALIGN(4); _end = .; PROVIDE (end = .);

#include CYGHWR_MEMORY_LAYOUT_LDI

_mn10300_interrupt_control = 0xD4000000;
_mn10300_interrupt_vectors = 0xC0000000;

#include CYGBLD_HAL_TARGET_H

#if CYGHWR_HAL_MN10300_AM33_REVISION == 1

#ifndef CYG_HAL_PLATFORM_HAL_VSR_TABLE_DEFINED
_hal_vsr_table = 0x50000100;
_hal_virtual_vector_table = 0x50000200;
#endif

#elif CYGHWR_HAL_MN10300_AM33_REVISION == 2

#ifndef CYG_HAL_PLATFORM_HAL_VSR_TABLE_DEFINED
_hal_vsr_table = 0x90000000;
_hal_virtual_vector_table = 0x90000100;
#endif

#else

#error Unknown AM33 Revision

#endif
