//PART 1
#define NUM_PINS 3
#define PIN_1 47
#define PIN_2 48
#define PIN_3 49
#define ALL_BITS 0xFF;
#define  BIT_2      1<<2;  //  shift “1” 2 places over
#define  BIT_1      1<<1;  //  shift “1” 1 places over
#define  BIT_0      1<<0;  //  shift “1” 0 places over

#define PART_ONE_TWO 1
#define TASK_A_AND_B 1

//PART 2
#define CLOCK_RATE 16000000
#define CYCLE_TIME 4

void taskA(int seconds);
void taskB();
void setTimer4Hertz(int hertz);
void taskC();

void setup() {
  // Enable Serial for debugging
  Serial.begin(9600);

  // initialize digital pin LED_BUILTIN as an output.
  if(PART_ONE_TWO){
    pinMode(PIN_1, OUTPUT);
    pinMode(PIN_2, OUTPUT);
    pinMode(PIN_3, OUTPUT);
  }
  else{
    DDRL = ALL_BITS;
  }
  pinMode(LED_BUILTIN, OUTPUT);

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

void loop() {
  taskC();
}

//Task A for LED Sequence
void taskA(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis<seconds*1000){
    currMillis = millis();
    startMillis = millis();
    if(PART_ONE_TWO){
      int del = 1000/NUM_PINS;
      //pin 1
      while(currMillis-startMillis < del){
        digitalWrite(PIN_1, HIGH);   // turn the LED on (HIGH is the voltage level)
        digitalWrite(PIN_1, LOW);   // turn the LED on (HIGH is the voltage level)
        currMillis = millis();
      }

      //pin 2
      while(currMillis-startMillis < 2*del){
        digitalWrite(PIN_2, HIGH);   // turn the LED on (HIGH is the voltage level)
        digitalWrite(PIN_2, LOW);   // turn the LED on (HIGH is the voltage level)
        currMillis = millis();
      }
      
      //pin 3
      while(currMillis-startMillis < 3*del){
        digitalWrite(PIN_3, HIGH);   // turn the LED on (HIGH is the voltage level)
        digitalWrite(PIN_3, LOW);   // turn the LED on (HIGH is the voltage level)
        currMillis = millis();
      }
    }
    else{
      int del = 1000/NUM_PINS;
      while(currMillis-startMillis < del){
        PORTL &= !BIT_0;
        PORTL |= BIT_2;
        currMillis = millis();
      }
      while(currMillis-startMillis < 2*del){
        PORTL &= !BIT_2;
        PORTL |= BIT_1;
        currMillis = millis();
      }
      while(currMillis-startMillis < 3*del){
        PORTL &= !BIT_1;
        PORTL |= BIT_0;
        currMillis = millis();
      }
    }
  }
}

void taskB(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  int del = 1000;
  while(currMillis-startMillis < seconds*1000){
    // Set tp 400Hz
    while(currMillis-startMillis < del){
      setTimer4Hertz(400);
      currMillis = millis();
    }
  
    // Set to 250Hz
    while(currMillis-startMillis < 2*del){
      setTimer4Hertz(250);
      currMillis = millis();
    }
  
    // Set to 800Hz
    while(currMillis-startMillis < 3*del){
      setTimer4Hertz(800);
      currMillis = millis();
    }

    // Disable then reenable the output
    while(currMillis-startMillis < 4*del){
      TCCR4A &= ~(1 << COM4A0);
      currMillis = millis();
    }
    TCCR4A |= 1 << COM4A0;
    digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
    currMillis = millis();
  }
}

void setTimer4Hertz(int hertz) {
  Serial.print(hertz);
  Serial.println("Hz");
  OCR4A = CLOCK_RATE/(hertz * 2);
}

void taskAB(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis < seconds*1000){
    taskB(CYCLE_TIME);
    taskA(2);
    currMillis = millis();
  }
}

void noOutputs(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
//  while(currMillis-startMillis < seconds*1000){
//    DDRL = !ALL_BITS;
//    TCCR4A &= (1 << COM4A0); //TODO: not sure how to turn off
//    currMillis = millis();
//  }
  DDRL &= !ALL_BITS;
  DDRH &= !ALL_BITS;
}

void taskC(){
  //task A
//  taskA(2);
  //task B  
//  taskB(4);
  //task A and B together
  if(TASK_A_AND_B){
      taskAB(10);
  }
  //no outputs
  noOutputs(10);
}
