BOARD_TAG    = uno
ARDUINO_LIBS = Wire extEEPROM Streaming
NAME = extEEPROM

include ../../Arduino.mk

install:
	echo "$(NAME)"
	avrdude -c usbtiny -p m328p -e -U flash:w:./build-uno/$(NAME).hex:i 
