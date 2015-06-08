# Arduino Make file. Refer to https://github.com/sudar/Arduino-Makefile

BOARD_TAG    = uno

include ../../Arduino.mk

install:
	echo "$@"
	avrdude -c usbtiny -p m328p -e -U flash:w:./build-uno/SerialTest.hex:i 
