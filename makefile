CC=avr-gcc
SECTION=avr-size
DEVICE=atmega328p

all: main
	
main.o: main.c
	$(CC) -Wall -c main.c -o main.o -mmcu=$(DEVICE) 
	
printf-stdarg.o: printf-stdarg.c
	$(CC) -Wall -c printf-stdarg.c -o printf-stdarg.o -mmcu=$(DEVICE)

main: main.o printf-stdarg.o
	$(CC) main.o printf-stdarg.o -o vnh2sp30_demo.elf -mmcu=$(DEVICE)
	$(SECTION) vnh2sp30_demo.elf
	@echo Building Succeeded !

