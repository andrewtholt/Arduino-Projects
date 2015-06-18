
#include "../ArduinoLibs/display.cpp"

display LED(5,4,3,1);
void setup() {

    LED.startup();
    LED.brightness(50);
    LED.clear();
}

void loop() {
    static uint8_t count=0;

    LED.writeDecNumber(count,0);
    LED.writeHexNumber(count,4);

    count++;

    if( count > 32 ) {
        count = 0;
        LED.clear();
    }
    delay(500);
}
