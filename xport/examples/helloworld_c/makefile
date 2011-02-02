TARGET = helloworld_c
CC = arm-agb-elf-gcc
SRCS=$(wildcard *.c)
OBJECTS=$(SRCS:.c=.o)

CFLAGS = -mthumb 
LDFLAGS = -mthumb ../../lib/libgba-thumb.a

upload: $(TARGET).bin
	xpcomm $(TARGET).bin ../primary

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET).elf $(LDFLAGS) 

clean:
	-rm *.o
	-rm *.elf
	-rm *.a
	-rm *.bin

%.o: %.c
	$(CC) -c $(CFLAGS) $<
