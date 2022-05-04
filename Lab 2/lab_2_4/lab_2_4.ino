// LED Matrix setup commands
#define OP_DECODEMODE  8
#define OP_SCANLIMIT   10
#define OP_SHUTDOWN    11
#define OP_DISPLAYTEST 14

// LED Matrix Pins
#define CS 1 << 0 // PIN 49
#define DIN 1 << 1 // PIN 48
#define CLK 1 << 2 // PIN 47

// Joystick Pins
#define JOY_X A1
#define JOY_Y A0

//Transfers 1 SPI command to LED Matrix for given row
//Input: row - row in LED matrix
//       data - bit representation of LEDs in a given row; 1 indicates ON, 0 indicates OFF
void spiTransfer(volatile byte row, volatile byte data);

uint16_t spidata; // SPI shift register uses 16 bits, 8 for ctrl and 8 for data

void setup(){
  DDRL = CS | DIN | CLK; // Enable output for the CS, DIN, and CLK pins

  PORTL |= CS; // Turn on CS

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
  int x = analogRead(JOY_X) / (1 << (10 - 3)); // Read JOY_X and then divide to a value for 0-7
  int y = analogRead(JOY_Y) / (1 << (10 - 3)); // Read JOY_Y and then divide to a value for 0-7

  spiTransfer(x, 1 << (7 - y)); // Set the LED at 7 - y, to flip the y-axis, for the current column.
  delay(50); // Delay 50ms
  spiTransfer(x, 0); // Clear the LED
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
