

default: xbee_tapi.so

serial.o: serial.c serial.h
	gcc -fPIC -c serial.c -o serial.o

xbee_tapi.so: xbee_tapi.c serial.o
	gcc -shared -ldl -fPIC -o xbee_tapi.so xbee_tapi.c serial.o

test: xbee_tapi.so
	LD_PRELOAD=./xbee_tapi.so avrdude -q -q -C/etc/avrdude/avrdude.conf -patmega328p -carduino -P/dev/ttyACM0 -b115200 -D -Uflash:r:/dev/null:i

clean:
	rm xbee_tapi.so serial.o

