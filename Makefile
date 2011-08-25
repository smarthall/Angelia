

default: xbee_tapi.so

xbee_tapi.so: xbee_tapi.c
	gcc -shared -ldl -fPIC -o xbee_tapi.so xbee_tapi.c

test: xbee_tapi.so
	LD_PRELOAD=./xbee_tapi.so avrdude -q -q -C/etc/avrdude/avrdude.conf -patmega328p -carduino -P/dev/ttyACM0 -b115200 -D -Uflash:r:/dev/null:i

clean:
	rm xbee_tapi.so

