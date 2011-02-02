HOST_OS_RAW := $(shell uname)

ifeq (CYGWIN,$(findstring CYGWIN,$(HOST_OS_RAW)))
   PLATFORM=win32
   TARGET=Debug/upload.exe
else
   PLATFORM=linux
   TARGET=upload
endif

install: $(TARGET)
	cp $(TARGET) ../../../../bin
