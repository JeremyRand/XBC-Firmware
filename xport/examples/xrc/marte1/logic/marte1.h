#ifndef _MARTE1_H
#define _MARTE1_H

// Marte1 bitstream 

// Primary module contains the following:
// Module                     Base location (GBA memory)
// -----------------------------------------------------
// Cport controller           0x9ffc200 (use CTextDisp)
// LED controller             0x9ffc220 (use xport.h)
// Interrupt controller       0x9ffc240 (use CInterruptCont)
// Bitstream identifier       0x9ffc3e0 (use xport.h)

// upper 8 bits of bitstream identifier (lower 8 bits are bitstream version)
#define BITSTREAM_IDENTIFIER				0x8e

// UART with receive FIFO -- use CUartQueue
#define UART_BASE							0x9ffd000
#define UART_VECTOR							16

#define GPIO_BASE							0x9ffd200
#define GPIO_NUM							19 // number of GPIOs
#define GPIO_REG_NUM						(GPIO_NUM+15)/16
#define GPIO_REG(i)							*((volatile unsigned short *)GPIO_BASE+i)
#define GPIO_DDR(i)							GPIO_REG(i)
#define GPIO_DATA(i)						GPIO_REG(i+GPIO_REG_NUM)

#define ADC_BASE							0x9ffd400

#define QUADRATURE_BASE						0x9ffd600
#define QUADRATURE_REG(i)					*((volatile unsigned short *)QUADRATURE_BASE+i)

#define PWM_BASE							0x9ffd800
#define PWM_REG(i)							*((volatile unsigned short *)PWM_BASE+i)

#define DIR_BASE							0x9ffda00

#define DIR_REG			*((volatile unsigned short *)DIR_BASE)


#endif
