#include <stdint.h>

#define RR 0x03 // Read Registers
#define WR 0x10 // Write Registers

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
    
    void waitForTheGap() {
        uint8_t data;
        bool exitFlag=false;
        
        do { // loop until we timeout
                Serial.setTimeout(4);
                data = Serial.readBytes(&packet[0],1);
                
                if( data == 0) { // we timed out and nothing was read.
                    exitFlag=true;
                }
                
        } while(!exitFlag);
    }
    
    bool waitForMe() {
        bool exitFlag = false;
        uint8_t data;
        
        
        do {
            Serial.setTimeout(60000); // a whole minute
            data = Serial.readBytes(&packet[0],1);
            
            if ( 0 == data ) { // we timedout
                        exitFlag = false;
            } else {
                exitFlag = true;
            }
            
        } while( !exitFlag );
        // 
        // byte 0 in packet is the rtu id, so is this for me ?
        //
        // if not go to the top and wait for a gap.
        // if it is go get the rest:
        //
        return (bool) ( packet[0] == rtu );
    }
    
    // 
    // Is the code valid.
    //
    bool validForFunctionCode() {
        return((packet[2] == RR ) || (packet[2] == WR));
    }
    
public:
    
    modbus(uint8_t id, SoftwareSerial debug) {
        
        debug.println("modbus created");
        rtu = id;
    }
    
    uint8_t getPacket() {
        uint8_t len; // including CRC
        uint8_t data;
        bool exitFlag = false;
        bool forMe = false;
        
        do {
            debug.println("Waiting for the gap ...");
            waitForTheGap();
            // 
            // OK, interpacket gap found 
            // Now wait a long time for a byte.
            //
        } while (!waitForMe());
        debug.println("... done");
        // 
        // So there's been a gap, followed by a byte that has
        // matched my rtu id.
        // Next thing is the function code:
        //
        Serial.setTimeout(2);
        len = Serial.readBytes(&packet[1],1);
        
        if(len > 0 ) {
            if( validForFunctionCode() ) {
                switch(packet[2]) {
                    case RR:
                        debug.println("RR");
                        break;
                    case WR:
                        break;
                }
            } else {
                // Construct an invalid function error packet
            }
            
        }
        
        return len;
    }
    
};

