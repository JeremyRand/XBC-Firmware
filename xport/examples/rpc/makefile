TARGET = rpc
CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)

CFLAGS = -mthumb 
LDFLAGS = -mthumb ../../lib/libgba-thumb.a

upload: $(TARGET).bin
	xpcomm $(TARGET).bin ../primary

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET).elf $(LDFLAGS) 

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<
