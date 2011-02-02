ROOT =../../devkitadv/arm-agb-elf/lib
CC=arm-agb-elf-gcc

all: 
	$(CC) -c crt0.S
	cp crt0.o $(ROOT)
	$(CC) -mthumb-interwork -c crt0.S
	cp crt0.o $(ROOT)/interwork
	$(CC) -mthumb -c crt0.S
	cp crt0.o $(ROOT)/thumb
	$(CC) -mthumb -mthumb-interwork -c crt0.S
	cp crt0.o $(ROOT)/thumb/interwork
	$(CC) -c -mthumb agb-exit.c
	arm-agb-elf-ar r $(ROOT)/thumb/libc.a agb-exit.o


