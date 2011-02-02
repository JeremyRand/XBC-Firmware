HOST_OS_RAW := $(shell uname)

ifeq (CYGWIN,$(findstring CYGWIN,$(HOST_OS_RAW)))
   PLATFORM=win32
   TARGET=xpcomm.exe
   CXXFLAGS=-I$(PLATFORM)
else
   PLATFORM=linux
   TARGET=xpcomm
   CXXFLAGS=-I$(PLATFORM) -DLINUX
endif

COMPILE=$(wildcard *.cxx) $(wildcard $(PLATFORM)/*.cxx)
OBJECTS=$(COMPILE:.cxx=.o)

$(TARGET): $(OBJECTS)
	g++ -o $(TARGET) $(OBJECTS) -lncurses 

install: $(TARGET)
	cp $(TARGET) ../../bin

debug:
	echo $(HOST_OS_RAW)
	$(MAKE) $(TARGET) CXXFLAGS="-g $(CXXFLAGS)" -f $(CURDIR)/makefile

clean:
	-rm *.o $(PLATFORM)/*.o $(TARGET)

%.o: %.cxx
	$(CXX) -g -c $(CXXFLAGS) $< -o $@
