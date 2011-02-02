CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
SRCS= vision-anne.cxx xrc_image.cxx blob.cxx blob_image.cxx tone.cxx
TARGET = vision-anne

#SRCS= tone.cxx test_tone.cxx
#TARGET = test_tone

OBJECTS=$(SRCS:.cxx=.o)

CFLAGS = -O2 -mthumb -Wall -I../../libxrc -I../../../../include 

# This version uses the serial uploader
LDFLAGS = -mthumb  ../../libxrc/libxrc-thumb.a \
	  ../../../../lib/libgba-thumb.a 

# Change to the following if you want to use xpcomm
#LDFLAGS = -mthumb  ../../libxrc/libxrc-thumb.a \
#	  ../../../../lib/libgba-thumb.a

# This version uses the serial uploader
upload: $(TARGET).bin 
	xpcomm $(TARGET).bin ../logic

# Change to the following if you want to use xpcomm
#upload: $(TARGET).bin 
#	xpcomm $(TARGET).bin ../logic

###################################################################
# Deal with dependencies
xrc_image.o:	xrc_image.cxx xrc_image.h
blob.o:		blob.cxx blob.h
blob_image.o:	blob_image.cxx blob_image.h blob.h xrc_image.h
vision-anne.o:	vision-anne.cxx xrc_image.h blob.h

tone.o:		tone.cxx tone.h
test_tone.o:	tone.cxx test_tone.cxx tone.h

$(TARGET).bin: $(TARGET).elf
	arm-agb-elf-objcopy -v -O binary $(TARGET).elf $(TARGET).bin

libxrc-thumb.a:
	$(MAKE) -C ../../libxrc libxrc-thumb.a

$(TARGET).elf: $(OBJECTS) libxrc-thumb.a
	$(CXX) -o $(TARGET).elf $(OBJECTS) $(LDFLAGS) 

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<