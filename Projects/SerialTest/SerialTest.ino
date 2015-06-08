
int data;
char buffer[32];

void setup() {
    pinMode(13,OUTPUT);
    digitalWrite(13,LOW);
	Serial.begin(9600);
}

void loop() {
    if( Serial.available() > 0) {
        Serial.readBytes(buffer,8);

        for(int i=0;i<8;i++) {
            digitalWrite(13,HIGH);
            Serial.write( buffer[i] );
            digitalWrite(13,LOW);
        }
        Serial.flush();
    }

}
