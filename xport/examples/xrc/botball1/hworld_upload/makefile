TARGET = hworld_upload
CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)

CFLAGS = -O2 -mthumb -I../../../../include 
LDFLAGS = -mthumb ../../../../lib/libgba-thumb.a -T../share/upload.ld

upload: $(TARGET).bin
	upload.exe $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET).elf $(LDFLAGS) -Lupload.ld

clean:
	-rm *.o
	-rm *.elf
	-rm *.a
	-rm *.bin

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<
