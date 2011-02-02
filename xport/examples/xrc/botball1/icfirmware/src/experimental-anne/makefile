CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
AR = arm-agb-elf-ar
OBJCOPY  = arm-agb-elf-objcopy
ARCH = -mthumb-interwork

TARGET = anne
COMPILE=$(wildcard *.cxx)
OBJECTS=$(COMPILE:.cxx=.o)
LIBS= ../../lib/libicxbc.a ../../lib/libicxportcommon.a ../../../../../../lib/libimage-thumb-interwork.a ../../../../libxrc/libxrc-thumb-interwork.a ../../../../../../lib/libgba-thumb-interwork.a 
CFLAGS = -O2 $(ARCH) -Wall -I../../include  -I../../../../libxrc -I../../../../../../include 
LDFLAGS = $(ARCH) ../../lib/libicxbc.a ../../lib/libicxportcommon.a ../../../../../../lib/libimage-thumb-interwork.a ../../../../libxrc/libxrc-thumb-interwork.a ../../../../../../lib/libgba-thumb-interwork.a

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -v -O binary $(TARGET).elf ../../bin/$(TARGET).bin

$(TARGET).elf: $(OBJECTS) $(LIBS)
	$(CXX) -o $(TARGET).elf  $(OBJECTS) $(LDFLAGS) 

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<

install: $(TARGET).bin
	xpcomm ../../bin/$(TARGET).bin ../../../logic

clean:
	-rm ../../bin/$(TARGET).bin
	-rm *.o
	-rm *.elf
	-rm *.bin
