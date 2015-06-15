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

        uint16_t tmp = r.getRegister(0);
        debug("Register 0 is");
        debug(String(tmp));
        debug("=============");
        
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
   
    void sendPacket(uint8_t *address,int len) {

        uint16_t crc = calcCRC((char *)address,len);
        address[len] = crc & 0xff;
        address[len+1] = (crc >> 8) & 0xff;

        Serial.write(address, (len+2));

    }
    
    modbusRegisters r;
public:

    modbus(uint8_t id, SoftwareSerial d) {
        
        debug("ModBus created.");
        rtu = id;
    
    }
    // 
    // Is the code valid.
    //
    bool validFunctionCode() {
        return((packet[1] == RR ) || (packet[1] == WR));
    }
    
    bool forMe() {
        return( packet[0] == rtu );
    }

    uint8_t getPacket() {
        uint8_t len;
        uint8_t ret;
        uint8_t i=1;
        bool runFlag;

        memset(packet,0,32);
        runFlag=true;
        debug("Waiting for the gap ...");
        waitForTheGap();
        // 
        // OK, interpacket gap found 
        // Now wait a long time for a byte.
        //
        Serial.setTimeout(60000); // wait a minute ...

        len=Serial.readBytes(packet,1); // 1st 4 bytes are always the same ...
                                        // rtu,func, address hi, address lo
        if (len > 0 ) { // possible len == 4
            i=1;

            // Put short delay here.
            debug("Here ");
            while( runFlag ) {
                Serial.setTimeout(2);
                len=Serial.readBytes(&packet[i++],1);

                if( len == 0 ) {
                    ret = i-1;
                    runFlag = false;
                }
            }
            dump(&packet[0],ret);
        } else {
            ret=0;
        }
        return(ret);
    }
      
    uint8_t processReadRegisters() {
        uint16_t address;
        uint16_t regCount;
        uint16_t tmp;

        uint8_t reply[32];

        uint8_t rtuId = packet[0];
        uint8_t func = packet[1];
        uint8_t addressHi = packet[2];
        uint8_t addressLo = packet[3];

        uint8_t regCountHi = packet[4];
        uint8_t regCountLo = packet[5];

        memset(reply,0,32);

        address = (addressHi << 8) | (addressLo & 0xff);
        regCount = (regCountHi << 8) | (regCountLo & 0xff);

        reply[0]=rtu;
        reply[1]=func;
        reply[2]= (uint8_t) regCount * 2; // count of bytes to follow

        for(uint8_t idx=0;idx < regCount; idx++) {
            tmp = r.getRegister(address+idx);
            reply[3+(2*idx)] = (tmp >> 8) & 0xff;
            reply[4+(2*idx)] = tmp  & 0xff;
        }

        sendPacket(reply,(3+(2*regCount)));

        for(uint8_t idx=0;idx<10;idx++) {
            debug(String(reply[idx]));
        }

    }
    void processPacket(uint8_t len) {
        uint8_t functionCode;

        uint16_t crc=calcCRC(&packet[0],len);

        if( 0 == crc ) {
            debug("CRC OK");
        } else {
            debug("CRC Error");
            return;
        }

        functionCode=packet[1];

        switch(functionCode) {
            case RR:
                debug("Read Registers");
                processReadRegisters();
                break;
            case WR:
                break;

        }
    }
};

