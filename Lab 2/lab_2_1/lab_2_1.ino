#define NUM_PINS 3 //total number of pins used in part 1.2
//pins used for part 1.2
#define PIN_1 47 
#define PIN_2 48
#define PIN_3 49

#define ALL_BITS 0xFF; //flag for setting all bits in the DDR to true

//masks used for bit setting
#define  BIT_2      1<<2;  //  shift “1” 2 places over
#define  BIT_1      1<<1;  //  shift “1” 1 places over
#define  BIT_0      1<<0;  //  shift “1” 0 places over

#define PART_ONE_TWO 1
// the setup function runs once when you press reset or power the board
void setup() {
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
}

// the loop function runs over and over again forever
void loop() {
  if(PART_ONE_TWO){
    int del = 1000/NUM_PINS; //delay of 1/number of pins of a second in this lab
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
    int del = 1000/NUM_PINS; //delay of 1/number of pins of a second in this lab
    PORTL &= !BIT_0; // turn the pin at bit 0, port L off
    PORTL |= BIT_2; // turn the pin at bit 2, port L on
    delay(del);
    PORTL &= !BIT_2; // turn the pin at bit 2, port L off
    PORTL |= BIT_1; // turn the pin at bit 1, port L on
    delay(del);
    PORTL &= !BIT_1; // turn the pin at bit 1, port L off
    PORTL |= BIT_0; // turn the pin at bit 0, port L on
    delay(del);
  }
}
