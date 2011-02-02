INCDIR=../../../include/ecos
LIBDIR=../../../lib/ecos/ram
TARGET=twothreads
CFLAGS=-mthumb -I$(INCDIR) -c -g -fno-exceptions -ffunction-sections -fdata-sections -Wall -fverbose-asm -nostdlib
LDFLAGS=-mthumb -nostdlib -nostartfiles -Ttarget.ld -L$(LIBDIR) -Wl,--gc-sections -Wl,--static
CC = arm-agb-elf-gcc

all: twothreads.elf

clean: 
	-rm *.o *.elf

$(TARGET).elf:twothreads.o 
	$(CC) -g -o $(TARGET).elf twothreads.o $(LDFLAGS)
