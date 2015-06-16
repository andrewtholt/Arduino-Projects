#include <stdint.h>

#define RR 0x03 // Read Registers
#define WR 0x10 // Write Registers

class modbus {
    
private:
    SEMAPHORE_DECL(modbusCommsSem,1);

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
        
        do { // loop until we timeout
                Serial.setTimeout(4);
                data = Serial.readBytes(&packet[0],1);
                
                if( data == 0) { // we timed out and nothing was read.
                    exitFlag=true;
                }
                
        } while(!exitFlag);
    }
    
    void sendPacket(uint8_t *address,int len) {

        uint16_t crc = calcCRC((char *)address,len);
        address[len] = crc & 0xff;
        address[len+1] = (crc >> 8) & 0xff;

        Serial.write(address, (len+2));

    }

    uint16_t toRegister(uint8_t hi, uint8_t lo) {
        uint16_t res;

        res = (hi << 8) | (lo & 0xff);

        return(res);
    }
    
public:

    modbus(uint8_t id) {
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
        const int timeout=3;

        memset(packet,0,32);
        runFlag=true;
        
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
            nilSemWait(&modbusCommsSem);
            while( runFlag ) {
                Serial.setTimeout(timeout);
                len=Serial.readBytes(&packet[i++],1);

                if( len == 0 ) {
                    ret = i-1;
                    runFlag = false;
                }
            }
            nilSemSignal(&modbusCommsSem);
//            dump(&packet[0],ret);
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

        // 
        // First check if the address and count are legal
        //

        memset(reply,0,32);

        address = (addressHi << 8) | (addressLo & 0xff);
        regCount = (regCountHi << 8) | (regCountLo & 0xff);

        if( (address + regCount) > 32) {
            reply[0] = packet[0];
            reply[1] = packet[1] | 0x80;
            reply[2] = ILLEGAL_DATA_ADDRESS;
            sendPacket(reply,3);
            return 0;
        }

        reply[0]=rtu;
        reply[1]=func;
        reply[2]= (uint8_t) regCount * 2; // count of bytes to follow

        for(uint8_t idx=0;idx < regCount; idx++) {
            tmp = r.getRegister(address+idx);

            reply[3+(2*idx)] = (tmp >> 8) & 0xff;
            reply[4+(2*idx)] = tmp  & 0xff;
        }

        sendPacket(reply,(3+(2*regCount)));
    }

    void unimplementedFunction() {
        uint8_t reply[8];

        memset(reply,0,sizeof(reply));

        reply[0] = packet[0];
        reply[1] = packet[1] | 0x80;
        reply[2] = ILLEGAL_FUNCTION;

        sendPacket(reply,3);

    }

    void processWriteRegisters() {
        uint8_t functionCode;
        uint16_t address;
        uint16_t regCount;
        uint8_t reply[32];
        uint8_t byteCount;
        uint16_t data;
        bool exception=false;

        memset(reply,0,sizeof(reply));

        address = toRegister(packet[2],packet[3]);
        regCount = toRegister(packet[4],packet[5]);


        exception= ((address + regCount) > SIZE);
        exception = exception || (( address + regCount ) < 0x10);

//        if( (address + regCount) > SIZE) {
        if( exception ) {
            reply[0] = packet[0];
            reply[1] = packet[1] | 0x80;
            reply[2] = ILLEGAL_DATA_ADDRESS;
            sendPacket(reply,3);
            return ;
        }
        byteCount = packet[6];

        for(uint8_t idx=0;idx < byteCount;idx+=2) {
            data = toRegister( packet[idx+7],packet[idx+8] );
            r.setRegister( (address +(idx/2)) , data);
        }

        reply[0] = packet[0];  // RTU
        reply[1] = packet[1];  // Function code
        reply[2] = packet[2];  // Start Address hi
        reply[3] = packet[3];  // Start Address lo
        reply[4] = packet[4];  // Start Address hi
        reply[5] = packet[5];  // Start Address lo

        sendPacket(reply,6);
        return ;

    }


    void processPacket(uint8_t len) {
        uint8_t functionCode;
        uint16_t crc=calcCRC(&packet[0],len);

        if( 0 != crc ) {
            return;
        }

        functionCode=packet[1];

        switch(functionCode) {
            case RR:
                processReadRegisters();
                break;
            case WR:
                processWriteRegisters();
                break;
            default:
                unimplementedFunction();
        }
    }
};

