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
#define CYCLE_TIME 4

//MARY HAD A LITTLE LAMB NOTES AND CONST ARRAY
#define NOTE_c 3830 // 261 Hz
#define NOTE_d 3400 // 294 Hz
#define NOTE_e 3038 // 329 Hz
#define NOTE_f 2864 // 349 Hz
#define NOTE_g 2550 // 392 Hz\
#define NOTE_a 2272 // 440 Hz
#define NOTE_b 2028 // 493 Hz
#define NOTE_C 1912 // 523 Hz
#define NOTE_R 0

const int melody[] = {NOTE_e, NOTE_R, NOTE_d, NOTE_R, NOTE_c, NOTE_R, NOTE_d, NOTE_R, NOTE_e, NOTE_R, NOTE_e, NOTE_R, NOTE_e, NOTE_R, NOTE_d, NOTE_R,NOTE_d, NOTE_R,NOTE_d, NOTE_R,NOTE_e, NOTE_R,NOTE_g, NOTE_R,NOTE_g, NOTE_R,NOTE_e, NOTE_R,NOTE_d, NOTE_R, NOTE_c, NOTE_R,NOTE_d, NOTE_R,NOTE_e, NOTE_R,NOTE_e, NOTE_R,NOTE_e, NOTE_R,NOTE_e, NOTE_R,NOTE_d, NOTE_R,NOTE_d, NOTE_R,NOTE_e, NOTE_R,NOTE_d, NOTE_R, NOTE_c, NOTE_R, NOTE_c};

void taskA(int seconds);
void taskB(int seconds);
void taskAMod(int seconds);
void taskBMod(int seconds);
void setTimer4Hertz(int hertz);
void mary();
void taskAB(int seconds);
void noOutputs(int seconds);
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

void taskA(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis<seconds*1000){
    if(PART_ONE_TWO){
      int del = 1000/NUM_PINS;
      digitalWrite(PIN_1, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(del);                       // wait for a second
      digitalWrite(PIN_1, LOW);    // turn the LED off by making the voltage LOW
      
      digitalWrite(PIN_2, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(del);                       // wait for a second
      digitalWrite(PIN_2, LOW);    // turn the LED off by making the voltage LOW
      
      digitalWrite(PIN_3, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(del);                       // wait for a second
      digitalWrite(PIN_3, LOW);    // turn the LED off by making the voltage LOW
    }
    else{
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
    }
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
void taskAMod(int seconds){
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis<seconds*1000){
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
    currMillis = millis();
  }
}

void taskBMod(int seconds){
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
    TCCR4A &= ~(1 << COM4A0);
    while(currMillis-startMillis < 4*del){
      currMillis = millis();
    }
    TCCR4A |= 1 << COM4A0;
    currMillis = millis();
  }
}

void setTimer4Hertz(int hertz) {
  Serial.print(hertz);
  Serial.println("Hz");
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
  unsigned long currMillis = millis();
  unsigned long startMillis = millis();
  while(currMillis-startMillis < seconds*1000){
    taskAMod(2);
    taskBMod(CYCLE_TIME);
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
////  //task A
//  taskA(2);
////  //task B  
//  taskB(4);
//  //task A and B together
//  if(TASK_A_AND_B){
//      taskAB(10);
//  }
  if(MARY){
    mary();
  }
//  //no outputs
  noOutputs(1);
}
