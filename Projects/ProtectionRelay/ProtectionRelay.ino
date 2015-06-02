#include <NilRTOS.h>

// Use tiny unbuffered NilRTOS NilSerial library.
#include <NilSerial.h>
#include <NilFIFO.h>
NilFIFO<int, 10> fifo;

// Macro to redefine Serial as NilSerial to save RAM.
// Remove definition to use standard Arduino Serial.

#define Serial NilSerial

#include <NilTimer1.h>
const int analogInPin = A0;  // Analog input pin that the sensor is attached to
int sensorValue = 0;        // value read.
#define TIMER_DELAY 2500
// #define TIMER_DELAY 400000

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
    Serial.print("sensor = " );
    Serial.println(sensorValue);
    */
    value = map(sensorValue, 0, 1023, -20000, 20000);

    total +=sq(value);    
    if (8 == count) {
      count=0;
      outputValue = sqrt(total/8);
        
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
    uint32_t t = micros();
    Serial.print(t - last);
    Serial.print(' ');
    Serial.println(n++);    
    last = t;
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

  Serial.begin(9600);
  // start kernel
  nilSysBegin();
  /*
   * Check if ModBus has been changed
   * if not enter setup mode.
   *
   * Check if something connected to second serial port.
   * if it is enter setup mode.
   *
   * Otherwise, we are done here.
   */
}

void loop() {
  // put your main code here, to run repeatedly:

}
