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
#define MARY 1

//PART 2
#define CLOCK_RATE 16000000
#define CYCLE_TIME 4000

//MARY HAD A LITTLE LAMB NOTES AND CONST ARRAY
#define NOTE_c 3830 // 261 Hz
#define NOTE_d 3400 // 294 Hz
#define NOTE_e 3038 // 329 Hz
#define NOTE_f 2864 // 349 Hz
#define NOTE_g 2550 // 392 Hz
#define NOTE_a 2272 // 440 Hz
#define NOTE_b 2028 // 493 Hz
#define NOTE_C 1912 // 523 Hz
#define NOTE_R 0

const int melody[] = {NOTE_e, NOTE_R, NOTE_d, NOTE_R, NOTE_c, NOTE_R, NOTE_d, NOTE_R, NOTE_e, NOTE_R, NOTE_e, NOTE_R, NOTE_e, NOTE_R, NOTE_d, NOTE_R,NOTE_d, NOTE_R,NOTE_d, NOTE_R,NOTE_e, NOTE_R,NOTE_g, NOTE_R,NOTE_g, NOTE_R,NOTE_e, NOTE_R,NOTE_d, NOTE_R, NOTE_c, NOTE_R,NOTE_d, NOTE_R,NOTE_e, NOTE_R,NOTE_e, NOTE_R,NOTE_e, NOTE_R,NOTE_e, NOTE_R,NOTE_d, NOTE_R,NOTE_d, NOTE_R,NOTE_e, NOTE_R,NOTE_d, NOTE_R, NOTE_c, NOTE_R, NOTE_c};

// Runs Task A
// Input: seconds - seconds to run the task for
void taskA(int seconds);

// Runs Task B
// Input: seconds - seconds to run the task for
void taskB(int seconds);

// Runs Task A syncronously
// Input: currMillis - the current milliseconds
//        startMillis - the milliseconds when the task started
void taskAMod(int currMillis, int startMillis);

// Runs Task B syncronously
// Input: currMillis - the current milliseconds
//        startMillis - the milliseconds when the task started
void taskBMod(int currMillis, int startMillis);

// Sets the output rate for TIMER4
// Input: hertz - the hertz to drive TIMER4 at. If hertz is zero, the timer is disabled.
void setTimer4Hertz(int hertz);

// Plays Mary had a little lamb
void mary();

// Runs Tasks A and B together
// Input: seconds - seconds to run the tasks for
void taskAB(int seconds);

// No outputs
// Input: seconds - seconds for no outputs
void noOutputs(int seconds);

// Runs Task C
void taskC();

void setup() {
  // Enable Serial for debugging
  Serial.begin(9600);

  // initialize digital pin LED_BUILTIN as an output.
  DDRL = ALL_BITS;

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

void taskA(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis<seconds*1000){
    int del = 1000/NUM_PINS;
    PORTL &= !BIT_0;
    PORTL |= BIT_2;
    delay(del);
    PORTL &= !BIT_2;
    PORTL |= BIT_1;
    delay(del);
    PORTL &= !BIT_1;
    PORTL |= BIT_0;
    delay(del);
    currMillis = millis();
  }
}

void taskB(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis<seconds*1000){
    TCCR4A |= 1 << COM4A0;
    // Set tp 400Hz
    setTimer4Hertz(400);
    delay(1000);
  
    // Set to 250Hz
    setTimer4Hertz(250);
    delay(1000);
  
    // Set to 800Hz
    setTimer4Hertz(800);
    delay(1000);
  
    // Disable then reenable the output
    TCCR4A &= ~(1 << COM4A0);
    delay(1000);
    currMillis = millis();
  }
}

//Task A for LED Sequence
void taskAMod(int currMillis, int startMillis){
    int del = 1000/NUM_PINS;
    //pin 1
    if(currMillis-startMillis < del){
      digitalWrite(PIN_1, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else if(currMillis-startMillis >= del){
      digitalWrite(PIN_1, LOW);   // turn the LED on (HIGH is the voltage level)
    }

    if(currMillis-startMillis >= del && currMillis-startMillis < 2*del){
      digitalWrite(PIN_2, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else if(currMillis-startMillis >= 2*del){
      digitalWrite(PIN_2, LOW);   // turn the LED on (HIGH is the voltage level)
    }

    if(currMillis-startMillis >= 2*del && currMillis-startMillis < 3*del){
      digitalWrite(PIN_3, HIGH);   // turn the LED on (HIGH is the voltage level)
    }
    else if(currMillis-startMillis >= 3*del){
      digitalWrite(PIN_3, LOW);   // turn the LED on (HIGH is the voltage level)
    }
}

void taskBMod(int currMillis, int startMillis){
  int del = 1000;
  // Set tp 400Hz
  if(currMillis-startMillis < del){
    TCCR4A |= 1 << COM4A0;
    setTimer4Hertz(400);
  }

  // Set to 250Hz
  else if(currMillis-startMillis >= del && currMillis-startMillis < 2*del){
    TCCR4A |= 1 << COM4A0;
    setTimer4Hertz(250);
  }

  // Set to 800Hz
  else if(currMillis-startMillis >= 2*del && currMillis-startMillis < 3*del){
    TCCR4A |= 1 << COM4A0;
    setTimer4Hertz(800);
  }

  else if(currMillis-startMillis >= 3*del && currMillis-startMillis < 4*del){
    // Disable the output
    TCCR4A &= ~(1 << COM4A0);
  }

  else {
    // Disable then reenable the output
    TCCR4A |= 1 << COM4A0;
  }
}

void setTimer4Hertz(int hertz) {
  OCR4A = CLOCK_RATE/(hertz * 2);
}

void mary(){
  int sizearr = sizeof melody / sizeof melody[0];
  int del = 1000;
  for(int i=0; i<sizearr; i++){
    int startMillis = millis();
    int currMillis = millis();
    if(melody[i] != 0){
      TCCR4A |= 1 << COM4A0;
      while(currMillis-startMillis < del){
       int freq = 1000000/melody[i]; //1 million
       setTimer4Hertz(freq);
       currMillis = millis();
      }
    }
    else{
      TCCR4A &= ~(1 << COM4A0);
      while(currMillis-startMillis < del/100){
       currMillis = millis();
      }
    }
  }
}

void taskAB(int seconds){
  unsigned long currMillisA = millis();
  unsigned long currMillisB = millis();
  unsigned long currMillis = millis();
 
  unsigned long startMillisA = millis();
  unsigned long startMillisB = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis < seconds*1000){
    if((currMillis-startMillisA)%(2000) == 0){
      startMillisA = currMillis;
    }
    taskAMod(currMillis, startMillisA);
    if((currMillis-startMillisB)%(CYCLE_TIME) == 0){
      startMillisB = currMillis;
    }
    taskBMod(currMillis, startMillisB);
    currMillis = millis();
  }
}

void noOutputs(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  DDRL = !ALL_BITS;
  TCCR4A &= ~(1 << COM4A0); //TODO: not sure how to turn off
  while(currMillis-startMillis < seconds*1000){
    currMillis = millis();
  }
  DDRL = ALL_BITS;
  TCCR4A |= 1 << COM4A0;
}

void taskC(){
  //task A and B together
  taskA(2);
  taskB(4);
  taskAB(10);
//  if(MARY){
//    mary();
//  }
//  //no outputs
  noOutputs(1);
}
