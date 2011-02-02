CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
AR = arm-agb-elf-ar
OBJCOPY  = arm-agb-elf-objcopy
ARCH = -mthumb-interwork

TARGET = icxbc
COMPILE=$(wildcard *.cxx)
OBJECTS=$(COMPILE:.cxx=.o)
LIBS= ../../lib/lib$(TARGET).a ../../lib/libicxportcommon.a ../../../../libxrc/libxrc-thumb-interwork.a ../../lib/libgba-thumb-interwork.a ../../lib/libgba-thumb-interwork.a ../../lib/icxview.a ../../lib/nonpcode.a ../../../../../../lib/libaxeso-thumb-interwork.a 
CFLAGS = -O2 $(ARCH) -Wall -I../../include  -I../../../../libxrc -I../../../../../../include -D $(TARGET)
LDFLAGS = $(ARCH) $(LIBS)

# By default this now uses the bootloader and serial port for upload.
# If you don't want to use the bootloader, comment out the following
# line.  If you do want to use the bootloader and it isn't already
# loaded, install it by going into ../../../bootloader and typing
# make.  If it can't find upload.exe, go into ../../../upload, build
# the project under visual studio, then do 'make' in that directory to
# copy upload/Debug/upload.exe to the top level bin directory.
ifndef USE_SIM
# NOT SIM
#USE_BOOTLOADER ?= 1
ifdef USE_BOOTLOADER
LDFLAGS += -T../../../share/upload.ld
endif
ifdef EMBED_BOOTLOADER
LDFLAGS += -T../../../share/upload.ld
endif
all: upload
else
# SIM
CFLAGS += -DUSE_SIM
all: vba 
endif

ifdef DEBUG
CFLAGS += -DDEBUG
endif

vba:  $(TARGET).elf
	VisualBoyAdvance.exe $(TARGET).elf

upload: ../../bin/$(TARGET).bin
ifdef USE_BOOTLOADER
ifdef IC_LIBDIR
	echo "*** Copying to IC lib directory $(IC_LIBDIR)/xbc"
	cp ../../bin/$(TARGET).bin $(IC_LIBDIR)/xbc/libxbc.frm
else
	echo "*** Uploading over serial"
	upload.exe ../../bin/$(TARGET).bin
endif
else
ifdef EMBED_BOOTLOADER
	echo "*** Embedding bootloader"
	cat ../../../bootloader/bootloader.bin ../../bin/$(TARGET).bin > ../../bin/$(TARGET)bl.bin
	echo "*** Uploading over xpcomm with bootloader"
	xpcomm ../../../logic ../../bin/$(TARGET)bl.bin
else
	echo "*** Uploading over xpcomm"
	xpcomm ../../../logic ../../bin/$(TARGET).bin 
endif
endif

ic-install: ../../bin/$(TARGET).bin
ifdef IC_LIBDIR
	cp ../../bin/$(TARGET).bin $(IC_LIBDIR)/xbc/icxbc.frm
endif

../../bin/$(TARGET).bin: $(TARGET).elf
ifdef CLEAR_PROGRAMS
	$(OBJCOPY) -v -O binary --pad-to 0x08180000 $(TARGET).elf ../../bin/$(TARGET).bin
else
	$(OBJCOPY) -v -O binary $(TARGET).elf ../../bin/$(TARGET).bin
endif

$(TARGET).elf: $(OBJECTS) $(LIBS)
	$(CXX) -o $(TARGET).elf  $(OBJECTS) $(LDFLAGS) 

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<

clean:
	-rm ../../bin/$(TARGET).bin
	-rm *.o
	-rm *.elf
	-rm *.bin
