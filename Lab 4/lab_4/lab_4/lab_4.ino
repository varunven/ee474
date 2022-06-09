#include <Keypad.h>
#include <Arduino_FreeRTOS.h>
#include "task.h"
#include "queue.h"

#define RESET_LED 46
#define WARNING_LED 48
#define CONFIRMATION_LED 50
#define OVERRIDE_LED 52
#define MEASURE_SIZE 4
#define NUM_MEASURES 4
#define MAX_FREQ 1000

#define CLOCK_RATE 16000000

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

//define the cymbols on the buttons of the keypads
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {12, 11, 9, 8}; //connect to the column pinouts of the keypad --> change as needed for both

TaskHandle_t prompt;
TaskHandle_t playT;

int frequenciesT[MEASURE_SIZE*NUM_MEASURES];
static int numsT;
static int startArrT = 0;
String inputStringT;

void promptFrequency(void *pvParameters);
void enableTimer4();
void setTimer4HertzT(int hertz);
void playTheme(void *pvParameters);
void TaskBlink(void *pvParameters);

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

QueueHandle_t data_queue_T;

void setup(){
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  cli();
  sei();
  enableTimer4();
    
  pinMode(WARNING_LED, OUTPUT);
  pinMode(CONFIRMATION_LED, OUTPUT);
  pinMode(RESET_LED, OUTPUT);

  data_queue_T = xQueueCreate(3, sizeof(boolean));

  // Now set up three tasks to run independently.
  xTaskCreate(
    promptFrequency
    ,  "Prompt Frequency"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &prompt );

  xTaskCreate(
    playThemeTreble
    ,  "Play Treble Notes"   // A name just for humans
    ,  1024  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &playT );

  vTaskStartScheduler();

  frequenciesT[0] = 440;  
  frequenciesT[1] = 880;
  frequenciesT[2] = 220;
}

//TODO: use pointer in array to mark the front, if they try to add notes at limit, then replace the old note and move pointer fwd one
void promptFrequency(void *pvParameters){  
  // (void) pvParameters;  // allocate stack space for params
  for(;;){
    boolean playNotes;
    char customKey = customKeypad.getKey();
    if (customKey){
      if (customKey >= '0' && customKey <= '9') {     // only act on numeric keys
        inputStringT += customKey;               // append new character to input string
        if(inputStringT.toInt() > MAX_FREQ){ //can flash an LED to show that frequency was too high and took previous value
          inputStringT = "";
          digitalWrite(WARNING_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
          vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
          digitalWrite(WARNING_LED, LOW);    // turn the LED off by making the voltage LOW
        }
        playNotes = false;
      }
      else if (customKey == 'A') {
        Serial.println(inputStringT);
        if (inputStringT.length() > 0) {
          if(numsT < MEASURE_SIZE*NUM_MEASURES){
            frequenciesT[numsT] = inputStringT.toInt();
            inputStringT = "";
            numsT++;
            digitalWrite(CONFIRMATION_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
            vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
            digitalWrite(CONFIRMATION_LED, LOW);    // turn the LED off by making the voltage LOW
          }
          else{
            startArrT++;
            frequenciesT[startArrT-1] = inputStringT.toInt();
            inputStringT = "";
            numsT++;
            if(startArrT == MEASURE_SIZE*NUM_MEASURES){
              startArrT = 0;
            }
            digitalWrite(OVERRIDE_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
            vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
            digitalWrite(OVERRIDE_LED, LOW);    // turn the LED off by making the voltage LOW
          }
          playNotes = false;
        }
        else{
          digitalWrite(WARNING_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
          vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
          digitalWrite(WARNING_LED, LOW);    // turn the LED off by making the voltage LOW
          playNotes = false;
        }
      }
      else if (customKey == '*') {
          inputStringT = "";
          digitalWrite(RESET_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
          vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
          digitalWrite(RESET_LED, LOW);    // turn the LED off by making the voltage LOW
          playNotes = false;
      }
      else if (customKey == '#'){
        inputStringT = "";
        digitalWrite(OVERRIDE_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
        vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
        digitalWrite(OVERRIDE_LED, LOW);    // turn the LED off by making the voltage LOW
        playNotes = true;
      }
      else{
        playNotes = false;
      }
      xQueueSend(data_queue_T, &playNotes, 0);
    }
  }
}

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
  TCCR4A |= 1 << (COM4A0);
  // Clear the timer counter
  TCNT4 = 0;
}

void setTimer4HertzT(int hertz) {
  if (hertz == 0) {
      // Disable the timer output
      TCCR4A &= ~(1 << (COM4A0));
  } else {
      // Make sure the timer is enabled
      TCCR4A |= (1 << COM4A0);
      // Set the CTC value based on the hertz. Multiplying by two because the clock toggles, and so the value for the ORC4A needs to be toggled twice to be one cycle.
      OCR4A = CLOCK_RATE/(hertz * 2);
  }
}

// create task for playing the sounds for treble
void playThemeTreble(void *pvParameters)
{
  // (void) pvParameters;
  //only play when user presses the # button on the keypad (pause other task during it,resume at the end)
  for(;;){
    boolean toPlay;
    int note = startArrT;
    int count = 0;
    if(!xQueueReceive(data_queue_T, &toPlay, 0)){
      toPlay = false;
    }
    if(toPlay){
      vTaskSuspend(prompt);
      while(count < min(numsT, MEASURE_SIZE*NUM_MEASURES)){
        Serial.println("Next note");
        if(note < MEASURE_SIZE*NUM_MEASURES){
          setTimer4HertzT(frequenciesT[note]);
          vTaskDelay( 800 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          setTimer4HertzT(0);
          vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          note++;
        }
        else{
          note = 0;
          setTimer4HertzT(frequenciesT[note]);
          vTaskDelay( 800 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          setTimer4HertzT(0);
          vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          note++;
        }
        count++;
      }
      setTimer4HertzT(0);
      vTaskResume(prompt);
    }
  }
}

void loop(){
  // remains empty because we are using RTOS
}
