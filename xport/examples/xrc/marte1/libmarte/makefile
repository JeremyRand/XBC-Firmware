CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
CFLAGS = -O2 -Wall -I../../../../include -mthumb

HEADERS=$(wildcard *.h)
COMPILE=$(wildcard *.cxx)
T_OBJECTS=$(COMPILE:.cxx=-t.o)
I_OBJECTS=$(COMPILE:.cxx=-i.o)
TARGET = libmarte

install: $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a
	cp $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a ../../../../lib
	cp *.h ../../../../include

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

.PHONY: $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a

$(TARGET)-thumb.a: $(T_OBJECTS) $(HEADERS)
	arm-agb-elf-ar rvs $(TARGET)-thumb.a $(T_OBJECTS)

$(TARGET)-thumb-interwork.a: $(I_OBJECTS) $(HEADERS)
	arm-agb-elf-ar rvs $(TARGET)-thumb-interwork.a $(I_OBJECTS)

clean:
	-rm *.o
	-rm *.a

%-t.o: %.cxx
	$(CXX) -c $(CFLAGS) -o $@ $< 

%-i.o: %.cxx
	$(CXX) -c $(CFLAGS) -mthumb-interwork -o $@ $<


# DO NOT DELETE
