BOARD_TAG    = uno
ARDUINO_LIBS = SoftwareSerial NilRTOS NilTimer1
MY_TARGET = build-uno/ProtectionRelay.hex

# include ../../Arduino.mk
# build-uno/ProtectionRelay.hex:
include ../../Arduino.mk

install:
	echo "$@"
	avrdude -c usbtiny -p m328p -e -U flash:w:./build-uno/ProtectionRelay.hex:i 
