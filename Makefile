

default: xbee-info

xbee-info: xbee-info.c xbee-info.h serial.h serial.o xbee.o xbee.h
	gcc -g xbee-info.c serial.o xbee.o -o xbee-info

serial.o: serial.c serial.h
	gcc -g -c serial.c -o serial.o

xbee.o: xbee.c xbee.h
	gcc -g -c xbee.c -o xbee.o

clean:
	rm xbee-info

