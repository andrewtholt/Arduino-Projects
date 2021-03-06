#define DEBUG false

#define TIMER_DELAY 2000
#define ILLEGAL_FUNCTION 1
#define ILLEGAL_DATA_ADDRESS 2

#define TRIP_BTN  6 // Port D bit 6
#define RESET_BTN 7 // Port D bit 7
#define MASK  0xc0 // top two bits set.

#define TRIP_LED 8 // PortB bit 0
#define RESET_LED 9 // Port B bit 1
#define RELAY 13
// 
// Read Only Registers
//
#define MODBUS_I_RMS 1
#define MODBUS_STATUS_REG 4 
// 
// Read/Write Registers
//
#define MODBUS_CONTROL_REG 0x10 

#include <NilRTOS.h>
#include <extEEPROM.h> 
extEEPROM eep(kbits_256, 1, 64);

#include <SoftwareSerial.h>
#include <NilSerial.h>
#define Serial NilSerial

#include "../ArduinoLibs/display.cpp"
display LED(5,4,3,1);


SEMAPHORE_DECL(modbusSem,1);
const int analogInPin = A0;  // Analog input pin that the sensor is attached to


static unsigned char auchCRCHi[] = { 
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1,
    0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
    0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
    0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80,
    0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
    0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40
};

static char     auchCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
    0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E,
    0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA,
    0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17,
    0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33,
    0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D,
    0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9,
    0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24,
    0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0,
    0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8,
    0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD,
    0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1,
    0x71, 0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97,
    0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B,
    0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E,
    0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82,
    0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40
};

SoftwareSerial setupSerial(10, 11);

uint16_t calcCRC(char *address, uint8_t len) {
    uint8_t  *puchMsg;
    uint16_t usDataLen;
    uint16_t result;

    unsigned char   uchCRCHi = 0xFF;    /* high byte of CRC
                                         * initialized   */
    unsigned char   uchCRCLo = 0xFF;    /* low byte of CRC
                                         * initialized   */
    unsigned        uIndex; /* will index into CRC lookup table   */
    int count=0;

    puchMsg = (uint8_t *)address;
    usDataLen=len;

    while (usDataLen--) {
        uIndex = uchCRCLo ^ *puchMsg++; /* calculate the CRC   */
        uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
        uchCRCHi = auchCRCLo[uIndex];
    }

    result = (uchCRCHi << 8) | (uchCRCLo & 0xff);
    return(result);
}


void debug(String msg) {
    nilSysLock();
    setupSerial.println(msg);
    nilSysUnlock();
}

void dump(char *addr,uint8_t len) {

    setupSerial.println(String(len));

    for(int i=0; i < len;i++) {
        setupSerial.print(String(i));
        setupSerial.print(":");
        setupSerial.println((uint8_t)addr[i],HEX);
    }
}
#include "../ArduinoLibs/modbusRegisters.cpp"
modbusRegisters r;
#include "../ArduinoLibs/modbus.cpp"

byte getReply() {
    while( setupSerial.available() == 0 );
    return(setupSerial.read());
}

int getNumber(int digits) {
    uint8_t idx=0;

    char buffer[digits+1];
    char n;
    uint8_t runFlag=TRUE;

    while(runFlag == TRUE ) { 
        while (setupSerial.available() > 0) {
            n = setupSerial.read();
            if(isDigit( n )) {
                setupSerial.print( n );
                buffer[idx++] = n;
            }   
            if( n == 0x08 ) { // BS
                idx--;
                if(idx < 0) {
                    idx=0;
                }   
                setupSerial.print("\010 \010");
            } else if ( n == '\n' ) { 
                runFlag = FALSE;
            } else if ( n == '\r' ) { 
                runFlag = FALSE;
            } else if (idx > digits ) { 
                buffer[idx]='\0';
                runFlag = FALSE;
            }   
        }   
    }   
    return(atoi(buffer));
}

modbus *m;

#include <../ArduinoLibs/setup.cpp>
// 
// START
//
void setup() {
    byte rtu;
    int data;

    bool reconfig=false;

    uint8_t eepStatus = eep.begin(twiClock400kHz); 
    uint8_t eedata[4];

    pinMode( TRIP_BTN,INPUT);
    pinMode( RESET_BTN,INPUT);

    pinMode(TRIP_LED, OUTPUT);    
    pinMode(RESET_LED, OUTPUT);  
    pinMode(RELAY, OUTPUT);  

    digitalWrite(TRIP_LED, LOW);
    digitalWrite(RESET_LED, LOW);
    digitalWrite(RELAY, LOW);

    Serial.begin(9600);
    setupSerial.begin(9600);
    delay(100);
    setupSerial.listen();
    setupSerial.println("Ready");

    LED.startup();
    LED.brightness(50);
    LED.clear();

    reconfig = (digitalRead(TRIP_BTN == false) & (digitalRead(RESET_BTN) == false));

    eep.read(0,eedata,1);
    rtu = eedata[0];
    // rtu = 1;

    LED.writeDecNumber(r.getRegister(0),0);
    LED.writeHexNumber(rtu,4);

    if ( (0xff == rtu) || (0x00 == rtu)  || reconfig) {
        bool exitFlag = false;
        byte reply;

        LED.clear();
        delay(250);
        LED.writeHexNumber(rtu,4);
        delay(250);
        LED.clear();
        delay(250);
        LED.writeHexNumber(rtu,4);

        LED.clear();
        LED.writeHexNumber(rtu,4);

        setupMenu();
        eep.read(0,eedata,1);
        rtu = eedata[0];
        /*
           setupSerial.println();
           setupSerial.println("No modbus address set, please enter one now.");
           setupSerial.print("Modbus Address>");

           rtu=getNumber(3);

           setupSerial.println();
           setupSerial.print("Modbus Address=");

           setupSerial.println(rtu);

           setupSerial.print("Is this correct [y/N] ? ");

           reply = getReply();
           setupSerial.println();
           setupSerial.println(reply);

           if ( 'y' == reply ) {
           eedata[0]=rtu;
           eep.write(0,eedata,1);
           delay(500);
           LED.clear();
           if( 0xff == rtu ) {
           setupSerial.println("No Change");
           exitFlag = false;
           } else {
           exitFlag=true;
           }
           }
           */
    }

    LED.clear();
    LED.writeHexNumber(rtu,4);
    delay(1000);

    m=new modbus(rtu);

    digitalWrite(TRIP_LED, HIGH);
    digitalWrite(RESET_LED, HIGH);

    nilSysBegin();
}

NIL_THREAD(thModBus,arg) {
    uint8_t len;

    while( true ) {
        nilThdSleepMicroseconds(500);
        len = m->getPacket();

        if( len > 0 ) {
            m->processPacket(len);
        }
    }
}

NIL_THREAD(thSensor,arg) {
    uint16_t sensorValue=0;
    int value=0;
    int total=0;
    uint8_t count=0;
    uint8_t outputValue;

    while(true) {
        sensorValue = analogRead( analogInPin );
        value = map(sensorValue, 0, 1023, -20000, 20000);
        total += sq(value);

        if( 8 == count ) {
            outputValue = sqrt(total / 8);
            r.setRegister(MODBUS_I_RMS, outputValue);
            count = 0;
            total = 0 ;
        } else {
            count++;
        }

        nilThdSleepMicroseconds(TIMER_DELAY);
    }
}

NIL_THREAD(thUI,arg) {
    uint8_t buttons;
    uint8_t br;
    uint8_t idx=0;

    bool fault = false;
    bool released = false;
    bool trip = false;
    bool reset = false;
    uint8_t count = 0;

    r.setRegister(MODBUS_STATUS_REG,0);

    digitalWrite(TRIP_LED, HIGH);
    digitalWrite(RESET_LED, HIGH);
    r.setRegisterBit(MODBUS_CONTROL_REG,15);

    while(true) {
        trip =  !digitalRead(TRIP_BTN);
        reset = !digitalRead(RESET_BTN);

        fault = trip & reset;

        if(trip == true) {
            r.setRegisterBit(MODBUS_CONTROL_REG,15);
        }
        if(reset == true) {
            r.setRegisterBit(MODBUS_CONTROL_REG,14);
        }
        // 
        // Should this be in a thread of it's own ?
        //
        if( r.getRegisterBit(MODBUS_CONTROL_REG,15)) {
            digitalWrite(RESET_LED, HIGH);
            digitalWrite(TRIP_LED, LOW);

            digitalWrite(RELAY, LOW);

            r.clrRegisterBit(MODBUS_CONTROL_REG,15);
            r.clrRegisterBit(MODBUS_STATUS_REG,15);
        } 

        if( r.getRegisterBit(MODBUS_CONTROL_REG,14)) {
            digitalWrite(RESET_LED, LOW);
            digitalWrite(TRIP_LED, HIGH);

            digitalWrite(RELAY, HIGH);

            r.clrRegisterBit(MODBUS_CONTROL_REG,14);
            r.setRegisterBit(MODBUS_STATUS_REG,15);
        }
        // 
        // END
        //
        if( (count % 50) == 0 ) {
            LED.writeDecNumber(r.getRegister(MODBUS_I_RMS),0);
        }

        nilThdSleepMilliseconds(10);
        count++;
    }
}

NIL_WORKING_AREA(waModBus, 64);
NIL_WORKING_AREA(waSensor, 64);
NIL_WORKING_AREA(waUI, 64);


void loop() {
    nilThdDelayMilliseconds(1000);

    /*
       uint8_t len = m.getPacket();

       if( len > 0 ) {

       m.processPacket(len);

       if( String(m.forMe()) ) {
       if( m.validFunctionCode() ) {
       debug("OK");
       }
       }
       }
       */
}

    NIL_THREADS_TABLE_BEGIN()
    NIL_THREADS_TABLE_ENTRY(NULL, thSensor, NULL, waSensor, sizeof(waSensor))
    NIL_THREADS_TABLE_ENTRY(NULL, thModBus, NULL, waModBus, sizeof(waModBus))
    NIL_THREADS_TABLE_ENTRY(NULL, thUI, NULL, waUI, sizeof(waUI))
NIL_THREADS_TABLE_END()
