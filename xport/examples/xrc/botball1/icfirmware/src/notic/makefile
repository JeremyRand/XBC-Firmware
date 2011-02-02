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

TARGET = notic

all: install

includes:
	find ../ \( -iname '*.inl' -o -iname '*.h' \) -exec cp '{}' ../../include \;


include makefile.library

