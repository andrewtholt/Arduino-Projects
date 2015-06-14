#define DEBUG true

#include <SoftwareSerial.h>
#include "../ArduinoLibs/modbus.cpp"

SoftwareSerial setupSerial(10, 11);

void setup() {
    
    Serial.begin(9600);
    setupSerial.begin(9600);
    setupSerial.println("Ready");
    modbus m(1,setupSerial);

}

void loop() {
  // put your main code here, to run repeatedly: 
  
}
