#include <SoftwareSerial.h>

SoftwareSerial setupSerial(10,11);
char buffer[32];


void setup() {
    Serial.begin(9600);
    setupSerial.begin(9600);

    setupSerial.println("Ready");


}

void dump(uint8_t l) {
    for(int i = 0; i < l;i++ ) {
        setupSerial.print(String(i));
        setupSerial.print(":");
        setupSerial.println((uint8_t)buffer[i],HEX);
    }
}

void loop() {
    uint8_t len;
    uint8_t i=1;
    bool runFlag;

    runFlag=true;
    Serial.setTimeout(60000);

    len=Serial.readBytes(buffer,1);
    setupSerial.print("HERE ");
    setupSerial.print(len,HEX);
    setupSerial.print(" ");
    setupSerial.print(buffer[0],HEX);

    if(len>0) {
        i=1;
        setupSerial.println("HERE2");

        while( runFlag) {
            Serial.setTimeout(1);
            len=Serial.readBytes(&buffer[i++],1);
            if( len > 0 ) {
                setupSerial.println("HERE3");
            } else {
                setupSerial.println("HERE4");
                i--;
                dump(i);
                runFlag=false;
            }
        }
    }
}

