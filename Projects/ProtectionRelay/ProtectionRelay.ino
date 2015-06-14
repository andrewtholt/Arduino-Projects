#define DEBUG true

#include <SoftwareSerial.h>

SoftwareSerial setupSerial(10, 11);

void debug(String msg) {
    setupSerial.println(msg);
}
#include "../ArduinoLibs/modbus.cpp"


void setup() {
    
    Serial.begin(9600);
    setupSerial.begin(9600);
    setupSerial.println("Ready");
    modbus m(1,setupSerial);
    m.getPacket();

}

void loop() {
  // put your main code here, to run repeatedly: 
  
}

