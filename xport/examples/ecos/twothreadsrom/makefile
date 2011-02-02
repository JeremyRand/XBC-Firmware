INCDIR=../../../include/ecos
LIBDIR=../../../lib/ecos/rom
TARGET=twothreads
CFLAGS=-I$(INCDIR) -c -g -mthumb -fno-exceptions -ffunction-sections -fdata-sections -Wall -fverbose-asm -nostdlib
LDFLAGS=-mthumb -nostdlib -nostartfiles -Ttarget.ld -L$(LIBDIR) -Wl,--gc-sections -Wl,--static
CC = arm-agb-elf-gcc

all: upload

upload: twothreads.srec
	xpcomm twothreads.srec ../../primary

clean:
	-rm *.o *.bin *.srec *.elf

$(TARGET).bin: $(TARGET).elf  
	arm-agb-elf-objcopy -O binary $(TARGET).elf $(TARGET).bin

$(TARGET).srec: $(TARGET).elf  
	arm-agb-elf-objcopy -O srec $(TARGET).elf $(TARGET).srec

$(TARGET).elf: twothreads.o 
	$(CC) -g -o $(TARGET).elf twothreads.o $(LDFLAGS)
