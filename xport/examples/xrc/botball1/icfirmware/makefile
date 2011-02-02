MAKETARGET = libgba nonpcode icxview libicxportcommon libicxbc icfirmware

export
#ifeq ($(OS),Windows_NT)
#CC = arm-elf-gcc
#CXX = arm-elf-g++
#AR = arm-elf-ar
#OBJCOPY  = arm-elf-objcopy
#else
CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
AR = arm-agb-elf-ar
OBJCOPY  = arm-agb-elf-objcopy
#endif

ARCH = -mthumb-interwork


all: includes
	$(foreach SINGLETARGET, $(MAKETARGET), $(MAKE) -C ./src/$(SINGLETARGET);)
	
includes:
	find ./src \( -iname '*.inl' -o -iname '*.h' \) -exec cp '{}' include \;

#install-xrc:
#	xpcomm  ./bin/icxrc.bin ./xrclogic

#install-xbc:
#	xpcomm -resetauto  ./bin/icxbc.bin ../logic

doc:
	doxygen doxygen.cfg

clean: 
	-rm bin/*.*
	-rm lib/*.*
	-rm include/*.*
	$(foreach SINGLETARGET, $(MAKETARGET), $(MAKE) -C ./src/$(SINGLETARGET) clean;)
