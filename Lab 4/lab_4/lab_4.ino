/* @file   lab3_demo5.ino
 *   @author    Varun Venkatesh, Eli Orona
 *   @date      22-May-2022
 *   @brief   Lab 4 
 *   
 *  Uses an RTOS to run certain tasks
 */
#include <Arduino_FreeRTOS.h>

#define OFF_BOARD_PIN 10

////////////////////////////////////////////////
// APPROVED FOR ECE 474   Spring 2021
//
//  NOTE: modify analogRead() on line 113 according
//   to your setup.
////////////////////////////////////////////////

// define two tasks for Blink & AnalogRead
void RT1( void *pvParameters );
void TaskAnalogRead( void *pvParameters );

/**
 * @brief Standard setup Arduino function
 * 
 * @see enableTimer4
 * @see schedulerSetup
 */void setup() {
  
  // initialize serial communication at 9600 bits per second:
  Serial.begin(19200);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  } 

  cli();
  schedulerSetup();
  sei();
  // Task 1 setup
  DDRL |= (1 << PL0);
  enableTimer4();

  // Now set up two tasks to run independently.
  xTaskCreate(
    RT1
    ,  "Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
  //  (note how the above comment is WRONG!!!)
  vTaskStartScheduler();
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskBlink(void *pvParameters)  // This is a task.
{
 // (void) pvParameters;  // allocate stack space for params

  // initialize digital LED_BUILTIN on pin 10 as an output.
  pinMode(OFF_BOARD_PIN, OUTPUT);

  for (;;) // A Task shall never return or exit.
  {
    digitalWrite(OFF_BOARD_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for 100 ms
    digitalWrite(OFF_BOARD_PIN, LOW);    // turn the LED off by making the voltage LOW
    vTaskDelay( 200 / portTICK_PERIOD_MS ); // wait for 200 ms
  }
}


int notes[] = {293, 329, 261, 130, 196};
void PlayTheme(void *pvParameters)
{
   // (void) pvParameters;
   
  for(int i=0; i<3; i++){
    static int note = 0;
    if (note == 5) {
      setTimer4Hertz(0);
      note = 0;
      return;
    }
    setTimer4Hertz(notes[note]);
    note++;
    vTaskDelay( 1500 / portTICK_PERIOD_MS ); // wait for 1.5 s;
  }
}

/**
 * @brief Enables the timer on the board
 *
 */
void enableTimer4() {
    // Enable the clock
    PRR1 &= ~(1 << PRTIM4);
    // Enable the output pin
    DDRH |= 1 << PH3;
    // Clear the timer registers
    TCCR4A = 0;
    TCCR4B = 0;
    // turn on CTC mode
    TCCR4B |= (1 << WGM12);
    // Set CS10 bit for a prescaler of 1
    TCCR4B |= (1 << CS10);
    // Enable output on PH3
    TCCR4A |= 1 << COM4A0;
    // Clear the timer counter
    TCNT4 = 0;
}

/**
 * @brief Sets the timer
 *
 * Uses @p hertz to set the frequency and value of the timer
 * 
 * @param hertz
 */
void setTimer4Hertz(int hertz) {
    if (hertz == 0) {
        // Disable the timer output
        TCCR4A &= ~(1 << COM4A0);
    } else {
        // Make sure the timer is enabled
        TCCR4A |= 1 << COM4A0;
        // Set the CTC value based on the hertz. Multiplying by two because the clock toggles, and so the value for the ORC4A needs to be toggled twice to be one cycle.
        OCR4A = 16000000/(hertz * 2);
    }
}


void TaskAnalogRead(void *pvParameters)  // This is a task.
{
 // (void) pvParameters;
  
/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

  for (;;)
  {
    // read the input on analog pin 0:
    int sensorValue = analogRead(A7);  /// modify for your input pin!
    // print out the value you read:
    Serial.println(sensorValue);
    vTaskDelay(500/portTICK_PERIOD_MS);  // 0.5 sec in between reads for stability
  }
}
