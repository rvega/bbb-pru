all: host pru.bin

#####################################################################
# Host C program

CFLAGS += -Wall -g -O2 -mtune=cortex-a8 -march=armv7-a
LDFLAGS += -lprussdrv


%.o: %.c 
	gcc $(CFLAGS) -c -o $@ $<

host: host.o
	gcc $(CFLAGS) -o host host.o $(LDFLAGS)

#####################################################################
# PRU Assembly program

pru.bin: pru.asm
	pasm -V3 -b pru.asm

#####################################################################
# Util
.PHONY: clean
clean:
	rm *.o
	rm *.bin
	rm host
