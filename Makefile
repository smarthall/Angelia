

default: xbee_tapi.so

serial.o: serial.c serial.h
	gcc -fPIC -c serial.c -o serial.o

xbee.o: xbee.c xbee.h
	gcc -fPIC -c xbee.c -o xbee.o

xbee_tapi.o: xbee_tapi.c
	gcc -c -fPIC -o xbee_tapi.o xbee_tapi.c

xbee_tapi.so: xbee_tapi.o serial.o xbee.o
	gcc -shared -ldl -fPIC -o xbee_tapi.so xbee_tapi.o serial.o xbee.o

test: xbee_tapi.so
	LD_PRELOAD=./xbee_tapi.so avrdude -q -q -C/etc/avrdude/avrdude.conf -patmega328p -carduino -P/dev/ttyUSB0 -b115200 -D -Uflash:r:/dev/null:i

clean:
	rm xbee_tapi.so xbee_tapi.o serial.o xbee.o

