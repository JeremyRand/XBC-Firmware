CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)

TARGET = verify

CFLAGS = -O2 -mthumb -Wall -I../../libxrc -I../../../../include -I../libimage
LDFLAGS = -mthumb  ../../libxrc/libxrc-thumb.a ../../../../lib/libaxeso-thumb.a ../../../../lib/libgba-thumb.a

upload: $(TARGET).bin 
	xpcomm $(TARGET).bin ../logic

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

libxrc-thumb.a:
	$(MAKE) -C ../../libxrc libxrc-thumb.a

# Normal link line.  Includes fast LUT code
$(TARGET).elf: $(OBJECTS) libxrc-thumb.a 
	$(CXX) -o $(TARGET).elf $(OBJECTS) $(LDFLAGS) 

# Use this instead to test using the slow non-LUT stub
#$(TARGET).elf: $(OBJECTS) libxrc-thumb.a 
#	$(CXX) -o $(TARGET).elf $(OBJECTS) ../../libxrc/stubs/stub_lut_rgb2hsv_ov6620.cxx $(LDFLAGS) 


%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<

clean:
	-rm *.o
	-rm *.a
