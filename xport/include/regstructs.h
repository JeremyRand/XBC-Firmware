#ifndef REGSTRUCTS_H
#define REGSTRUCTS_H

#error "Do not use regstructs.h.  Use regbits.h instead!"
#include "gba.h"

#define PACKED __attribute__((packed))
#pragma pack(1)

struct InputStatusRegister
{
	volatile unsigned aButton : 1;
	volatile unsigned bButton : 1;
	volatile unsigned selectButton : 1;
	volatile unsigned startButton : 1;
	volatile unsigned rightButton : 1;
	volatile unsigned leftButton : 1;
	volatile unsigned upButton : 1;
	volatile unsigned downButton : 1;
	volatile unsigned rButton : 1;
	volatile unsigned lButton : 1;
	volatile unsigned filler: 6;
} PACKED;

struct InputControlRegister
{
	volatile unsigned aButtonInt : 1;
	volatile unsigned bButtonInt : 1;
	volatile unsigned selectButtonInt : 1;
	volatile unsigned startButtonInt : 1;
	volatile unsigned rightButtonInt : 1;
	volatile unsigned leftButtonInt : 1;
	volatile unsigned upButtonInt : 1;
	volatile unsigned downButtonInt : 1;
	volatile unsigned rButtonInt : 1;
	volatile unsigned lButtonInt : 1;
	volatile unsigned filler: 4;
	volatile unsigned enableKeyInt : 1;
	volatile unsigned intOperator : 1;
} PACKED;

struct TimerRegister
{
	volatile unsigned clockCount : 16;
	volatile unsigned clockSpan : 2;
	volatile unsigned incOnOverflow : 1;
	volatile unsigned fillSpace : 3;
	volatile unsigned intOnOverflow : 1;
	volatile unsigned enableTimer : 1;
	volatile unsigned fillSpace2 : 8;
} PACKED;

struct DisplayStatusRegister
{
	volatile unsigned isVBlank : 1;
	volatile unsigned isHBlank : 1;
	volatile unsigned triggerStatus : 1;
	volatile unsigned enableVBInt : 1;
	volatile unsigned enableHBInt : 1;
	volatile unsigned enableTriggerInt : 1;
	volatile unsigned filler : 2;
	volatile unsigned vcountTrigger : 8;
} PACKED;

struct DisplayControlRegister
{
	volatile unsigned videoMode : 3;
	volatile unsigned isGameboy : 1;
	volatile unsigned doDoubleBuffer : 1;
	volatile unsigned forceHBlank : 1;
	volatile unsigned objectMapping : 1;
	volatile unsigned forceScreenBlank : 1;
	volatile unsigned drawBG0 : 1;
	volatile unsigned drawBG1 : 1;
	volatile unsigned drawBG2 : 1;
	volatile unsigned drawBG3 : 1;
	volatile unsigned drawSprites : 1;
	volatile unsigned drawWindow0 : 1;
	volatile unsigned drawWindow1 : 1;
	volatile unsigned drawWindow2 : 1;
} PACKED;

struct BackgroundControlRegister
{
	volatile unsigned priority : 2;
	volatile unsigned baseTileBlock : 2;
	volatile unsigned filler : 2;
	volatile unsigned mosaic : 1;
	volatile unsigned palette : 1;
	volatile unsigned baseMapBlock : 5;
	volatile unsigned wrapFlag : 1;
	volatile unsigned backgroundSize : 2;
} PACKED;

struct BackgroundScrollRegister
{
	volatile short horizontalOffset : 10;
	volatile unsigned unusedHorizontal : 6;
	volatile short verticalOffset : 10;
	volatile unsigned unusedVertical : 6;
} PACKED;

struct DirectMemoryAccessRegister
{
	volatile unsigned sourceAddr : 28;
	volatile unsigned unusedSource : 4;
	volatile unsigned destinationAddr : 27;
	volatile unsigned unusedDest : 5;
	volatile unsigned count : 16;
	volatile unsigned filler : 5;
	volatile unsigned destinationAdjust : 2;
	volatile unsigned sourceAdjust : 2;
	volatile unsigned repeat : 1;
	volatile unsigned chunkSize : 1;
	volatile unsigned filler2 : 1;
	volatile unsigned sendTime : 2;
	volatile unsigned intOnFinish : 1;
	volatile unsigned enableDMA : 1;
} PACKED;


#define g_InputStatus ((volatile InputStatusRegister*)(&GBA_REG_P1))
#define g_InputControl ((volatile InputControlRegister*)(&GBA_REG_P1CNT))
#define g_Timers ((volatile TimerRegister*)(&GBA_REG_TM0D))
#define g_DMAControl ((volatile DirectMemoryAccessRegister*)(&GBA_REG_DM0DAD))
#define g_DisplayControl ((volatile DisplayControlRegister*)(&GBA_REG_DISPCNT))
#define g_DisplayStatus ((volatile DisplayStatusRegister*)(&GBA_REG_DISPSTAT))
#define g_BackgroundControl ((volatile BackgroundControlRegister*)(&GBA_REG_BG0CNT))
#define g_BackgroundScroll ((volatile BackgroundScrollRegister*)(&GBA_REG_BG0HOFS))
#define g_BackgroundPalette ((volatile unsigned short *)0x5000000)

#endif
