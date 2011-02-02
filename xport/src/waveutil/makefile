ifeq ($(TERM),cygwin)   PLATFORM=win32
   TARGET=waveutil.exe
else 
   TARGET=waveutil
endif
CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
COMPILE=$(wildcard *.cxx)
OBJECTS=$(COMPILE:.cxx=.o)

install: $(TARGET)
	cp $(TARGET) ../../bin

clean:
	-rm *.o $(TARGET)

$(TARGET): waveutil.cxx
	g++ -o $(TARGET) waveutil.cxx

