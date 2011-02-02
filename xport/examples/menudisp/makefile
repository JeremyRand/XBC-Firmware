TARGET = menudisp
CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)

CFLAGS = -g -mthumb -I../../include
LDFLAGS = -g -mthumb ../../lib/libgba-thumb.a

all:  vba 

main.o:		main.cxx ../../lib/libgba-thumb.a

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

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<
