BOARD_TAG    = uno
ARDUINO_LIBS =
MY_TARGET = build-uno/ProtectionRelay.hex

# include ../../Arduino.mk
build-uno/ProtectionRelay.hex:
include ../../Arduino.mk

install:
	echo "$@"
	cd build-uno
	avrdude -c usbtiny -p m328p -e -U flash:w:ProtectionRelay.hex:i 
