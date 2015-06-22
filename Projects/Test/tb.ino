#include <SoftwareSerial.h>
#include <NilRTOS.h>
#include "../ArduinoLibs/display.cpp"
#include "../ArduinoLibs/modbusRegisters.cpp"
display LED(5,4,3,1);

#define TRIP_BTN  6 // Port D bit 6
#define RESET_BTN 7 // Port D bit 7
#define MASK  0xc0 // top two bits set.

#define TRIP_LED 8
#define RESET_LED 9

SoftwareSerial setupSerial(10, 11);

modbusRegisters r;

void setup () {
    pinMode( TRIP_BTN,INPUT);
    pinMode( RESET_BTN,INPUT);

    pinMode(TRIP_LED, OUTPUT);    
    pinMode(RESET_LED, OUTPUT);  

    digitalWrite(TRIP_LED, LOW);
    digitalWrite(RESET_LED, LOW);

    DDRD = DDRD & 0x3f;
    Serial.begin(9600);
    setupSerial.begin(9600);

    LED.startup();
    LED.brightness(50);
    LED.clear();
    delay(1000);

    LED.writeDecNumber(1,0);
    LED.writeHexNumber(0x1,4);
    delay(1000);

    LED.writeDecNumber(12,0);
    LED.writeHexNumber(0x12,4);
    delay(1000);

    LED.writeDecNumber(123,0);
    LED.writeHexNumber(0x123,4);
    delay(1000);

    LED.writeDecNumber(1234,0);
    LED.writeHexNumber(0x123f,4);
    delay(1000);

    LED.writeDecNumber(0,0);
    LED.writeHexNumber(0x8000,4);
    delay(1000);

}


void loop () {
    static int count = 99;

    delay(500);
    LED.writeDecNumber(count,0);
    LED.writeHexNumber(count,4);
    count += 11;
}

