#ifndef _GBA_H
#define _GBA_H

#define GBA_BASE_EXT_RAM	((unsigned short *)0x2000000)
#define GBA_BASE_INT_RAM	((unsigned short *)0x3000000)
#define GBA_BASE_PAL_RAM	((unsigned short *)0x5000000)
#define GBA_BASE_VRAM		((unsigned short *)0x6000000)
#define GBA_BASE_OAM		((unsigned short *)0x7000000)

#define GBA_BG_MODE_0		0
#define GBA_BG_MODE_1		1
#define GBA_BG_MODE_2		2
#define GBA_BG_MODE_3		3
#define GBA_BG_MODE_4		4
#define GBA_BG_MODE_5		5

#define GBA_BG0_ENABLE		1
#define GBA_BG1_ENABLE		2
#define GBA_BG2_ENABLE		4
#define GBA_BG3_ENABLE		8
#define GBA_OBJ_ENABLE		16

#define GBA_OBJ_1D_MAP		1
#define GBA_OBJ_2D_MAP		0

#define GBA_CHR_BASE		0xc

#define GBA_DISP_NROWS 160
#define GBA_DISP_NCOLS 240

#define GBA_KEY_A			0x0001
#define GBA_KEY_B			0x0002
#define GBA_KEY_SL			0x0004
#define GBA_KEY_ST			0x0008
#define GBA_KEY_RT			0x0010
#define GBA_KEY_LFT			0x0020
#define GBA_KEY_UP			0x0040
#define GBA_KEY_DWN			0x0080
#define GBA_KEY_R			0x0100
#define GBA_KEY_L			0x0200

#define GBA_SER_BAUD_MASK	0x0003
#define GBA_SER_BAUD_9600	0x0000
#define GBA_SER_BAUD_38400	0x0001
#define GBA_SER_BAUD_57600	0x0002
#define GBA_SER_BAUD_115200	0x0003
#define GBA_SER_CTS			0x0004

#define GBA_PHI_MASK		0x1800
#define GBA_PHI_NONE		0x0000
#define GBA_PHI_4_19MHZ		0x0800
#define GBA_PHI_8_38MHZ		0x1000
#define GBA_PHI_16_76MHZ	0x1800

#define GBA_INT_VBLANK		0x0001
#define GBA_INT_HBLANK		0x0002
#define GBA_INT_VCOUNT		0x0004
#define GBA_INT_TIMER0		0x0008
#define GBA_INT_TIMER1		0x0010
#define GBA_INT_TIMER2		0x0020
#define GBA_INT_TIMER3		0x0040
#define GBA_INT_SERIAL		0x0080
#define GBA_INT_DMA0		0x0100
#define GBA_INT_DMA1		0x0200
#define GBA_INT_DMA2		0x0400
#define GBA_INT_DMA3		0x0800
#define GBA_INT_KEY			0x1000
#define GBA_INT_CART		0x2000


#define GBA_REG_DISPCNT		(*(volatile unsigned short *)0x4000000)
#define GBA_REG_DISPSTAT	(*(volatile unsigned short *)0x4000004)
#define GBA_REG_VCOUNT		(*(volatile unsigned short *)0x4000006)
#define GBA_REG_BG0CNT		(*(volatile unsigned short *)0x4000008)
#define GBA_REG_BG1CNT		(*(volatile unsigned short *)0x400000A)
#define GBA_REG_BG2CNT		(*(volatile unsigned short *)0x400000C)
#define GBA_REG_BG3CNT		(*(volatile unsigned short *)0x400000E)
#define GBA_REG_BG0HOFS		(*(volatile unsigned short *)0x4000010)
#define GBA_REG_BG0VOFS		(*(volatile unsigned short *)0x4000012)
#define GBA_REG_BG1HOFS		(*(volatile unsigned short *)0x4000014)
#define GBA_REG_BG1VOFS		(*(volatile unsigned short *)0x4000016)
#define GBA_REG_BG2HOFS		(*(volatile unsigned short *)0x4000018)
#define GBA_REG_BG2VOFS		(*(volatile unsigned short *)0x400001A)
#define GBA_REG_BG3HOFS		(*(volatile unsigned short *)0x400001C)
#define GBA_REG_BG3VOFS		(*(volatile unsigned short *)0x400001E)
#define GBA_REG_BG2PA		(*(volatile unsigned short *)0x4000020)
#define GBA_REG_BG2PB		(*(volatile unsigned short *)0x4000022)
#define GBA_REG_BG2PC		(*(volatile unsigned short *)0x4000024)
#define GBA_REG_BG2PD		(*(volatile unsigned short *)0x4000026)
#define GBA_REG_BG2X		(*(volatile unsigned long *)0x4000028)
#define GBA_REG_BG2X_L		(*(volatile unsigned short *)0x4000028)
#define GBA_REG_BG2X_H		(*(volatile unsigned short *)0x400002A)
#define GBA_REG_BG2Y		(*(volatile unsigned long *)0x400002C)
#define GBA_REG_BG2Y_L		(*(volatile unsigned short *)0x400002C)
#define GBA_REG_BG2Y_H		(*(volatile unsigned short *)0x400002E)
#define GBA_REG_BG3PA		(*(volatile unsigned short *)0x4000030)
#define GBA_REG_BG3PB		(*(volatile unsigned short *)0x4000032)
#define GBA_REG_BG3PC		(*(volatile unsigned short *)0x4000034)
#define GBA_REG_BG3PD		(*(volatile unsigned short *)0x4000036)
#define GBA_REG_BG3X		(*(volatile unsigned long *)0x4000038)
#define GBA_REG_BG3X_L		(*(volatile unsigned short *)0x4000038)
#define GBA_REG_BG3X_H		(*(volatile unsigned short *)0x400003A)
#define GBA_REG_BG3Y		(*(volatile unsigned long *)0x400003C)
#define GBA_REG_BG3Y_L		(*(volatile unsigned short *)0x400003C)
#define GBA_REG_BG3Y_H		(*(volatile unsigned short *)0x400003E)
#define GBA_REG_WIN0H		(*(volatile unsigned short *)0x4000040)
#define GBA_REG_WIN1H		(*(volatile unsigned short *)0x4000042)
#define GBA_REG_WIN0V		(*(volatile unsigned short *)0x4000044)
#define GBA_REG_WIN1V		(*(volatile unsigned short *)0x4000046)
#define GBA_REG_WININ		(*(volatile unsigned short *)0x4000048)
#define GBA_REG_WINOUT		(*(volatile unsigned short *)0x400004A)
#define GBA_REG_MOSAIC		(*(volatile unsigned short *)0x400004C)
#define GBA_REG_BLDMOD		(*(volatile unsigned short *)0x4000050)
#define GBA_REG_COLEY1		(*(volatile unsigned short *)0x4000052)
#define GBA_REG_COLEY2		(*(volatile unsigned short *)0x4000054)
#define GBA_REG_SG10		(*(volatile unsigned long *)0x4000060)
#define GBA_REG_SG10_L		(*(volatile unsigned short *)0x4000060)
#define GBA_REG_SG10_H		(*(volatile unsigned short *)0x4000062)
#define GBA_REG_SG11		(*(volatile unsigned short *)0x4000064)
#define GBA_REG_SG20		(*(volatile unsigned short *)0x4000068)
#define GBA_REG_SG21		(*(volatile unsigned short *)0x400006C)
#define GBA_REG_SG30		(*(volatile unsigned long *)0x4000070)
#define GBA_REG_SG30_L		(*(volatile unsigned short *)0x4000070)
#define GBA_REG_SG30_H		(*(volatile unsigned short *)0x4000072)
#define GBA_REG_SG31		(*(volatile unsigned short *)0x4000074)
#define GBA_REG_SG40		(*(volatile unsigned short *)0x4000078)
#define GBA_REG_SG41		(*(volatile unsigned short *)0x400007C)
#define GBA_REG_SGCNT0		(*(volatile unsigned long *)0x4000080)
#define GBA_REG_SGCNT0_L	(*(volatile unsigned short *)0x4000080)
#define GBA_REG_SGCNT0_H	(*(volatile unsigned short *)0x4000082)
#define GBA_REG_SGCNT1		(*(volatile unsigned short *)0x4000084)
#define GBA_REG_SGBIAS		(*(volatile unsigned short *)0x4000088)
#define GBA_REG_SGWR0		(*(volatile unsigned long *)0x4000090)
#define GBA_REG_SGWR0_L		(*(volatile unsigned short *)0x4000090)
#define GBA_REG_SGWR0_H		(*(volatile unsigned short *)0x4000092)
#define GBA_REG_SGWR1		(*(volatile unsigned long *)0x4000094)
#define GBA_REG_SGWR1_L		(*(volatile unsigned short *)0x4000094)
#define GBA_REG_SGWR1_H		(*(volatile unsigned short *)0x4000096)
#define GBA_REG_SGWR2		(*(volatile unsigned long *)0x4000098)
#define GBA_REG_SGWR2_L		(*(volatile unsigned short *)0x4000098)
#define GBA_REG_SGWR2_H		(*(volatile unsigned short *)0x400009A)
#define GBA_REG_SGWR3		(*(volatile unsigned long *)0x400009C)
#define GBA_REG_SGWR3_L		(*(volatile unsigned short *)0x400009C)
#define GBA_REG_SGWR3_H		(*(volatile unsigned short *)0x400009E)
#define GBA_REG_SGFIF0A		(*(volatile unsigned long *)0x40000A0)
#define GBA_REG_SGFIFOA_L	(*(volatile unsigned short *)0x40000A0)
#define GBA_REG_SGFIFOA_H	(*(volatile unsigned short *)0x40000A2)
#define GBA_REG_SGFIFOB		(*(volatile unsigned long *)0x40000A4)
#define GBA_REG_SGFIFOB_L	(*(volatile unsigned short *)0x40000A4)
#define GBA_REG_SGFIFOB_H	(*(volatile unsigned short *)0x40000A6)
#define GBA_REG_DM0SAD		(*(volatile unsigned long *)0x40000B0)
#define GBA_REG_DM0SAD_L	(*(volatile unsigned short *)0x40000B0)
#define GBA_REG_DM0SAD_H	(*(volatile unsigned short *)0x40000B2)
#define GBA_REG_DM0DAD		(*(volatile unsigned long *)0x40000B4)
#define GBA_REG_DM0DAD_L	(*(volatile unsigned short *)0x40000B4)
#define GBA_REG_DM0DAD_H	(*(volatile unsigned short *)0x40000B6)
#define GBA_REG_DM0CNT		(*(volatile unsigned long *)0x40000B8)
#define GBA_REG_DM0CNT_L	(*(volatile unsigned short *)0x40000B8)
#define GBA_REG_DM1SAD_H	(*(volatile unsigned short *)0x40000BE)
#define GBA_REG_DM1DAD		(*(volatile unsigned long *)0x40000C0)
#define GBA_REG_DM1DAD_L	(*(volatile unsigned short *)0x40000C0)
#define GBA_REG_DM1DAD_H	(*(volatile unsigned short *)0x40000C2)
#define GBA_REG_DM1CNT		(*(volatile unsigned long *)0x40000C4)
#define GBA_REG_DM1CNT_L	(*(volatile unsigned short *)0x40000C4)
#define GBA_REG_DM1CNT_H	(*(volatile unsigned short *)0x40000C6)
#define GBA_REG_DM2SAD		(*(volatile unsigned long *)0x40000C8)
#define GBA_REG_DM2SAD_L	(*(volatile unsigned short *)0x40000C8)
#define GBA_REG_DM2SAD_H	(*(volatile unsigned short *)0x40000CA)
#define GBA_REG_DM2DAD		(*(volatile unsigned long *)0x40000CC)
#define GBA_REG_DM2DAD_L	(*(volatile unsigned short *)0x40000CC)
#define GBA_REG_DM2DAD_H	(*(volatile unsigned short *)0x40000CE)
#define GBA_REG_DM2CNT		(*(volatile unsigned long *)0x40000D0)
#define GBA_REG_DM2CNT_L	(*(volatile unsigned short *)0x40000D0)
#define GBA_REG_DM2CNT_H	(*(volatile unsigned short *)0x40000D2)
#define GBA_REG_DM3SAD		(*(volatile unsigned long *)0x40000D4)
#define GBA_REG_DM3SAD_L	(*(volatile unsigned short *)0x40000D4)
#define GBA_REG_DM3SAD_H	(*(volatile unsigned short *)0x40000D6)
#define GBA_REG_DM3DAD		(*(volatile unsigned long *)0x40000D8)
#define GBA_REG_DM3DAD_L	(*(volatile unsigned short *)0x40000D8)
#define GBA_REG_DM3DAD_H	(*(volatile unsigned short *)0x40000DA)
#define GBA_REG_DM3CNT		(*(volatile unsigned long *)0x40000DC)
#define GBA_REG_DM3CNT_L	(*(volatile unsigned short *)0x40000DC)
#define GBA_REG_DM3CNT_H	(*(volatile unsigned short *)0x40000DE)
#define GBA_REG_TM0D		(*(volatile unsigned short *)0x4000100)
#define GBA_REG_TM0CNT		(*(volatile unsigned short *)0x4000102)
#define GBA_REG_TM1D		(*(volatile unsigned short *)0x4000104)
#define GBA_REG_TM1CNT		(*(volatile unsigned short *)0x4000106)
#define GBA_REG_TM2D		(*(volatile unsigned short *)0x4000108)
#define GBA_REG_TM2CNT		(*(volatile unsigned short *)0x400010A)
#define GBA_REG_TM3D		(*(volatile unsigned short *)0x400010C)
#define GBA_REG_TM3CNT		(*(volatile unsigned short *)0x400010E)
#define GBA_REG_SCD0		(*(volatile unsigned short *)0x4000120)
#define GBA_REG_SCD1		(*(volatile unsigned short *)0x4000122)
#define GBA_REG_SCD2		(*(volatile unsigned short *)0x4000124)
#define GBA_REG_SCD3		(*(volatile unsigned short *)0x4000126)
#define GBA_REG_SCCNT		(*(volatile unsigned long *)0x4000128)
#define GBA_REG_SCCNT_L		(*(volatile unsigned short *)0x4000128)
#define GBA_REG_SCCNT_H		(*(volatile unsigned short *)0x400012A)
#define GBA_REG_P1			(*(volatile unsigned short *)0x4000130)
#define GBA_REG_P1CNT		(*(volatile unsigned short *)0x4000132)
#define GBA_REG_R			(*(volatile unsigned short *)0x4000134)
#define GBA_REG_HS_CTRL		(*(volatile unsigned short *)0x4000140)
#define GBA_REG_JOYRE		(*(volatile unsigned long *)0x4000150)
#define GBA_REG_JOYRE_L		(*(volatile unsigned short *)0x4000150)
#define GBA_REG_JOYRE_H		(*(volatile unsigned short *)0x4000152)
#define GBA_REG_JOYTR		(*(volatile unsigned long *)0x4000154)
#define GBA_REG_JOYTR_L		(*(volatile unsigned short *)0x4000154)
#define GBA_REG_JOYTR_H		(*(volatile unsigned short *)0x4000156)
#define GBA_REG_JSTAT		(*(volatile unsigned long *)0x4000158)
#define GBA_REG_JSTAT_L		(*(volatile unsigned short *)0x4000158)
#define GBA_REG_JSTAT_H		(*(volatile unsigned short *)0x400015A)
#define GBA_REG_IE			(*(volatile unsigned short *)0x4000200)
#define GBA_REG_IF			(*(volatile unsigned short *)0x4000202)
#define GBA_REG_WSCNT		(*(volatile unsigned short *)0x4000204)
#define GBA_REG_IME			(*(volatile unsigned short *)0x4000208)
#define GBA_REG_PAUSE		(*(volatile unsigned short *)0x4000300)	

#define GBA_REG_DMA0SAD     (*(volatile unsigned long *)0x40000B0)	//DMA0 Source Address
#define GBA_REG_DMA0SAD_L   (*(volatile unsigned short *)0x40000B0)	//DMA0 Source Address Low Value
#define GBA_REG_DMA0SAD_H   (*(volatile unsigned short *)0x40000B2)	//DMA0 Source Address High Value
#define GBA_REG_DMA0DAD     (*(volatile unsigned long *)0x40000B4)	//DMA0 Destination Address
#define GBA_REG_DMA0DAD_L   (*(volatile unsigned short *)0x40000B4)	//DMA0 Destination Address Low Value
#define GBA_REG_DMA0DAD_H   (*(volatile unsigned short *)0x40000B6)	//DMA0 Destination Address High Value
#define GBA_REG_DMA0CNT     (*(volatile unsigned long *)0x40000B8)	//DMA0 Control (Amount)
#define GBA_REG_DMA0CNT_L   (*(volatile unsigned short *)0x40000B8)	//DMA0 Control Low Value
#define GBA_REG_DMA0CNT_H   (*(volatile unsigned short *)0x40000BA)	//DMA0 Control High Value

#define GBA_REG_DMA1SAD     (*(volatile unsigned long *)0x40000BC)	//DMA1 Source Address
#define GBA_REG_DMA1SAD_L   (*(volatile unsigned short *)0x40000BC)	//DMA1 Source Address Low Value
#define GBA_REG_DMA1SAD_H   (*(volatile unsigned short *)0x40000BE)	//DMA1 Source Address High Value
#define GBA_REG_DMA1DAD     (*(volatile unsigned long *)0x40000C0)	//DMA1 Desination Address
#define GBA_REG_DMA1DAD_L   (*(volatile unsigned short *)0x40000C0)	//DMA1 Destination Address Low Value
#define GBA_REG_DMA1DAD_H   (*(volatile unsigned short *)0x40000C2)	//DMA1 Destination Address High Value
#define GBA_REG_DMA1CNT     (*(volatile unsigned long *)0x40000C4)	//DMA1 Control (Amount)
#define GBA_REG_DMA1CNT_L   (*(volatile unsigned short *)0x40000C4)	//DMA1 Control Low Value
#define GBA_REG_DMA1CNT_H   (*(volatile unsigned short *)0x40000C6)	//DMA1 Control High Value

#define GBA_REG_DMA2SAD     (*(volatile unsigned long *)0x40000C8)	//DMA2 Source Address
#define GBA_REG_DMA2SAD_L   (*(volatile unsigned short *)0x40000C8)	//DMA2 Source Address Low Value
#define GBA_REG_DMA2SAD_H   (*(volatile unsigned short *)0x40000CA)	//DMA2 Source Address High Value
#define GBA_REG_DMA2DAD     (*(volatile unsigned long *)0x40000CC)	//DMA2 Destination Address
#define GBA_REG_DMA2DAD_L   (*(volatile unsigned short *)0x40000CC)	//DMA2 Destination Address Low Value
#define GBA_REG_DMA2DAD_H   (*(volatile unsigned short *)0x40000CE)	//DMA2 Destination Address High Value
#define GBA_REG_DMA2CNT     (*(volatile unsigned long *)0x40000D0)	//DMA2 Control (Amount)
#define GBA_REG_DMA2CNT_L   (*(volatile unsigned short *)0x40000D0)	//DMA2 Control Low Value
#define GBA_REG_DMA2CNT_H   (*(volatile unsigned short *)0x40000D2)	//DMA2 Control High Value

#define GBA_REG_DMA3SAD     (*(volatile unsigned long *)0x40000D4)	//DMA3 Source Address
#define GBA_REG_DMA3SAD_L   (*(volatile unsigned short *)0x40000D4)	//DMA3 Source Address Low Value
#define GBA_REG_DMA3SAD_H   (*(volatile unsigned short *)0x40000D6)	//DMA3 Source Address High Value
#define GBA_REG_DMA3DAD     (*(volatile unsigned long *)0x40000D8)	//DMA3 Destination Address
#define GBA_REG_DMA3DAD_L   (*(volatile unsigned short *)0x40000D8)	//DMA3 Destination Address Low Value
#define GBA_REG_DMA3DAD_H   (*(volatile unsigned short *)0x40000DA)	//DMA3 Destination Address High Value
#define GBA_REG_DMA3CNT     (*(volatile unsigned long *)0x40000DC)	//DMA3 Control (Amount)
#define GBA_REG_DMA3CNT_L   (*(volatile unsigned short *)0x40000DC)	//DMA3 Control Low Value
#define GBA_REG_DMA3CNT_H   (*(volatile unsigned short *)0x40000DE)	//DMA3 Control High Value

#endif
