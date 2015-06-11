#include <NilRTOS.h>

// Use tiny unbuffered NilRTOS NilSerial library.
#include <NilSerial.h>
#include <NilFIFO.h>

#include <SoftwareSerial.h>

#define BUFFSIZE 32
#define REGISTERS 0x20
NilFIFO<int, 10> fifo;

uint8_t modbusBuffer[BUFFSIZE];
uint16_t modbusRegisters[REGISTERS];
#define V_RMS 0 // Reg 0 is RMS
#define RTU 0x11 // RTU id


SEMAPHORE_DECL(modbusSem,1);

// Macro to redefine Serial as NilSerial to save RAM.
// Remove definition to use standard Arduino Serial.

#define Serial NilSerial
#define RR 0x03
// ModBus exception Codes
//
#define ILLEGAL_FUNCTION 0x01
#define ILLEGAL_DATA_ADDRESS 0x02
#define ILLEGAL_DATA_VALUE 0x03

#include <NilTimer1.h>
const int analogInPin = A0;  // Analog input pin that the sensor is attached to
const int TRIP = 2;
const int RESET = 3;

#define TIMER_DELAY 2000
// #define TIMER_DELAY 400000

// Global variables.  These will need a seamphore/mutex to serialise access.
//
// Initialise ModBus CRC tables.
//

// static unsigned char auchCRCHi[] = {
static uint8_t auchCRCHi[] = {
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

static uint8_t auchCRCLo[] = {0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
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

/*
void writeModbusRegister(int reg, uint16_t v) {
    nilSemWait(&modbusSem);
    modbusRegisters[reg] = v;
    nilSemSignal(&modbusSem);
}
*/

//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread1, 64);
NIL_WORKING_AREA(waModBus, 64);

SoftwareSerial setupSerial(10, 11); // RX, TX

uint16_t calcCRC(uint8_t *data,int len) {
    uint8_t  *puchMsg;
    uint16_t usDataLen;
    unsigned char   uchCRCHi = 0xFF;    /* high byte of CRC
                                         * initialized   */
    unsigned char   uchCRCLo = 0xFF;    /* low byte of CRC
                                         * initialized   */
    unsigned        uIndex; /* will index into CRC lookup table   */

    puchMsg = (uint8_t *)data;
    usDataLen = len;
    while (usDataLen--) {
        uIndex = uchCRCLo ^ *puchMsg++; /* calculate the CRC   */
        uchCRCLo = uchCRCHi ^ auchCRCHi[uIndex];
        uchCRCHi = auchCRCLo[uIndex];
    }

    return( uchCRCHi << 8 | uchCRCLo );
}
/*
 *
 * Screen control sequences
 */
void home() {
    setupSerial.print("\033[H");
}

void ces() { // clear to end of screen
    setupSerial.print("\033[J");
}

void cls() {
    home();
    ces();
}

// x=line
// y=column
//
void move(int x, int y) {
    setupSerial.print("\033[");
    setupSerial.print(x);
    setupSerial.print(";");
    setupSerial.print(y);
    setupSerial.print("H");
}

/*
 * ModBus.
 */

uint8_t ModBusPause(unsigned int time) {
    //    uint32_t start = micros();
    int start = micros();
    uint8_t loopFlag = TRUE;

    memset(&modbusBuffer,0,BUFFSIZE);
    do {
        if( Serial.available() ) {
            if( (micros() - start) > time) {
                loopFlag = FALSE;
            } else {
                Serial.read();
                start = micros();
                nilThdSleep(1);
            }
        }

    } while (loopFlag) ;
    return(Serial.read());
}

uint8_t getByte() {

    int data;
    uint8_t runFlag=TRUE;

    do {
        data=Serial.read();

        if( data >= 0) {
            runFlag = FALSE;
        }
        nilThdSleepMicroseconds(500);
    } while(runFlag == TRUE);

    return(data);
}

void sendPacket(uint8_t *packet) {
    uint8_t idx;
    uint8_t len;
    uint16_t res;

    // Is this a data packet, or an exception
    // If an exception then bit 7 of [01] will 
    // be set.

    if( packet[1] & 0x80) { // Exception
        len = 3;
    } else { // OK
        // Get length and add 3 to point to the CRC.
        len = packet[2] + 3;
    }
    res=calcCRC(&modbusBuffer[0],len);

    packet[len] = (res >> 8) & 0xff;
    packet[len+1] = res ;
    len +=2;
        nilSysLock();
        setupSerial.println(len,HEX);

        nilSysUnlock();

    for(idx=0;idx < len ;idx++) {
        Serial.write(packet[idx]);
    }
}

uint8_t readMultipleRegisters() {
    uint8_t modbusOut[32];
    uint8_t len=8;
    uint16_t byteCount;
    uint16_t registerCount;
    uint16_t startAddress;
    int idx;
    uint16_t res;
    uint16_t address;
    uint8_t errorCode=0;
    int tmp;

    memset(&modbusOut,0,sizeof(modbusOut));

    for(idx=2;idx<len;idx++) {
        modbusBuffer[idx] = getByte();
    }

    res=calcCRC(&modbusBuffer[0],len);
    //
    // If this is 0 then it was a good packet.
    //
    if( 0 == res ) {
        int rn=0;
        uint8_t loByte;
        uint8_t hiByte;

        startAddress = 0;
        registerCount = 0;
        startAddress = (modbusBuffer[2] << 8) | modbusBuffer[3] ;
        registerCount = ((modbusBuffer[4] << 8) | modbusBuffer[5]);
        byteCount = registerCount * 2;

        if ( (startAddress + registerCount) > REGISTERS ) {
            return ( ILLEGAL_DATA_ADDRESS );
        }
        nilSemWait(&modbusSem);
        modbusOut[0]=modbusBuffer[0];
        modbusOut[1]=modbusBuffer[1];
        modbusOut[2]=(uint8_t) (byteCount & 0xff);

        idx = 3;
        for(address=startAddress;
                address <(startAddress + registerCount);address++) {

            loByte = (modbusRegisters[address] & 0xff00 ) >> 8;
            hiByte = (modbusRegisters[address] & 0xff );

            modbusOut[idx++] = hiByte;
            modbusOut[idx++] = loByte;
                
            /*
        nilSysLock();
        nilSysUnlock();
            */
        }
        nilSemSignal(&modbusSem);
        sendPacket(&modbusOut[0]);
    } else {
        // CRC error
    }
    return(errorCode);
}

void sendException(uint8_t function,uint8_t exception) {
    uint8_t op[8];
    uint16_t crc;

    op[0] = modbusRegisters[RTU];
    op[1] = function | 0x80;
    op[2] = exception;

    /*
    crc = calcCRC(&op[0],3);
    op[3] = crc & 0xff;
    op[4] = (crc >> 8) & 0xff;
    */
    sendPacket(&op[0]);
}

NIL_THREAD(thModBus, arg) {
    uint8_t data = 0;
    uint8_t errorCode;

    while( TRUE ) {
        errorCode = 0;
        nilThdSleep(1);
        data = ModBusPause(3647);
        if ( data == modbusRegisters[RTU] ) {
            modbusBuffer[0] = data;
            data = getByte();
            modbusBuffer[1] = data;

            switch(data) {
                case RR:
                    errorCode = readMultipleRegisters();
                    break;
                default:
                    errorCode = ILLEGAL_FUNCTION;
                    break;
            }
        }

        if(errorCode != 0) {
            // needs error code, address & function
            sendException(modbusBuffer[1],errorCode);
        }

        nilThdSleep(1);
    }
}

/*
 * Collect data from current sensor.
 */
NIL_THREAD(Sensor, arg) {
    uint16_t n = 0;
    uint16_t sensorValue = 0;        // value read.
    int value = 0;        // value read.
    int count=0;
    int total = 0;
    uint16_t outputValue;

    // Execute while loop every 2.5 milli seconds.
    while (TRUE) {
        digitalWrite(13,LOW);

        nilThdSleepMicroseconds(TIMER_DELAY);

        sensorValue = analogRead(analogInPin);
        value = map(value, 0, 1023, -20000, 20000);
        total += sq(value);

        if( 8 == count ) {
            outputValue = sqrt(total / 8);
            modbusRegisters[V_RMS] = outputValue;
            count=0;
            total=0;
            digitalWrite(13,HIGH);
        } else {
            count++;
        }
    }
}
//
// Send ascii chars down the serial port, up to 'digits' and
// return the result as an integer.  Ignore none numeric, except enter.
// allow backspace.
//
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

void showSettings() {
    int line;
    int col;
    uint8_t runFlag=TRUE;

    line = 3;
    col = 20;

    cls();
    move(line++, col);
    setupSerial.print("Settings");
    move(line++, col);
    setupSerial.print("========");

    line = 6;
    col = 10;

    move(line,col);
    setupSerial.print("ModBus Address :");
    setupSerial.println(modbusRegisters[RTU]);

    move(20, 20);
    setupSerial.print("Press a key to continue");
    while(runFlag) {
        while (setupSerial.available() > 0) {
            setupSerial.read();
            runFlag=FALSE;
        }
    }


}

void drawModbusMenu() {
    int line;
    int col;

    line = 3;
    col = 20;

    cls();
    move(line++, col);
    setupSerial.print("Modbus Menu");
    move(line++, col);
    setupSerial.print("===========");

    line = 6;
    col = 10;

    move(line++, col);
    setupSerial.print("1:    RTU Address");
    setupSerial.print(" (");
    setupSerial.print(modbusRegisters[RTU]);
    setupSerial.print(")");

    move(line++, col);
    setupSerial.print("2:    Baud Rate");

    line++;
    move(line++, col);
    setupSerial.print("x:    Exit menu");

    move(20, 20);
    setupSerial.print("Option> ");
}

void modbusMenu() {
    uint8_t flag = 0;
    uint8_t redraw = 1;

    char r;

    while ( 0 == flag ) {
        if (redraw) {
            drawModbusMenu();
            redraw = 0;
        }
        r = setupSerial.read();

        if (r > 0) {
            switch (r) {
                case '1':
                    move(6,35);
                    setupSerial.setTimeout(1000);
                    modbusRegisters[RTU]=getNumber(3);
                    redraw = 1;
                    break;
                case '2':
                    setupSerial.print("Baud Rate");
                    delay(1000);
                    redraw = 1;
                    break;
                case 'x':
                    flag++;
                    break;
                default:
                    redraw = 1;
                    break;
            }

            if ( 'x' == r) {
                flag++;
            }
        }
    }
    cls();
    setupSerial.println("Setup exited.");
}
void drawSetupMenu() {
    int line;
    int col;

    line = 3;
    col = 20;

    cls();
    move(line++, col);
    setupSerial.print("Setup Menu");
    move(line++, col);
    setupSerial.print("==========");

    line = 6;
    col = 10;

    move(line++, col);
    setupSerial.print("1:    ModBus Settings");

    move(line++, col);
    setupSerial.print("2:    Power Settings");

    line++;
    move(line++, col);
    setupSerial.print("s:    Show Settings");

    line++;
    move(line++, col);
    setupSerial.print("6:    Exit Settings");

    move(20, 20);
    setupSerial.print("Option> ");
}


void setupMenu() {
    uint8_t flag = 0;
    uint8_t redraw = 1;

    char r;

    while ( 0 == flag ) {
        if (redraw) {
            drawSetupMenu();
            redraw = 0;
        }
        r = setupSerial.read();

        if (r > 0) {
            switch (r) {
                case '1':
                    setupSerial.print("Modbus");
                    modbusMenu();
                    redraw = 1;
                    break;
                case '2':
                    setupSerial.print("Power");
                    delay(1000);
                    redraw = 1;
                    break;
                case 's':
                    setupSerial.print("Show Settings");
                    showSettings();
                    redraw = 1;
                    break;
                case 'q':
                    flag++;
                    break;
                default:
                    redraw = 1;
                    break;
            }

            if ( 'q' == r) {
                flag++;
            }
        }
    }
    cls();
    setupSerial.println("Setup exited.");
}


/*
 * Threads static table, one entry per thread.  A thread's priority is
 * determined by its position in the table with highest priority first.
 *
 * These threads start with a null argument.  A thread's name is also
 * null to save RAM since the name is currently not used.
 *
 */
    NIL_THREADS_TABLE_BEGIN()
    NIL_THREADS_TABLE_ENTRY(NULL, Sensor, NULL, waThread1, sizeof(waThread1))
    NIL_THREADS_TABLE_ENTRY(NULL, thModBus, NULL, waModBus, sizeof(waModBus))
NIL_THREADS_TABLE_END()

    /*
     * START Here
     */
    void setup() {

        for(int i=0;i<0x20;i++) {
            modbusRegisters[i] = 0x1234;
        }

        modbusRegisters[RTU]=0x01;

        pinMode(TRIP, INPUT_PULLUP);
        pinMode(RESET, INPUT_PULLUP);
        pinMode(13, OUTPUT);

        //        digitalWrite(13,HIGH);
        delay(100);
        digitalWrite(13,LOW);
        delay(100);


        setupSerial.begin(9600); // Second, soft serial port
        setupSerial.print("Setup port ready.");
        setupSerial.print(RTU);
        setupSerial.print("->");
        setupSerial.println(modbusRegisters[RTU]);

        Serial.begin(9600); // H/w port, for ModBus

        // nilSysBegin();
        /*
         * Check if both TRIP and RESET buttons are pressed
         * If they are enter setup mode.
         *
         */
        if ( TRIP == LOW && RESET == LOW ) {
            setupSerial.println("Both buttons pressed.");
            setupSerial.println("Entering setup menus on soft serial port.");
            setupMenu();
            setupSerial.println("Exiting setup menus.");
        }

        /* Check if ModBus address has been changed
         * if not enter setup mode.
         */

        // Read RTU_ID from EEPROM
        //
        if (0 == modbusRegisters[RTU] ) {
            setupSerial.println("RTU ID Still set to default.");
            setupSerial.println("Entering setup menus on soft serial port.");
            setupMenu();
        }

        /* Check if something connected to second serial port.
         * if it is enter setup mode.
         * This assumes an FTDI module or similar that can generate a signal
         * Indicating connection.
         *
         * Otherwise, we are done here.
         */

        // start kernel
        nilSysBegin();
    }

void loop() {
    // put your main code here, to run repeatedly:
    nilThdDelayMilliseconds(1000);

}
