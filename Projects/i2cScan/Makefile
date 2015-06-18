BOARD_TAG    = uno
ARDUINO_LIBS = Wire
NAME = i2cScan
# include ../../Arduino.mk
include ../../Arduino.mk

install:
	echo "$@"
	avrdude -c usbtiny -p m328p -e -U flash:w:./build-uno/$(NAME).hex:i 
