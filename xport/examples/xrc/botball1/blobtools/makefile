######################################################################
# Shared 
BL_SOURCES=../../libxrc/blob.cxx ../../libxrc/xrc_lut.cxx \
	   ../../libxrc/image.cc ../../libxrc/host_image.cc \
	   ../../libxrc/lut_rgb2hsv_ov6620.cxx

BL_INCLUDES= ../../libxrc/blob.h ../../libxrc/xrc_lut.h ../../libxrc/hsv.h \
	     ../../libxrc/image.h

CFLAGS := -g -Wall -DHOST -I../../libxrc -I../libimage

all:  imageserver.exe ppm2gbasrc.exe install bloblog.exe

../libimage/libimage.a:
	$(MAKE) -C ../libimage libimage.a

######################################################################
# Bloblog
LOG_SOURCES=bloblog.cc sleep_us.c $(BL_SOURCES)
LOG_INCLUDES= sleep_us.h $(BL_INCLUDES)

bloblog.exe: $(LOG_SOURCES) $(LOG_INCLUDES) 
	g++ $(CFLAGS) $(LOG_SOURCES) ../libimage/libimage.a -o $@


######################################################################
# Imageserver
IS_SOURCES= imageserver.cc test_xrc_lut.cc tipc.cc string_printf.cc $(BL_SOURCES)
IS_INCLUDES= $(BL_INCLUDES)

imageserver.exe: $(IS_SOURCES) $(IS_INCLUDES) ../libimage/libimage.a
	g++ $(CFLAGS) $(IS_SOURCES) ../libimage/libimage.a -o $@

######################################################################
# ppm2gbasrc
PG_SOURCES= ppm2gbasrc.cc ../../libxrc/host_image.cc
PG_INCLUDES= ../../libxrc/image.h

ppm2gbasrc.exe: $(PG_SOURCES) $(PG_INCLUDES) ../libimage/libimage.a
	g++ $(CFLAGS) $(PG_SOURCES) ../libimage/libimage.a -o $@

install: ppm2gbasrc.exe
	cp ppm2gbasrc.exe ../../../../bin

%.o: %.c
	$(CXX) -c $(CFLAGS) $<

clean:
	-rm *.o
	-rm *.a
	-rm *.exe

run: imageserver
	cd data; ../imageserver.exe