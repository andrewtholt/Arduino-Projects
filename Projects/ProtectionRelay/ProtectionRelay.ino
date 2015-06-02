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

SoftwareSerial setupSerial(10, 11); // RX, TX

void setupMenu() {
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
  
  // start kernel
  nilSysBegin();
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
    setupSerial.println("Exiting setup menus.");
  }
  
  /* Check if something connected to second serial port.
   * if it is enter setup mode.
   * This assumes an FTDI module or similar that can generate a signal
   * Indicating connection.
   *
   * Otherwise, we are done here.
   */
  nilSysBegin();
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
