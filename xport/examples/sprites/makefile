TARGET = sprites
CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)

CFLAGS = -g -mthumb -I../../include
LDFLAGS = -g -mthumb ../../lib/libgba-thumb.a

all:  vba

main.o:		main.cxx test_sprites.c ../../lib/libgba-thumb.a
sprites.o:	sprites.cxx sprites.h

vba:  $(TARGET).elf
	VisualBoyAdvance.exe $(TARGET).elf

upload: $(TARGET).bin
	xpcomm $(TARGET).bin ../primary


$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET).elf $(LDFLAGS) 

clean:
	-rm *.o
	-rm *.elf
	-rm *.a
	-rm *.bin

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<

%.c: %.pcx
	-pcx2sprite -c $<