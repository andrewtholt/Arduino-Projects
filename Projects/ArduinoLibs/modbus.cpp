#include <stdint.h>
#include <SoftwareSerial.h>

class modbus {

    private:
        char packet[32];
        uint8_t rtu;

        void addCRC() {
        }
        uint8_t getByte() {
            uint8_t data;
            bool runFlag;

            runFlag = true;
            do {
                data = Serial.read();
                if( data >= 0) {
                    runFlag = false;
                }

            } while( runFlag );

            return(data);

        }

    public:
        modbus(uint8_t id, int speed) {

            rtu = id;

            if(0 == speed)
                speed = 9600;

            Serial.begin(speed);

            if(DEBUG) {
                SoftwareSerial setupSerial(10, 11);
            }
        }
        uint8_t getPacket() {
            uint8_t len; // including CRC
            uint8_t data;
            bool exitFlag = false;
            bool forMe = false;

            do {
                do { // loop until we timeout
                    Serial.setTimeout(4);
                    data = Serial.readBytes(&packet[0],1);

                    if( data == 0) { // we timed out and nothing was read.
                        exitFlag=true;
                    }

                } while(!exitFlag);
                // 
                // OK, interpacket gap found 
                // Now wait a long time for a byte.
                //
                exitFlag = false;
                do {
                    Serial.setTimeout(60000); // a whole minute
                    data = Serial.readBytes(&packet[0],1);

                    if ( 0 == data ) // we timedout
                        exitFlag = false;
                    else
                        exitFlag = true;
                } while( !exitFlag );
                // 
                // byte 0 in packet is the rtu id, so is this for me ?
                //
                // if not go to the top and wait for a gap.
                // if it is go get the rest:
                //
                if( packet[0] == rtu )
                    forMe = true;
                else
                    forMe = false;

            } while (!forMe);
            // 
            // So there's been a gap, followed by a byte that was
            // matched my rtu id.
            // Next thing is the function code:
            //
            Serial.setTimeout(2);
            len = Serial.readBytes(&packet[1],1);

            if(len > 0 ) {

            }

            return len;
        }

};

