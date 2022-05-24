/* @file   lab3_demo4.ino  
 *   @author    Varun Venkatesh, Eli Orona
 *   @date      22-May-2022
 *   @brief   Demo #4 of Lab 3
 *   
 *  Uses a SSRI to run Tasks 1, 2, and 3 continuously
 */
 
using task = void(*)();
void schedulerSetup();
void schedulerUpdate();
void schedule_sync();
void task1();
void task2();
void seven_seg_updater();
void task3_counter();

/// Array of tasks
task tasks[10];
enum TaskState { READY, RUNNING, SLEEPING };
/// Array of task states
static TaskState states[10];
/// Array of sleeping tasks
static int sleeping[10];

/**
 * @brief Sets a task state to sleeping
 *
 * Uses @p t to set a certain task state to sleeping for t seconds if it is currently running
 * 
 * @param t
 */
void sleep_474(int t) {
    for(int i = 0; i < 10; i++) {
        if (states[i] == TaskState::RUNNING) {
        sleeping[i] = t;
        states[i] = TaskState::SLEEPING;
        return;
        }
    }
}

enum SchedulerState { PENDING, DONE };
volatile SchedulerState sFlag = SchedulerState::PENDING;

/**
 * @brief Sets up the scheduler to prepare for the tasks.
 */
void schedulerSetup() {
    // Initialize the timer
    TCCR0A = (1 << WGM01); // CTC mode
    TCCR0B = (1 << CS02) ; // 1024 Prescaler
    OCR0A = 117; // Full period of the clock every 500Hz/2ms
    TIMSK0 = (1 << OCIE0A); // Enable Interrupts 
}

/**
 * @brief Interrupt Service Routine
 */
ISR(TIMER0_COMPA_vect) {
    sFlag = SchedulerState::DONE;
}

/**
 * @brief Updates the scheduler.
 * 
 * For each alive task in the task array, updates the state to RUNNING or READY
 */
void schedulerUpdate() {
    for (int i = 0; i < 10; i++) {
        if (tasks[i] == nullptr) {
        break;
        }
        if (states[i] == TaskState::READY) {
        states[i] = TaskState::RUNNING;
        tasks[i]();
        if (states[i] != TaskState::SLEEPING) {
            states[i] = TaskState::READY;
        }
        }
    }
}

/**
 * @brief Syncs the scheduler
 * 
 * Uses the current value of sFlag boolean to change the amount of time each task still has to sleep
 */
void schedule_sync() {
    while (sFlag == SchedulerState::PENDING) {
        //no op
    }
    for(int i = 0; i < 10; i++) {
        if (states[i] == TaskState::SLEEPING) {
        sleeping[i] -= 2;
        if (sleeping[i] < 2) {
            states[i] = TaskState::READY;
        }
        }
    }
    sFlag = SchedulerState::PENDING;
}


/**
 * @brief Task1 that flashes an LED every 250 ms
 *
 * Use the sleep_474 method to update the state of the LED based on time
 * 
 * @see sleep_474
 */
void task1() {
  bool on = PORTL & (1 << PL0);
  if (on) {
    PORTL &= ~(1 << PL0);
    sleep_474(750);
  } else {
    PORTL |= (1 << PL0);
    sleep_474(250);
  }
}

// Sets the output rate for TIMER4
// Input: hertz - the hertz to drive TIMER4 at. If hertz is zero, the timer is disabled.
void setTimer4Hertz(int hertz);
int notes[] = {293, 329, 261, 130, 196};

/**
 * @brief Enables the timer on the board
 *
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
 * Uses sleep_474 and the setTimer4Hertz methods to set the current frequency to a specific value in an array of values 
 * 
 * @see notes
 * @see setTimer4Hertz
 * @see sleep_474
 */
void task2() {
  static int note = 0;
  if (note == 5) {
    setTimer4Hertz(0);
    sleep_474(4000);
    note = 0;
    return;
  }
  setTimer4Hertz(notes[note]);
  note++;
  sleep_474(1000);
}

uint16_t display = 0;
uint16_t sevenSegmentToDigit[2][4] = {{1 << PF4, 1 << PF5, 1 << PF6, 1 << PF7}, {1, 10, 100, 1000}};
uint8_t digitToOutput[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};

/**
 * @brief Updates the seg7 display
 * 
 * Uses an array of values to set the output of the seg7 to certain values
 * 
 * @see sleep_474
 */
void seven_seg_updater() {
    static uint8_t led = 0;
    uint8_t value = (display % (sevenSegmentToDigit[1][led] * 10)) / sevenSegmentToDigit[1][led];
    PORTF = (1 << PF7 | 1 << PF6 | 1 << PF5 | 1 << PF4) ^ sevenSegmentToDigit[0][led];
    PORTK = digitToOutput[value];
    led++;
    led %= 4;
    sleep_474(2);
}

/**
 * @brief Updates the counter for task3
 * 
 * Increment the value to be displayed by 1 every 100 ms
 * 
 * @see sleep_474
 */
void task3_counter() {
    display += 1;
    sleep_474(100);
}

/**
 * @brief Standard setup Arduino function
 * 
 * @see enableTimer4
 * @see schedulerSetup
 */
void setup() {
    Serial.begin(9600);
    cli();
    schedulerSetup();
    sei();
    // Task 1 setup
    DDRL |= (1 << PL0);
    enableTimer4();
    DDRK = 0xFF; // All ports used
    DDRF = 1 << PF7 | 1 << PF6 | 1 << PF5 | 1 << PF4;
    tasks[0] = *task1;
    tasks[1] = *task2;
    tasks[2] = *task3_counter;
    tasks[3] = *seven_seg_updater;
    tasks[4] = *schedule_sync;
    tasks[5] = nullptr;
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
