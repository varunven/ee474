#define NUM_PINS 3 //total number of pins used in part 1.2
#define LED_DELAY 1000 / NUM_PINS

#define PART_ONE_TWO 0

#ifdef PART_ONE_TWO
  //pins used for part 1.2
  #define LED_1 47 
  #define LED_2 48
  #define LED_3 49
#else
  #define ALL_BITS 0xFF //flag for setting all bits in the DDR to true
  //masks used for bit setting
  #define  LED_1 1 << 2  // Pin 47
  #define  LED_2 1 << 1  // Pin 48
  #define  LED_3 1 << 0  // Pin 49
#endif

void setup() {
  // initialize output pins
  #ifdef PART_ONE_TWO)
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
  #else
    DDRL = LED_1 | LED_2 | LED_3;
  #endif
}

void loop() {
  #ifdef PART_ONE_TWO 
    digitalWrite(LED_1, HIGH);   // Turn on LED_1 for delay
    delay(LED_DELAY);                
    digitalWrite(LED_1, LOW);    
    
    digitalWrite(LED_2, HIGH);   // Turn on LED_2 for delay
    delay(LED_DELAY);                
    digitalWrite(LED_2, LOW);    
    
    digitalWrite(LED_3, HIGH);   // Turn on LED_3 for delay
    delay(LED_DELAY);                
    digitalWrite(LED_3, LOW);    
  #else
    PORTL |= LED_1; // Turn on LED_1 for delay
    delay(LED_DELAY);
    PORTL &= !LED_1;

    PORTL |= LED_2; // Turn on LED_2 for delay
    delay(LED_DELAY);
    PORTL &= !LED_2;

    PORTL |= LED_3; // Turn on LED_3 for delay
    delay(LED_DELAY);
    PORTL &= !LED_3;
  #endif
}
