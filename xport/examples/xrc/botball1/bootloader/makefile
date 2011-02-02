TARGET = bootloader
CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)
STUBS=../../libxrc/stubs/stub_lut_rgb2hsv_ov6620.cxx

CFLAGS = -O2 -mthumb -Wall -I../../libxrc -I../../librpc  -I../../../../include 
#-I../libimage
LDFLAGS = -mthumb  ../../librpc/librpc-thumb.a  ../../libxrc/libxrc-thumb.a ../../../../lib/libaxeso-thumb.a ../../../../lib/libgba-thumb.a
#../libimage/libimage-thumb.a

upload: $(TARGET).bin 
	xpcomm $(TARGET).bin ../logic

$(TARGET).bin: $(TARGET).elf
#padding to 0x20000 so that we can cat with icfirmware
	arm-agb-elf-objcopy -v -O binary --pad-to 0x08020000 $(TARGET).elf $(TARGET).bin

libxrc-thumb.a:
	$(MAKE) -C ../../libxrc libxrc-thumb.a

librpc-thumb.a:
	$(MAKE) -C ../../librpc librpc-thumb.a

$(TARGET).elf: $(OBJECTS) $(STUBS) librpc-thumb.a libxrc-thumb.a
	$(CXX) -o $(TARGET).elf $(OBJECTS) $(STUBS) $(LDFLAGS) 

clean:
	-rm *.o
	-rm *.elf
	-rm *.a
	-rm *.bin

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<
