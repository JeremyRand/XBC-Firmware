CXX = arm-agb-elf-g++
SRCS=$(wildcard *.cxx)
OBJECTS=$(SRCS:.cxx=.o)
TARGET = librpc
CFLAGS = -O2 -DGBA -I../../../include

install: 
	-rm *.o
	$(MAKE) $(TARGET)-thumb.a 
	-rm *.o
	$(MAKE) $(TARGET)-thumb-interwork.a
	rm $(TARGET)temp.a
	rm *.o
	cp $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a ../../../lib
	cp *.h ../../../include

depend:
	makedepend -- $(CFLAGS) -- $(SRCS)

.PHONY: $(TARGET)-thumb.a $(TARGET)-thumb-interwork.a

$(TARGET)-thumb.a: 
	$(MAKE) $(TARGET)temp.a CFLAGS="-mthumb $(CFLAGS)"  
	cp $(TARGET)temp.a $(TARGET)-thumb.a
	
$(TARGET)-thumb-interwork.a: 
	$(MAKE) $(TARGET)temp.a CFLAGS="-mthumb -mthumb-interwork $(CFLAGS)"  
	cp $(TARGET)temp.a $(TARGET)-thumb-interwork.a

$(TARGET)temp.a: $(OBJECTS)
	arm-agb-elf-ar rvs $(TARGET)temp.a $(OBJECTS)

clean:
	-rm *.o
	-rm *.a

%.o: %.cxx
	$(CXX) -c $(CFLAGS) $<

# DO NOT DELETE
