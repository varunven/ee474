/* @file   lab3_demo5.ino
 *   @author    Varun Venkatesh, Eli Orona
 *   @date      22-May-2022
 *   @brief   Lab 4 
 *   
 *  Uses an RTOS to run certain tasks
 */
#include <Arduino_FreeRTOS.h>
#include <queue.h>

#define OFF_BOARD_PIN A0

#define RANDOM_VALUES 64

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

// define two tasks for Blink & AnalogRead
void TaskBlink( void *pvParameters );
void PlayTheme( void *pvParameters );
void RT3p0( void *pvParameters );
void RT3p1( void *pvParameters );
void RT4( void *pvParameters );

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

  DDRL |= (1 << PL0);
  enableTimer4();

  xTaskCreate(
    TaskBlink
    ,  "Blink"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    PlayTheme
    ,  "Theme"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    RT3p0
    ,  "Value Generation"
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
  for (;;) {
    for(int i=0; i < 3; i++) {
      for (int j = 0; j < 5; j ++) {
        setTimer4Hertz(notes[j]);
        vTaskDelay( 1000 / portTICK_PERIOD_MS ); // wait for 1 s;
      } 
      setTimer4Hertz(0);
      vTaskDelay( 1500 / portTICK_PERIOD_MS ); // wait for 1.5 s;
    }

    vTaskSuspend( NULL );
  }
}

double data[RANDOM_VALUES];
QueueHandle_t data_queue;
QueueHandle_t time_queue;

void RT3p0(void *pvParameters) {
  for (;;) {
    for (int i = 0; i < RANDOM_VALUES; i++) {
      data[i] = analogRead(A7) / 1024.0;
      Serial.println(data[i]);
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    data_queue = xQueueCreate(5, sizeof(double*));
    time_queue = xQueueCreate(5, sizeof(int));

    xTaskCreate(
    RT3p1
    ,  "Value Sending"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

    xTaskCreate(
    RT4    
    ,  "FFT"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

    vTaskSuspend( NULL );
  }
}

void RT3p1(void *pvParameters) {
  for (;;) {
    int time, total_time = 0;
    for(int i = 0; i < 5; i++) {
      xQueueSendToBack(data_queue, data, 0);

      double* data2;

      xQueuePeek(data_queue, data2, 0);
      Serial.println(data[0]);
      Serial.println(*data2);
      Serial.println();

      // while(xQueueReceive(time_queue, &time, 1) != pdPASS) {
      //   vTaskDelay(10/portTICK_PERIOD_MS);
      // }

      total_time += time;
    }

    Serial.println(total_time);
    vTaskSuspend( NULL );
  }
}
void RT4(void *pvParameters) {
  for (;;) {
    int time = 100;
    double* data;
    for(int i = 0; i < 5; i++) {
      Serial.println("Start PT4 Loop");
      while(xQueueReceive(data_queue, data, 0) != pdPASS) {
        vTaskDelay(100/portTICK_PERIOD_MS);
      }
      Serial.println("PT4 received");
      Serial.println(*data);
      
      vTaskDelay(100/portTICK_PERIOD_MS);

      Serial.println("PT4 sent");
      xQueueSendToBack(time_queue, &time, 0);
    }
    vTaskSuspend( NULL );
  }
}