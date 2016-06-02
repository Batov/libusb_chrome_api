MSPDebug (alpha version port to chrome USB API)
========

MSPDebug is a free debugger for use with MSP430 MCUs. It supports
FET430UIF, eZ430, RF2500 and Olimex MSP430-JTAG-TINY programmers, as
well as many other compatible devices. 
http://dlbeer.co.nz/mspdebug/


Brief
--------
 - Libusb platform layer based on Chrome USB API;
 - Emscripten as sourse-to-sourse translation;
 - MSPdebug has been ported from C-code to JS-code (to Chrome extension environment);
 

Libusb 
--------
Folder: 
	libusb/chrome_usb.c - platform layer source;
	libusb/chrome_usb.h - platform layer header (like linux.h for linux environment);

	libusb/usb.h	    - common endpoint of platform-independent libusb part;

	and some of system source and headers...

This is an alpha version of libusb port. There is not standart libusb make algorithm. Just source and headers =)


Translation 
--------
Source-to-source port linux application (with libusb) to Chrome.

1. Install Emscripten - [Tutorial](http://kripken.github.io/emscripten-site/docs/getting_started/index.html)
2. Edit your application's makefile:
	- remove `-lusb` key;
	- add include path to libusb folder (like '-Ilibusb')
	- add next flags to your LDFLAGS make variable
		`LDFLAGS ?=  --preload-file firmware -s ASSERTIONS=1 -s EMTERPRETIFY=1 -s EMTERPRETIFY_ASYNC=1 -s EXPORTED_FUNCTIONS="['_main','_usb_os_find_busses_cb','_usb_os_find_devices_cb','_usb_os_open_cb','_ready_to_cont']" $(ASYNCFUNC)`
		where `$(ASYNCFUNC)` - list of functions for interpreter. You can stay it is empty. Might be slow, but it should work! Might be =)
	- rename binary (e.g mspdebug -> mspdebug.html)
	- add precompiled files (e.g. devices firmwares) http://kripken.github.io/emscripten-site/docs/getting_started/Tutorial.html#using-files
3. Edit your application's makefile for emscripten - http://kripken.github.io/emscripten-site/docs/porting/index.html




Mspdebug 
--------
Base example of libusb port.
1. To make it add emscripten to your PATH variable
2. `make WITHOUT_READLINE=1`
3. The result of it is mspdebug.js (JS source-to-source port)
4. Add console call arguments to end of mspdebug (e.g. `Module.arguments=["rf2500","prog firmware/slow_blink.elf"];` , where firmware/slow_blink.elf is path to your precompiled firmware)
5. And need to wrap it Chrome Application
6. Run it and permit access to your TI device




