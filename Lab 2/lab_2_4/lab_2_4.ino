// LED Matrix setup commands
#define OP_DECODEMODE  8
#define OP_SCANLIMIT   10
#define OP_SHUTDOWN    11
#define OP_DISPLAYTEST 14

// LED Matrix Pins
#define CS 1 << 3 // PIN 46
#define DIN 1 << 4 // PIN 45
#define CLK 1 << 5 // PIN 44

// Joystick Pins
#define JOY_X A1
#define JOY_Y A0

// Mary Had a Little Lamb Data
#define NOTE_c 261 // 261 Hz
#define NOTE_d 294 // 294 Hz
#define NOTE_e 329 // 329 Hz
#define NOTE_f 349 // 349 Hz
#define NOTE_g 392 // 392 Hz
#define NOTE_a 440 // 440 Hz
#define NOTE_b 493 // 493 Hz
#define NOTE_C 523 // 523 Hz
#define NOTE_R 0 // Empty note

const int melody[] = { 
  NOTE_e, NOTE_R, 
  NOTE_d, NOTE_R, 
  NOTE_c, NOTE_R, 
  NOTE_d, NOTE_R, 
  NOTE_e, NOTE_R, 
  NOTE_e, NOTE_R, 
  NOTE_e,
  NOTE_e, NOTE_R,
  NOTE_d, NOTE_R, 
  NOTE_d, NOTE_R, 
  NOTE_d,
  NOTE_d, NOTE_R, 
  NOTE_e, NOTE_R, 
  NOTE_g, NOTE_R, 
  NOTE_g,
  NOTE_g, NOTE_R, 

  NOTE_e, NOTE_R, 
  NOTE_d, NOTE_R, 
  NOTE_c, NOTE_R,
  NOTE_d, NOTE_R,
  NOTE_e, NOTE_R,
  NOTE_e, NOTE_R,
  NOTE_e,
  NOTE_e, NOTE_R,
  NOTE_d, NOTE_R,
  NOTE_d, NOTE_R,
  NOTE_e, NOTE_R,
  NOTE_d, NOTE_R,
  NOTE_c, 
  NOTE_c, NOTE_R,

  NOTE_R, NOTE_R,
  NOTE_R, NOTE_R,
  NOTE_R, NOTE_R,
  NOTE_R, NOTE_R};

// constexpr makes this evaluate at runtime, reducing calculation on the microcontroller
constexpr int NOTES = sizeof(melody) / sizeof(int); // Note count in melody

// Clock Rate for MEGA2560
#define CLOCK_RATE 16000000

// Sets the output rate for TIMER4
// Input: hertz - the hertz to drive TIMER4 at. If hertz is zero, the timer is disabled.
void setTimer4Hertz(int hertz);

//Transfers 1 SPI command to LED Matrix for given row
//Input: row - row in LED matrix
//       data - bit representation of LEDs in a given row; 1 indicates ON, 0 indicates OFF
void spiTransfer(volatile byte row, volatile byte data);

// Enables Timer 4
void enableTimer4();

uint16_t spidata; // SPI shift register uses 16 bits, 8 for ctrl and 8 for data
uint8_t currentNote = 0; // the current note for the song

void setup(){
  DDRL = CS | DIN | CLK; // Enable output for the CS, DIN, and CLK pins

  PORTL |= CS; // Turn on CS

  enableTimer4();

  // Setup the LED Matrix
  spiTransfer(OP_DISPLAYTEST, 0);
  spiTransfer(OP_SCANLIMIT, 7);
  spiTransfer(OP_DECODEMODE, 0);
  spiTransfer(OP_SHUTDOWN, 1);

  // Clear the LED Matrix
  for (int i = 0; i < 8; i++){
    spiTransfer(i, 0);
  }
}

void loop(){
  int repeats = melody[currentNote % NOTES] == NOTE_R ? 1 : 6; // Play the empty notes shorter. 
  for (int i = 0; i < repeats; i++) {
    int x = analogRead(JOY_X) / (1 << (10 - 3)); // Read JOY_X and then divide to a value for 0-7
    int y = analogRead(JOY_Y) / (1 << (10 - 3)); // Read JOY_Y and then divide to a value for 0-7

    spiTransfer(x, 1 << (7 - y)); // Set the LED at 7 - y, to flip the y-axis, for the current column.
    delay(50); // Delay 50ms
    spiTransfer(x, 0); // Clear the LED
  }
  // Set the new note.
  setTimer4Hertz(melody[(++currentNote) % NOTES]);
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

void spiTransfer(volatile byte opcode, volatile byte data) {
  //load in spi data
  spidata = (opcode+1) << 8 | data;

  // Disable the LED Matrix
  PORTL &= ~CS;

  // Shift all 16 bits, starting from the highest bit to the lowest bit 
  for(int i = 15; i >= 0; i--) {
    // Set the data pin
    if (spidata >> i & 1) {
      PORTL |= DIN;
    } else {
      PORTL &= ~DIN;
    }
    // Toggle the clock
    PORTL |= CLK;
    PORTL &= ~CLK;
  }
  
  // Enable the LED Matrix
  PORTL |= CS;
}
