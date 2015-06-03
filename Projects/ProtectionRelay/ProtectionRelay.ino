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

int V_RMS;
int I_RMS;
uint8_t RTU_ID=0;

//------------------------------------------------------------------------------
// Declare a stack with 64 bytes beyond context switch and interrupt needs.
NIL_WORKING_AREA(waThread1, 64);
NIL_WORKING_AREA(waThread2, 64);

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
void move(int x,int y) {
    setupSerial.print("\033[");
    setupSerial.print(x);
    setupSerial.print(";");
    setupSerial.print(y);
    setupSerial.print("H");
}


/*
 * Calculate RMS
 */
NIL_THREAD(Thread2, arg) {  
    int *p;
    int outputValue;
    //  long value[8];
    int value;
    int total=0;
    int count=0;

    while(TRUE) {
        p = fifo.waitData(TIME_INFINITE);
        count++;
        /*
         *    Serial.print("sensor = " );
         *    Serial.println(sensorValue);
         */
        value = map(sensorValue, 0, 1023, -20000, 20000);

        total +=sq(value);    
        if (8 == count) {
            count=0;
            outputValue = sqrt(total/8);
            /* 
             * Save into a semaphore protected  global variable.
             */

            Serial.print("\t output = ");
            Serial.print(outputValue);
            Serial.println(" mA");
        }

        fifo.signalFree();    
    }
}
/*
 * Collect data from current sensor.
 */
NIL_THREAD(Sensor, arg) {
    uint16_t n = 0;
    Serial.println("Start");
    nilTimer1Start(TIMER_DELAY);  
    uint32_t last = micros();
    int missed=0;

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
            Serial.print("==================== MISSED ");
            Serial.println( missed );
            Serial.println("============================");

        } else {
            *p = sensorValue;
            fifo.signalData();
        }
    }
}

void drawSetupMenu() {
    int line;
    int col;

    line=3;
    col=20;

    cls();
    move(line++,col);
    setupSerial.print("Setup Menu");
    move(line++,col);
    setupSerial.print("==========");

    line = 6;
    col=10;

    move(line++,col);
    setupSerial.print("1:    ModBus Settings");

    move(line++,col);
    setupSerial.print("2:    Power Settings");

    line++;
    move(line++,col);
    setupSerial.print("q:    Exit Settings");


    move(20,20);
    setupSerial.print("Option> ");
}

void setupMenu() {
    uint8_t flag=0;
    uint8_t redraw=1;

    char r;

    // drawSetupMenu();

    while ( 0 == flag ){
        if(redraw) {
            drawSetupMenu();
            redraw=0;
        }
        r=setupSerial.read();

        switch(r) {
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
        }

        if( 'q' == r) {
            flag++;
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
NIL_THREADS_TABLE_END()
    //------------------------------------------------------------------------------

    void setup() {
        pinMode(TRIP, INPUT_PULLUP); 
        pinMode(RESET, INPUT_PULLUP); 

        setupSerial.begin(9600);
        setupSerial.println("Setup port ready.");

        Serial.begin(9600);

        // nilSysBegin();
        /*
         * Check if both TRIP and RESET buttons are pressed
         * If they are enter setup mode.
         * 
         */
        if( TRIP == LOW && RESET == LOW ) {
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
