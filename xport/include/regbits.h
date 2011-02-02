#ifndef REGBITS_H
#define REGBITS_H

#include "gba.h"

// This is designed as replacement for regstructs.h given that there
// is a problem with using #pragma pack(1).  It uses the same
// nomenclature as regstructs.h to make it easy to convert.  
//
// For setting, where you would use something like:
//   g_DisplayControl->drawSprites=1;
// when using regstructs.h, you would now use
//   BFSET(GBA_REG_DISPCNT, drawSprites, 1)
//
// For getting, where you would use something like:
//   bool inVblank = g_DisplayStatus->isVBlank;
// you would replace this with 
//   bool inVblank = BFGET(GBA_REG_DISPSTAT, isVBlank);
//
// The structure of the field definitions is:   
//   #define GBA_REG_field_SHIFT	shift
//   #define GBA_REG_field_SIZE	        size
//
// where GBA_REG is the register name from gba.h, field is the field
// name from regstructs.h, shift is the number of bits to shift a
// value to put it in the right position for the field, and size is
// the bit witdth of the field.

//////////////////////////////////////////////////////////////////////
// Display Status Register

#define GBA_REG_DISPSTAT_isVBlank_SHIFT		0
#define GBA_REG_DISPSTAT_isVBlank_SIZE			1
#define GBA_REG_DISPSTAT_isHBlank_SHIFT		1
#define GBA_REG_DISPSTAT_isHBlank_SIZE			1
#define GBA_REG_DISPSTAT_triggerStatus_SHIFT	2
#define GBA_REG_DISPSTAT_triggerStatus_SIZE		1
#define GBA_REG_DISPSTAT_enableVBInt_SHIFT	3
#define GBA_REG_DISPSTAT_enableVBInt_SIZE		1
#define GBA_REG_DISPSTAT_enableHBInt_SHIFT	4
#define GBA_REG_DISPSTAT_enableHBInt_SIZE		1
#define GBA_REG_DISPSTAT_enableTriggerInt_SHIFT	5
#define GBA_REG_DISPSTAT_enableTriggerInt_SIZE		1
#define GBA_REG_DISPSTAT_filler_SHIFT		6
#define GBA_REG_DISPSTAT_filler_SIZE			2
#define GBA_REG_DISPSTAT_vcountTrigger_SHIFT	8
#define GBA_REG_DISPSTAT_vcountTrigger_SIZE		8

//////////////////////////////////////////////////////////////////////
// Display Control Register

#define GBA_REG_DISPCNT_videoMode_SHIFT		0
#define GBA_REG_DISPCNT_videoMode_SIZE			3
#define GBA_REG_DISPCNT_isGameboy_SHIFT		3
#define GBA_REG_DISPCNT_isGameboy_SIZE			1
#define GBA_REG_DISPCNT_doDoubleBuffer_SHIFT	4
#define GBA_REG_DISPCNT_doDoubleBuffer_SIZE		1
#define GBA_REG_DISPCNT_forceHBlank_SHIFT	5
#define GBA_REG_DISPCNT_forceHBlank_SIZE		1
#define GBA_REG_DISPCNT_objectMapping_SHIFT	6
#define GBA_REG_DISPCNT_objectMapping_SIZE		1
#define GBA_REG_DISPCNT_forceScreenBlank_SHIFT	7
#define GBA_REG_DISPCNT_forceScreenBlank_SIZE		1
#define GBA_REG_DISPCNT_drawBG0_SHIFT		8
#define GBA_REG_DISPCNT_drawBG0_SIZE			1
#define GBA_REG_DISPCNT_drawBG1_SHIFT		9
#define GBA_REG_DISPCNT_drawBG1_SIZE			1
#define GBA_REG_DISPCNT_drawBG2_SHIFT		10
#define GBA_REG_DISPCNT_drawBG2_SIZE			1
#define GBA_REG_DISPCNT_drawBG3_SHIFT		11
#define GBA_REG_DISPCNT_drawBG3_SIZE			1
#define GBA_REG_DISPCNT_drawSprites_SHIFT	12
#define GBA_REG_DISPCNT_drawSprites_SIZE		1
#define GBA_REG_DISPCNT_drawWindow0_SHIFT	13
#define GBA_REG_DISPCNT_drawWindow0_SIZE		1
#define GBA_REG_DISPCNT_drawWindow1_SHIFT	14
#define GBA_REG_DISPCNT_drawWindow1_SIZE		1
#define GBA_REG_DISPCNT_drawWindow2_SHIFT	15
#define GBA_REG_DISPCNT_drawWindow2_SIZE		1

//////////////////////////////////////////////////////////////////////
// Background Control Registers

#define GBA_REG_BG0CNT_priority_SHIFT		0
#define GBA_REG_BG0CNT_priority_SIZE			2
#define GBA_REG_BG0CNT_baseTileBlock_SHIFT	2
#define GBA_REG_BG0CNT_baseTileBlock_SIZE		2
#define GBA_REG_BG0CNT_filler_SHIFT		4
#define GBA_REG_BG0CNT_filler_SIZE			2
#define GBA_REG_BG0CNT_mosaic_SHIFT		6
#define GBA_REG_BG0CNT_mosaic_SIZE			1
#define GBA_REG_BG0CNT_palette_SHIFT		7
#define GBA_REG_BG0CNT_palette_SIZE			1
#define GBA_REG_BG0CNT_baseMapBlock_SHIFT	8
#define GBA_REG_BG0CNT_baseMapBlock_SIZE		5
#define GBA_REG_BG0CNT_wrapFlag_SHIFT		13
#define GBA_REG_BG0CNT_wrapFlag_SIZE			1
#define GBA_REG_BG0CNT_backgroundSize_SHIFT	14
#define GBA_REG_BG0CNT_backgroundSize_SIZE		2

#define GBA_REG_BG1CNT_priority_SHIFT		0
#define GBA_REG_BG1CNT_priority_SIZE			2
#define GBA_REG_BG1CNT_baseTileBlock_SHIFT	2
#define GBA_REG_BG1CNT_baseTileBlock_SIZE		2
#define GBA_REG_BG1CNT_filler_SHIFT		4
#define GBA_REG_BG1CNT_filler_SIZE			2
#define GBA_REG_BG1CNT_mosaic_SHIFT		6
#define GBA_REG_BG1CNT_mosaic_SIZE			1
#define GBA_REG_BG1CNT_palette_SHIFT		7
#define GBA_REG_BG1CNT_palette_SIZE			1
#define GBA_REG_BG1CNT_baseMapBlock_SHIFT	8
#define GBA_REG_BG1CNT_baseMapBlock_SIZE		5
#define GBA_REG_BG1CNT_wrapFlag_SHIFT		13
#define GBA_REG_BG1CNT_wrapFlag_SIZE			1
#define GBA_REG_BG1CNT_backgroundSize_SHIFT	14
#define GBA_REG_BG1CNT_backgroundSize_SIZE		2

#define GBA_REG_BG2CNT_priority_SHIFT		0
#define GBA_REG_BG2CNT_priority_SIZE			2
#define GBA_REG_BG2CNT_baseTileBlock_SHIFT	2
#define GBA_REG_BG2CNT_baseTileBlock_SIZE		2
#define GBA_REG_BG2CNT_filler_SHIFT		4
#define GBA_REG_BG2CNT_filler_SIZE			2
#define GBA_REG_BG2CNT_mosaic_SHIFT		6
#define GBA_REG_BG2CNT_mosaic_SIZE			1
#define GBA_REG_BG2CNT_palette_SHIFT		7
#define GBA_REG_BG2CNT_palette_SIZE			1
#define GBA_REG_BG2CNT_baseMapBlock_SHIFT	8
#define GBA_REG_BG2CNT_baseMapBlock_SIZE		5
#define GBA_REG_BG2CNT_wrapFlag_SHIFT		13
#define GBA_REG_BG2CNT_wrapFlag_SIZE			1
#define GBA_REG_BG2CNT_backgroundSize_SHIFT	14
#define GBA_REG_BG2CNT_backgroundSize_SIZE		2

#define GBA_REG_BG3CNT_priority_SHIFT		0
#define GBA_REG_BG3CNT_priority_SIZE			2
#define GBA_REG_BG3CNT_baseTileBlock_SHIFT	2
#define GBA_REG_BG3CNT_baseTileBlock_SIZE		2
#define GBA_REG_BG3CNT_filler_SHIFT		4
#define GBA_REG_BG3CNT_filler_SIZE			2
#define GBA_REG_BG3CNT_mosaic_SHIFT		6
#define GBA_REG_BG3CNT_mosaic_SIZE			1
#define GBA_REG_BG3CNT_palette_SHIFT		7
#define GBA_REG_BG3CNT_palette_SIZE			1
#define GBA_REG_BG3CNT_baseMapBlock_SHIFT	8
#define GBA_REG_BG3CNT_baseMapBlock_SIZE		5
#define GBA_REG_BG3CNT_wrapFlag_SHIFT		13
#define GBA_REG_BG3CNT_wrapFlag_SIZE			1
#define GBA_REG_BG3CNT_backgroundSize_SHIFT	14
#define GBA_REG_BG3CNT_backgroundSize_SIZE		2

//////////////////////////////////////////////////////////////////////
// Timer Control Registers

#define GBA_REG_TM0CNT_clockSpan_SHIFT			0
#define GBA_REG_TM0CNT_clockSpan_SIZE			2

#define GBA_REG_TM0CNT_incOnOverflow_SHIFT		2
#define GBA_REG_TM0CNT_incOnOverflow_SIZE		1
// Skip 3 unused bits
#define GBA_REG_TM0CNT_intOnOverflow_SHIFT		6
#define GBA_REG_TM0CNT_intOnOverflow_SIZE		1

#define GBA_REG_TM0CNT_enableTimer_SHIFT		7
#define GBA_REG_TM0CNT_enableTimer_SIZE			1
// Leave off 8 unused bits at end

#define GBA_REG_TM1CNT_clockSpan_SHIFT			0
#define GBA_REG_TM1CNT_clockSpan_SIZE			2
#define GBA_REG_TM1CNT_incOnOverflow_SHIFT		2
#define GBA_REG_TM1CNT_incOnOverflow_SIZE		1
#define GBA_REG_TM1CNT_intOnOverflow_SHIFT		6
#define GBA_REG_TM1CNT_intOnOverflow_SIZE		1
#define GBA_REG_TM1CNT_enableTimer_SHIFT		7
#define GBA_REG_TM1CNT_enableTimer_SIZE			1

#define GBA_REG_TM2CNT_clockSpan_SHIFT			0
#define GBA_REG_TM2CNT_clockSpan_SIZE			2
#define GBA_REG_TM2CNT_incOnOverflow_SHIFT		2
#define GBA_REG_TM2CNT_incOnOverflow_SIZE		1
#define GBA_REG_TM2CNT_intOnOverflow_SHIFT		6
#define GBA_REG_TM2CNT_intOnOverflow_SIZE		1
#define GBA_REG_TM2CNT_enableTimer_SHIFT		7
#define GBA_REG_TM2CNT_enableTimer_SIZE			1

#define GBA_REG_TM3CNT_clockSpan_SHIFT			0
#define GBA_REG_TM3CNT_clockSpan_SIZE			2
#define GBA_REG_TM3CNT_incOnOverflow_SHIFT		2
#define GBA_REG_TM3CNT_incOnOverflow_SIZE		1
#define GBA_REG_TM3CNT_intOnOverflow_SHIFT		6
#define GBA_REG_TM3CNT_intOnOverflow_SIZE		1
#define GBA_REG_TM3CNT_enableTimer_SHIFT		7
#define GBA_REG_TM3CNT_enableTimer_SIZE			1


//////////////////////////////////////////////////////////////////////
// Macros
#define BFMASK(shift,size) (((1<<(size))-1)<<(shift))
#define BFSET_HELPER(reg,shift,size,value) ((reg)= ((reg) & ~BFMASK(shift,size)) | (((value)<<(shift)) & BFMASK(shift,size)))
#define BFGET_HELPER(reg,shift,size) (((reg)>>(shift)) & BFMASK(0,size))

#define BFSET(reg,field,value) BFSET_HELPER(reg, reg##_##field##_SHIFT, reg##_##field##_SIZE, value)
#define BFGET(reg,field) BFGET_HELPER(reg, reg##_##field##_SHIFT, reg##_##field##_SIZE)


#endif
