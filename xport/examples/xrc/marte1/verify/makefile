CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)

TARGET = axistest

CFLAGS = -O2 -mthumb -Wall -I../../libxrc -I../../../../include -I../libimage
LDFLAGS = -mthumb  ../../libxrc/libxrc-thumb.a ../../../../lib/libaxeso-thumb.a ../libmarte/libmarte-thumb.a ../../../../lib/libgba-thumb.a

upload: $(TARGET).bin 
	xpcomm $(TARGET).bin ../logic

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

libxrc-thumb.a:
	$(MAKE) -C ../../libxrc libxrc-thumb.a

libmarte-thumb.a:
	$(MAKE) -C ../libmarte libmarte-thumb.a

$(TARGET).elf: $(OBJECTS) libxrc-thumb.a libmarte-thumb.a
	$(CXX) -o $(TARGET).elf $(OBJECTS) $(LDFLAGS) 

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<

clean:
	-rm *.o
	-rm *.a
