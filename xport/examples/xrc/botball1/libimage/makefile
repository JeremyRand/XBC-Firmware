############################################## 
# This library builds for both the GBA and the host computers.  Host
# sources should #include image.h or blob_image.h and link against
# libimage.a. GBA sources should #include gba_image.h or
# gba_blob_image.h and link against libimage-thumb.a.  Alternatively,
# GBA sources or makefiles can define GBA and include image.h or
# blob_image.h.

# WARNING: This depends on the hack that in this directory the host
# sources end in .cc, and the gba sources end in .cxx.  The common
# code is in the .cc files, and the source files gba_image.cxx and
# gba_blob_image.cxx define GBA and #include image.cc and
# blob_image.cc respectively.  This makefile will no longer work if
# these naming conventions are not followed, and there will be
# conflicts unless the .cc and .cxx files have different base names.

TARGET = libimage
CFLAGS = -O2 -I. -I../../../../include

##############################################
# GBA
GBA_CXX = arm-agb-elf-g++
GBA_SRCS=$(wildcard *.cxx)
GBA_OBJECTS=$(GBA_SRCS:.cxx=.o)
GBA_CFLAGS= $(CFLAGS) -DGBA -mthumb 

##############################################
# HOST
HOST_SRCS=$(wildcard *.cc)
HOST_OBJECTS=$(HOST_SRCS:.cc=.o)

##############################################
# Targets and build rules
all:	install

image.o:	image.cc image.h
blob_image.o:	blob_image.cc blob_image.h

gba_image.o:	gba_image.cxx gba_image.h image.cc image.h
gba_blob_image.o:	gba_blob_image.cxx blob_image.cc blob_image.h

install: 
	-rm *.o
	$(MAKE) $(TARGET)-thumb.a 
	-rm *.o
	$(MAKE) $(TARGET)-thumb-interwork.a
	rm $(TARGET)temp.a
	$(MAKE) $(TARGET).a
	rm *.o
	cp $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a ../../../../lib
	cp *.h ../../../../include

depend:
	makedepend -- $(CFLAGS) -- $(GBA_SRCS) $(HOST_SRCS)

.PHONY: $(TARGET)-thumb.a

$(TARGET)-thumb.a: 
	$(MAKE) $(TARGET)temp.a CFLAGS="-mthumb $(CFLAGS)"  
	cp $(TARGET)temp.a $(TARGET)-thumb.a

$(TARGET)-thumb-interwork.a: 
	$(MAKE) $(TARGET)temp.a CFLAGS="-mthumb -mthumb-interwork $(CFLAGS)"  
	cp $(TARGET)temp.a $(TARGET)-thumb-interwork.a

$(TARGET)temp.a: $(GBA_OBJECTS)
	arm-agb-elf-ar rvs $(TARGET)temp.a $(GBA_OBJECTS)

$(TARGET).a: $(HOST_OBJECTS)
	ar rvs $(TARGET).a $(HOST_OBJECTS)

clean:
	-rm *.o
	-rm *.a

%.o: %.cxx
	$(GBA_CXX) -c $(GBA_CFLAGS) $<

%.o: %.cc
	g++ -g -c $(CFLAGS) $<

# DO NOT DELETE
