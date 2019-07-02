# moxa-push-button

`moxa-push-button` is a C library for using SW push button.

## Build

This project use autotools as buildsystem. You can build this project by the following commands:

* If the build target architecture is x86_64

	```
	# ./autogen.sh --host=x86_64-linux-gnu --includedir=/usr/include/moxa --libdir=/usr/lib/x86_64-linux-gnu --sbindir=/sbin
	# make
	# make install
	```
* If the build target architecture is armhf

	```
	# ./autogen.sh --host=arm-linux-gnueabihf --includedir=/usr/include/moxa --libdir=/usr/lib/arm-linux-gnueabihf --sbindir=/sbin
	# make
	# make install
	```

The autogen script will execute ./configure and pass all the command-line
arguments to it.

## Documentation

[Config Example](/Config_Example.md)

[API Reference](/API_References.md)