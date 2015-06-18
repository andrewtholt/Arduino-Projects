BOARD_TAG    = uno
ARDUINO_LIBS = Wire I2C_EEPROM
NAME = I2C_eeprom_test

include ../../Arduino.mk

install:
	echo "$@"
	avrdude -c usbtiny -p m328p -e -U flash:w:./build-uno/$(NAME).hex:i 
