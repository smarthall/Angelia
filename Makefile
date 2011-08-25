

default: xbee_tapi.so

xbee_tapi.so: xbee_tapi.c
	gcc -shared -ldl -fPIC -o xbee_tapi.so xbee_tapi.c

clean:
	rm xbee_tapi.so

