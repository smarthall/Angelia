

default: xbee-info

xbee-info: xbee-info.c xbee-info.h
	gcc -g xbee-info.c -o xbee-info

clean:
	rm xbee-info

