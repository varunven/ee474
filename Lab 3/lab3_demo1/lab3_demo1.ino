/* 
 * @file   lab3_demo1.ino
 *   @author    Varun Venkatesh, Eli Orona
 *   @date      22-May-2022
 *   @brief   Demo #1 of Lab 3
 *   
 *  Uses a Round Robin Scheduler to run two tasks (Task 1 and Task 2) concurrently infinitely.
 */


using task = void(*)(uint32_t);
void schedulerSetup();
void schedulerUpdate();
void schedule_sync(uint32_t millis);
void task1(uint32_t millis);
void task2(uint32_t millis);

/**
 * @brief Sets up the scheduler to prepare for the tasks.
 * 
 * @see schedulerUpdate
 * @note this method is empty for a round robin scheduler
 */
void schedulerSetup() {}

/**
 * @brief Updates the scheduler. Calls task1 and task2 repeatedly with updated time
 * 
 * @see task1
 * @see task2
 */

void schedulerUpdate() {
    while(1) {
        uint32_t ms = millis();
        task1(ms);
        task2(ms);
        delayMicroseconds((ms + 1) * 1000 - micros());
    }
}


/**
 * @brief Task1 that flashes an LED every 250 ms
 *
 * Use @p millis to update the state of the LED
 * @param millis
 */

void task1(uint32_t millis) {
  int on = (millis / 250) % 4;
  if (on == 0) {
    PORTL |= (1 << PL0);
  } else {
    PORTL &= ~(1 << PL0);
  }
}

// Sets the output rate for TIMER4
// Input: hertz - the hertz to drive TIMER4 at. If hertz is zero, the timer is disabled.
void setTimer4Hertz(int hertz);
int notes[] = {293, 329, 261, 130, 196};

/**
 * @brief Enables the timer on the board
 *
 * @see setup
 * @see setTimer4Hertz
 */
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

/**
 * @brief Sets the timer
 *
 * Uses @p hertz to set the frequency and value of the timer
 * 
 * @param hertz
 * @see task2
 */

void setTimer4Hertz(int hertz) {
    if (hertz == 0) {
        // Disable the timer output
        TCCR4A &= ~(1 << COM4A0);
    } else {
        // Make sure the timer is enabled
        TCCR4A |= 1 << COM4A0;
        // Set the CTC value based on the hertz. Multiplying by two because the clock toggles, and so the value for the ORC4A needs to be toggled twice to be one cycle.
        OCR4A = 16000000/(hertz * 2);
    }
}


/**
 * @brief Plays a theme on the board
 *
 * Uses @p millis and the setTimer4Hertz methods to set the current frequency to a specific value in an array of values 
 * 
 * @see notes
 * @see setTimer4Hertz
 */
void task2(uint32_t millis) {
        int second = (millis / 1000) % (5 + 4);
        if (second < 5) {
        setTimer4Hertz(notes[second]);
        } else {
        setTimer4Hertz(0);
        }
}

/**
 * @brief Standard setup Arduino function
 */

void setup() {
    Serial.begin(9600);
    cli();
    schedulerSetup();
    sei();
    // Task 1 setup
    DDRL |= (1 << PL0);
    enableTimer4();
}

/**
 * @brief Standard loop Arduino function
 * 
 * Calls the schedulerUpdate function perpetually
 * 
 * @see schedulerUpdate
 */
void loop() {
  schedulerUpdate();
}
