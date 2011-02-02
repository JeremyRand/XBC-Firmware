CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++

SRCS= motor_runaway.cxx
TARGET = motor_runaway.exe

OBJECTS=$(SRCS:.cxx=.o)

CFLAGS = -O2 -mthumb -Wall -I../../libxrc -I../../../../include 

# This version uses the serial uploader
LDFLAGS = -mthumb  ../../librpc/librpc-thumb.a ../../libxrc/libxrc-thumb.a \
	  ../../../../lib/libgba-thumb.a \
	  ../../../../lib/libaxeso-thumb.a -T../share/upload.ld

# Change to the following if you want to use xpcomm
#LDFLAGS = -mthumb  ../../libxrc/libxrc-thumb.a \
#	  ../../../../lib/libgba-thumb.a

# This version uses the serial uploader
upload: $(TARGET).bin 
	upload $(TARGET).bin

# Change to the following if you want to use xpcomm
#upload: $(TARGET).bin 
#	xpcomm $(TARGET).bin ../logic

../../libxrc/libxrc-thumb.a:
	$(MAKE) -C ../../libxrc libxrc-thumb.a

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin


$(TARGET).elf: $(OBJECTS) ../../libxrc/libxrc-thumb.a
	$(CXX) -o $(TARGET).elf $(OBJECTS) $(LDFLAGS) 

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<