CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
CFLAGS = -O2 -Wall -I../../../include -mthumb

HEADERS=$(wildcard *.h)
COMPILE=$(wildcard *.cxx)
T_OBJECTS=$(COMPILE:.cxx=-t.o)
I_OBJECTS=$(COMPILE:.cxx=-i.o)
TARGET = libxrc

# Global support for DEBUG
ifdef DEBUG
CFLAGS += -DDEBUG
endif

install: $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a
	cp $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a ../../../lib
	cp *.h ../../../include

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

blobcontext-i.o blobcontext-t.o:		blobcontext.cxx blobcontext.h blob_sprites.c
modelcontext-i.o modelcontext-t.o:		modelcontext.cxx modelcontext.h model_sprites.c

.PHONY: $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a

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

%.c: %.pcx
	-pcx2sprite -c $<

NATIVE_CXX=g++

generate_lut_rgb2hsv_ov6620: generate_lut_rgb2hsv_ov6620.cc xrc_lut.cxx stubs/stub_lut_rgb2hsv_ov6620.cxx
	$(NATIVE_CXX) -Wall $^ -o $@

lut: generate_lut_rgb2hsv_ov6620
	./generate_lut_rgb2hsv_ov6620 > lut_rgb2hsv_ov6620.cxx

#####################################################################
# msimage

MSI_SRCS= generate_msimage.cc image.cc host_image.cc \
		      xrc_lut.cxx lut_rgb2hsv_ov6620.cxx colormodel.cxx \
		      ../botball1/libimage/file_utils.cc \
		      ../botball1/libimage/hsv.cc
MSI_HDRS= colormodel.h
MSI_CFLAGS= -DUSE_HOST -Wall -I. -I../botball1/libimage 

generate_msimage.exe: $(MSI_SRCS) $(MSI_HDRS)
	$(NATIVE_CXX) $(MSI_CFLAGS) $(MSI_SRCS) -o $@

msimage:  generate_msimage.exe
	generate_msimage.exe
	ppm2gbasrc msimage.ppm

# DO NOT DELETE
