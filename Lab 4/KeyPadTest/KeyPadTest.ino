#include <Keypad.h>
#include <Arduino_FreeRTOS.h>

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
byte rowPins[ROWS] = {10, 9, 8, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad --> change as needed for both

TaskHandle_t prompt;
TaskHandle_t play;

int frequencies[MEASURE_SIZE*NUM_MEASURES];
static int nums = 0;
static int startArr = 0;
String inputString;

void promptFrequency(void *pvParameters);
void enableTimer4();
void setTimer4Hertz(int hertz);
void playTheme(void *pvParameters);
void TaskBlink(void *pvParameters);

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup(){
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  cli();
  sei();
  // Task 1 setup
  DDRL |= (1 << PL0); //port 6
  enableTimer4();
  DDRK = 0xFF; // All ports used
  DDRF = 1 << PF7 | 1 << PF6 | 1 << PF5 | 1 << PF4;
    
  pinMode(WARNING_LED, OUTPUT);
  pinMode(CONFIRMATION_LED, OUTPUT);
  pinMode(RESET_LED, OUTPUT);

  // Now set up three tasks to run independently.
  xTaskCreate(
    promptFrequency
    ,  "Prompt Frequency"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &prompt );

  xTaskCreate(
    playTheme
    ,  "Play Notes"   // A name just for humans
    ,  256  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  0  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &play );

  vTaskStartScheduler();
}

//TODO: use pointer in array to mark the front, if they try to add notes at limit, then replace the old note and move pointer fwd one
void promptFrequency(void *pvParameters){  
  // (void) pvParameters;  // allocate stack space for params

  for(;;){
    char customKey = customKeypad.getKey();
    if (customKey){
      if (customKey >= '0' && customKey <= '9') {     // only act on numeric keys
        inputString += customKey;               // append new character to input string
        if(inputString.toInt() > MAX_FREQ){ //can flash an LED to show that frequency was too high and took previous value
          inputString = "";
          digitalWrite(WARNING_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
          vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
          digitalWrite(WARNING_LED, LOW);    // turn the LED off by making the voltage LOW
        }
      } 
      else if (customKey == '#') {
        if (inputString.length() > 0) {
          if(nums < MEASURE_SIZE*NUM_MEASURES){
            frequencies[nums] = inputString.toInt();
            inputString = "";
            nums++;
            digitalWrite(CONFIRMATION_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
            vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
            digitalWrite(CONFIRMATION_LED, LOW);    // turn the LED off by making the voltage LOW
          }
          else{
            startArr++;
            frequencies[startArr-1] = inputString.toInt();
            inputString = "";
            nums++;
            if(startArr == MEASURE_SIZE*NUM_MEASURES){
              startArr = 0;
            }
            digitalWrite(OVERRIDE_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
            vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
            digitalWrite(OVERRIDE_LED, LOW);    // turn the LED off by making the voltage LOW
          }
        }
        else{
          digitalWrite(WARNING_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
          vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
          digitalWrite(WARNING_LED, LOW);    // turn the LED off by making the voltage LOW
        }
      }
      else if (customKey == '*') {
          inputString = "";
          digitalWrite(RESET_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
          vTaskDelay( 500 / portTICK_PERIOD_MS ); // wait for 1000 ms
          digitalWrite(RESET_LED, LOW);    // turn the LED off by making the voltage LOW
      }
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
  TCCR4A |= 1 << COM4A0;
  // Clear the timer counter
  TCNT4 = 0;
}

void setTimer4Hertz(int hertz) {
  if (hertz == 0) {
      // Disable the timer output
      TCCR4A &= ~(1 << COM4A0);
  } else {
      // Make sure the timer is enabled
      TCCR4A |= 1 << COM4A0;
      // Set the CTC value based on the hertz. Multiplying by two because the clock toggles, and so the value for the ORC4A needs to be toggled twice to be one cycle.
      OCR4A = CLOCK_RATE/(hertz * 2);
  }
}

// create task for playing the sounds
void playTheme(void *pvParameters)
{
  // (void) pvParameters;
  //only play when user presses the D button on the keypad (pause other task during it,resume at the end)
  for(;;){
    int note = startArr;
    int count = 0;
    if(nums%MEASURE_SIZE == 0){
      vTaskSuspend(prompt);
      while(count < min(nums, MEASURE_SIZE*NUM_MEASURES)){
        if(note < MEASURE_SIZE*NUM_MEASURES){
          setTimer4Hertz(frequencies[note]);
          vTaskDelay( 800 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          setTimer4Hertz(0);
          vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          note++;
        }
        else{
          note = 0;
          setTimer4Hertz(frequencies[note]);
          vTaskDelay( 800 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          setTimer4Hertz(0);
          vTaskDelay( 100 / portTICK_PERIOD_MS ); // wait for 0.5 s;
          note++;
        }
        count++;
      }
    }
    setTimer4Hertz(0);
    vTaskResume(prompt);
  }
}

void loop(){
  // remains empty because we are using RTOS
}
