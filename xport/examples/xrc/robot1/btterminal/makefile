CXX = arm-agb-elf-g++

CFLAGS = -O2 -mthumb -Wall -I../../libxrc -I../../../../include 
LDFLAGS = -mthumb  ../../libxrc/libxrc-thumb.a ../../../../lib/libaxeso-thumb.a ../../../../lib/libgba-thumb.a

upload: termslave.bin 
	xpcomm $< ../logic

all: termmaster.bin termslave.bin 

libxrc-thumb.a:
	$(MAKE) -C ../../libxrc libxrc-thumb.a

termmaster.elf: libxrc-thumb.a
	$(CXX) -o $@ -DMASTER btterminal.cxx $(CFLAGS) $(LDFLAGS) 

termslave.elf: libxrc-thumb.a
	$(CXX) -o $@ btterminal.cxx $(CFLAGS) $(LDFLAGS) 

%.bin: %.elf
	arm-agb-elf-objcopy -v -O binary $< $@
