CC = arm-agb-elf-gcc
CXX = arm-agb-elf-g++
CFLAGS=-O2 -mthumb

HEADERS=$(wildcard *.h)
COMPILE=$(wildcard *.cxx)
T_OBJECTS=$(COMPILE:.cxx=-t.o)
I_OBJECTS=$(COMPILE:.cxx=-i.o)
TARGET = libgba

THISMAKEFILE=-f $(CURDIR)/makefile

# Global support for DEBUG
ifdef DEBUG
CFLAGS += -DDEBUG
endif

all: install

textdisp-t.o:	textdisp.cxx textdisp.h
textdisp-i.o:	textdisp.cxx textdisp.h

install: $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a
	cp $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a ../../lib
	cp *.h ../../include

$(TARGET)-thumb.a: $(T_OBJECTS) $(HEADERS)
	arm-agb-elf-ar rvs $(TARGET)-thumb.a $(T_OBJECTS)

$(TARGET)-thumb-interwork.a: $(I_OBJECTS) $(HEADERS)
	arm-agb-elf-ar rvs $(TARGET)-thumb-interwork.a $(I_OBJECTS)

doc:
	doxygen doxygen.cfg

clean:
	-rm *.o
	-rm *.a

%-t.o: %.cxx
	$(CXX) -c $(CFLAGS) -o $@ $< 

%-i.o: %.cxx
	$(CXX) -c $(CFLAGS) -mthumb-interwork -o $@ $<
