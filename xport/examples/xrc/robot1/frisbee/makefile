CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++

TARGET = frisbee

CFLAGS = -O2 -mthumb -Wall -I../../libxrc -I../../../../include 
LDFLAGS = -mthumb  ../../libxrc/libxrc-thumb.a ../../../../lib/libaxeso-thumb.a ../../../../lib/libgba-thumb.a

upload: $(TARGET).bin 
	xpcomm $(TARGET).bin ../logic

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

libxrc-thumb.a:
	$(MAKE) -C ../../libxrc libxrc-thumb.a

$(TARGET).elf: $(TARGET).o libxrc-thumb.a
	$(CXX) -o $(TARGET).elf $(TARGET).o $(LDFLAGS) 

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<