/*
  Button
 
 Turns on and off a light emitting diode(LED) connected to digital  
 pin 13, when pressing a pushbutton attached to pin 2. 
 
 
 The circuit:
 * LED attached from pin 13 to ground 
 * pushbutton attached to pin 2 from +5V
 * 10K resistor attached to pin 2 from ground
 
 * Note: on most Arduinos there is already an LED on the board
 attached to pin 13.
 
 
 created 2005
 by DojoDave <http://www.0j0.org>
 modified 30 Aug 2011
 by Tom Igoe
 
 This example code is in the public domain.
 
 http://www.arduino.cc/en/Tutorial/Button
 */

#include <SoftwareSerial.h>
SoftwareSerial setupSerial(10, 11);

// constants won't change. They're used here to 
// set pin numbers:
#define TRIP_BTN  6 // Port D bit 6
#define RESET_BTN 7 // Port D bit 7

#define TRIP_LED  8 // Port B bit 0
#define RESET_LED 9 // Port B bit 1
#define MASK 0x03

// variables will change:
int buttonState = 0;         // variable for reading the pushbutton status

void setup() {
    setupSerial.begin(9600);
  // initialize the LED pin as an output:
  pinMode(TRIP_LED, OUTPUT);      
  pinMode(RESET_LED, OUTPUT);      
  // initialize the pushbutton pin as an input:
//  pinMode(buttonPin, INPUT_PULLUP);     
  pinMode(TRIP_BTN, INPUT);     
  pinMode(RESET_BTN, INPUT);     

       setupSerial.print(PORTD,HEX);
       setupSerial.println();
}

void loop(){
  // read the state of the pushbutton value:
   if (digitalRead(TRIP_BTN)) {
    digitalWrite(TRIP_LED, HIGH);  
   } else {
       setupSerial.print(PORTD,HEX);
       setupSerial.println();
       digitalWrite(TRIP_LED, LOW);  
   }

   if (digitalRead(RESET_BTN))
    digitalWrite(RESET_LED, HIGH);  
   else
    digitalWrite(RESET_LED, LOW);  
}
