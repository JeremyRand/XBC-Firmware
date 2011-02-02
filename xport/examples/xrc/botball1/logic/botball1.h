#ifndef _BOTBALL1_H
#define _BOTBALL1_H

// Botball1 bitstream 

// Primary module contains the following:
// Module                     Base location (GBA memory)
// -----------------------------------------------------
// Cport controller           0x9ffc200 (use CTextDisp)
// LED controller             0x9ffc220 (use xport.h)
// Interrupt controller       0x9ffc240 (use CInterruptCont)
// Bitstream identifier       0x9ffc3e0 (use xport.h)

// upper 8 bits of bitstream identifier (lower 8 bits are bitstream version)
#define BITSTREAM_IDENTIFIER				0x8d
#define BITSTREAM_XBC2005					0x8d01
#define BITSTREAM_XBC2006					0x8d03
#define BITSTREAM_XBC2007					0x8d05

// UART with receive FIFO -- use CUartQueue
#define UART_BASE							0x9ffd000
#define UART_VECTOR							16
#define UART_SET_BT()						*((volatile unsigned short *)UART_BASE) &= ~0x0030; \
											*((volatile unsigned short *)UART_BASE) |= 0x0010
#define UART_SET_RS232()					*((volatile unsigned short *)UART_BASE) &= ~0x0030
// USB is the same mux setting at the legacy RS232 port
#define UART_SET_USB()						UART_SET_RS232()
#define UART_SET_EXTRA()					*((volatile unsigned short *)UART_BASE) &= ~0x0030; \
											*((volatile unsigned short *)UART_BASE) |= 0x0020
// returns nonzero if USB is connected, zero if unconnected
#define UART_GET_USB_CONNECTED()			(*((volatile unsigned short *)UART_BASE) & 0x0080)

// returns nonzero if the UART is set to bluetooth
#define UART_GET_BT_MODE()				(*((volatile unsigned short *)UART_BASE) & 0x0010)

// Vision controller with frame grabber and 3-channel color filter -- use CVision
#define VISION_BASE							0x9ffe000
#define VISION_FRAME_VECTOR					17
#define VISION_SEGMENTQUEUE_VECTOR			18
#define VISION_LINEQUEUE_VECTOR				19
#define VISION_VERSION						2 // XBC2006

// 4-axis back-emf motor controller -- use CAxesOpen or CAxesClosed
#define BEMF4_BASE							0x9ffc400
#define BEMF4_VECTOR						20
#define BEMF4_VERSION						2 // XBC2006

// 16-bit gpio controller with edge-triggered interrupts -- use CGpioInt
#define GPIOINT_BASE						0x9ffd200
#define GPIOINT_VECTOR						21

// 4-axis RC servo controller -- use CRCServo
#define RCSERVO_BASE						0x9ffd400
#define RCSERVO_VERSION						2 // XBC2006

// 48-bit 1 microsecond resolution timer -- use CSimpTimer
#define TIMER_BASE							0x9ffd600

#endif
