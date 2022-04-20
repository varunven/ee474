// the setup function runs once when you press reset or power the board
#define SPEAKER_PIN 2

// #define PROBLEM_4
#define PROBLEM_5

// Some Constants
const int SECOND = 1000;

const int SPEAKER_HZ = 250;
const int SPEAKER_MS = SECOND / SPEAKER_HZ;

const int LED_HZ = 2;
const int LED_HZ_TOGGLE = LED_HZ / 2;
const int LED_MS = SECOND / LED_HZ_TOGGLE;

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(SPEAKER_PIN, OUTPUT);  //need to pick up speaker to test
}


uint32_t counter = 0;
// the loop function runs over and over again forever
void loop() {
#ifdef PROBLEM_4
  digitalWrite(10, HIGH);          // turn the LED on (HIGH is the voltage level) 
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED off (LOW is the voltage level)
  digitalWrite(SPEAKER_PIN, HIGH); // Click the speaker for problem 4 only
  delay(SECOND);  // wait for a second

  digitalWrite(10, LOW);            // turn the LED off (LOW is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level)
  digitalWrite(SPEAKER_PIN, LOW); // Click the Speaker for Problem 4 only
  delay(SECOND); // wait for a second
#endif

#ifdef PROBLEM_5
  if (counter % (SPEAKER_MS/2) == 0 && counter < 4 * SECOND) { // Toggle the speaker every SPEAKER_MS/2 ms for square wave with 50% duty cycle
    digitalWrite(SPEAKER_PIN, (counter % SPEAKER_MS) / (SPEAKER_MS / 2)); 
  }

  if (counter % LED_MS == 0) { // Toggle the LEDs every LED_MS ms
    digitalWrite(10, (counter % (LED_MS * 2)) / LED_MS);
    digitalWrite(LED_BUILTIN,  1 - (counter % (LED_MS * 2)) / LED_MS);
  }

  counter ++; // Increment the ms
  delay(1); // Delay one ms
#endif
}