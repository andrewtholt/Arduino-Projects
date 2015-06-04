#include <NilRTOS.h>

// Use tiny unbuffered NilRTOS NilSerial library.
#include <NilSerial.h>
#include <NilFIFO.h>

#include <SoftwareSerial.h>

NilFIFO<int, 10> fifo;

// Macro to redefine Serial as NilSerial to save RAM.
// Remove definition to use standard Arduino Serial.

#define Serial NilSerial

#include <NilTimer1.h>
const int analogInPin = A0;  // Analog input pin that the sensor is attached to
const int TRIP = 2;
const int RESET = 3;

int sensorValue = 0;        // value read.
#define TIMER_DELAY 2500
// #define TIMER_DELAY 400000

// Global variables.  These will need a seamphore/mutex to serialise access.
//
// Initialise ModBus CRC tables.
//

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

static char     auchCRCLo[] = {0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,
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






int V_RMS;
int I_RMS;
uint8_t RTU_ID = 0;

//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread1, 64);
NIL_WORKING_AREA(waThread2, 64);
NIL_WORKING_AREA(waModBus, 64);

SoftwareSerial setupSerial(10, 11); // RX, TX

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

uint8_t ModBusPause(int time) {
    uint32_t start = micros();
    uint8_t data;
    uint8_t loopFlag = TRUE;

    do {
        if( Serial.available() ) {
            if( (micros() - start) > time) {
                loopFlag = FALSE;
            } else {
                data = Serial.read();
                start = micros();
                nilThdSleep(1);
            }
        }

    } while (loopFlag) ;
    return(Serial.read()); 
}

NIL_THREAD(ModBus, arg) {
    uint8_t flag = 0;
    uint8_t ipBuffer[32];
    uint8_t idx=0;
    uint8_t ready = 0;
    uint8_t data = 0;

    while( TRUE ) {
        setupSerial.println("Entering ModBusPause....");
        data = ModBusPause(3647);
        setupSerial.println("... Leaving ModBusPause");
        setupSerial.println(data,HEX);
    }
}

/*
 * Calculate RMS
 */
NIL_THREAD(Thread2, arg) {
    int *p;
    int outputValue;
    //  long value[8];
    int value;
    int total = 0;
    int count = 0;

    while (TRUE) {
        p = fifo.waitData(TIME_INFINITE);
        count++;
        /*
         *    Serial.print("sensor = " );
         *    Serial.println(sensorValue);
         */
        value = map(sensorValue, 0, 1023, -20000, 20000);

        total += sq(value);
        if (8 == count) {
            count = 0;
            outputValue = sqrt(total / 8);
            /*
             * Save into a semaphore protected  global variable.
             */

            /*
            setupSerial.print("\t output = ");
            setupSerial.print(outputValue);
            setupSerial.println(" mA");
            */
        }

        fifo.signalFree();
    }
}
/*
 * Collect data from current sensor.
 */
NIL_THREAD(Sensor, arg) {
    uint16_t n = 0;
    //    Serial.println("Start");
    nilTimer1Start(TIMER_DELAY);
    uint32_t last = micros();
    int missed = 0;

    // Execute while loop every 0.4 seconds.
    while (TRUE) {
        nilTimer1Wait();
        sensorValue = analogRead(analogInPin);

        /*
         *    uint32_t t = micros();
         *    Serial.print(t - last);
         *    Serial.print(' ');
         *    Serial.println(n++);
         *    last = t;
         */
        int* p = fifo.waitFree(TIME_IMMEDIATE);
        if (p == 0) {
            missed++;
            /*
               Serial.print("==================== MISSED ");
               Serial.println( missed );
               Serial.println("============================");
               */
        } else {
            *p = sensorValue;
            fifo.signalData();
        }
    }
}

void drawModBusMenu() {
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

    move(line++, col);
    setupSerial.print("2:    Baud Rate");

    line++;
    move(line++, col);
    setupSerial.print("q:    Exit Settings");

    move(20, 20);
    setupSerial.print("Option> ");
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
    setupSerial.print("q:    Exit Settings");

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
                    delay(1000);
                    redraw = 1;
                    break;
                case '2':
                    setupSerial.print("Power");
                    delay(1000);
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

//------------------------------------------------------------------------------
/*
 * Threads static table, one entry per thread.  A thread's priority is
 * determined by its position in the table with highest priority first.
 *
 * These threads start with a null argument.  A thread's name is also
 * null to save RAM since the name is currently not used.
 */
    NIL_THREADS_TABLE_BEGIN()
    NIL_THREADS_TABLE_ENTRY(NULL, Sensor, NULL, waThread1, sizeof(waThread1))
    NIL_THREADS_TABLE_ENTRY(NULL, Thread2, NULL, waThread2, sizeof(waThread2))
    NIL_THREADS_TABLE_ENTRY(NULL, ModBus, NULL, waModBus, sizeof(waModBus))
NIL_THREADS_TABLE_END()
    //------------------------------------------------------------------------------

    void setup() {
        pinMode(TRIP, INPUT_PULLUP);
        pinMode(RESET, INPUT_PULLUP);

        setupSerial.begin(9600); // Second, soft serial port
        setupSerial.println("Setup port ready.");

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
        if (0 == RTU_ID ) {
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

}
