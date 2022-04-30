#define CLOCK_RATE 16000000

void setTimer4Hertz(int hertz);

void setup() {
  // Enable Serial for debugging
  Serial.begin(9600);

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
  TCCR4A |= 1 << COM4A0;
}

void setTimer4Hertz(int hertz) {
  Serial.print(hertz);
  Serial.println("Hz");
  OCR4A = CLOCK_RATE/(hertz * 2);
}
