BOARD_TAG    = uno
ARDUINO_LIBS = LedControl SoftwareSerial NilRTOS
NAME = Test
# include ../../Arduino.mk
include ../../Arduino.mk

install:
	echo "$(NAME)"
	avrdude -c usbtiny -p m328p -e -U flash:w:./build-uno/$(NAME).hex:i 
