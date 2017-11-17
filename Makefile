all:
	$(CC) $(CFLAGS) -fPIC -c mx_led.c -o mx_led.o
	$(CC) $(CFLAGS) -shared -Wl,-soname,libmx_led.so.1 -o libmx_led.so.1.0.0 mx_led.o
	ln -sf libmx_led.so.1.0.0 libmx_led.so.1
	ln -sf libmx_led.so.1 libmx_led.so
	$(CC) $(CFLAGS) $(LDFLAGS) push_btn.c -DDEBUG -L. -lmx_led -ljson-c -o mx-pbtn
	$(STRIP) -s mx-pbtn

clean:
	rm -rf libmx* mx-pbtn *.o test
