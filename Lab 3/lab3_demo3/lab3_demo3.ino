/* @file   lab3_demo3.ino
 *   @author    Varun Venkatesh, Eli Orona
 *   @date      22-May-2022
 *   @brief   Demo #3 of Lab 3
 *   
 *  Uses a DDS to run tasks 1 and 2 continuously
 */

using task = void(*)();
enum TaskState { READY, RUNNING, SLEEPING, DEAD };

/// This is a struct that keeps track of unique tasks and their states
struct TCB {
  int id_code;
  char desc[20];
  int times_started;
  TaskState state;
  int32_t sleep_time;
  task run;
};

uint8_t uuid = 0;
void schedulerSetup();
void schedulerUpdate();
void schedule_sync();TCB schedule_sync_TCB = { uuid++, "Syncs the scheduler", 0, TaskState::READY, 0, *schedule_sync};
void task1();
TCB task1_TCB = { uuid++, "Led Flash", 0, TaskState::READY, 0, *task1};
void task2();
TCB task2_TCB = { uuid++, "Close Encounters", 0, TaskState::READY, 0, *task2};
void seven_seg_updater();
TCB seven_seg_updater_TCB = { uuid++, "7 seg updater", 0, TaskState::READY, 0, *seven_seg_updater};

/// Array of tasks represented by TCB struct
TCB* tasks[10];
uint8_t taskCount;
uint8_t currentTask;
/// Array of dead tasks
TCB* deadTasks[10];
uint8_t deadTaskCount;

/**
 * @brief Gets the currently running task
 * 
 * Identifies and returns the index of which task is currently running in the tasks array
 * 
 * @return the index of which task is currently running in the tasks array
 */
uint8_t get_running_task() {
    for(uint8_t i = 0; i < 10; i++) {
        if (tasks[i]->state == TaskState::RUNNING) {
        return i;
        }
    }
    return 0xFF;
}

/**
 * @brief Removes a task from the array
 * 
 * Uses @p index to remove a specific task from @p array with length equal to @p size
 * 
 * @param array
 * @param size
 * @param index
 */
void remove_index_from_array(TCB** array, uint8_t size, uint8_t index) {
    for (uint8_t i = index; i < size; i++) {
        array[i] = array[i + 1];
    }
    array[size - 1] = nullptr;
}

/**
 * @brief quits the current task
 *
 * Uses the task array to set a current task state to DEAD until there are no more running tasks
 * 
 * @warning Will not quit any more tasks and prints an error message when there are 10 dead tasks
 */
void task_self_quit(){
    uint8_t currentTask = get_running_task();
    if (currentTask == 0xFF) {
        Serial.println("Called task_self_quit with no running task");
        return;
    }
    TCB* task = tasks[currentTask];
    task->state = TaskState::DEAD;
    remove_index_from_array(tasks, taskCount--, currentTask);
    if (deadTaskCount == 10) {
        Serial.println("Quit task when no dead task slots available.");
        return;
    }
    deadTasks[deadTaskCount++] = task;
}

/**
 * @brief Starts a task
 *
 * Uses @p task to start tasks unless they are all started already or the task is not found
 * 
 * @param task
 * 
 * @see remove_index_from_array
 */
void task_start(TCB* task) {
    uint8_t deadTaskIndex = 0xFF;
    for (uint8_t i = 0; i < deadTaskCount; i++) {
        if (task == deadTasks[i]) {
        deadTaskIndex = i;
        }
    }
    if (deadTaskIndex == 0xFF) {
        Serial.print("Task ");
        Serial.write(task->desc);
        Serial.println(" not found.");
    } else {
        remove_index_from_array(deadTasks, deadTaskCount--, deadTaskIndex);
    }
    task->state = TaskState::READY;
    TCB* sync = tasks[taskCount - 1];
    tasks[taskCount - 1] = task;
    tasks[taskCount++] = sync;
}

/**
 * @brief Sets a task state to sleeping
 *
 * Uses @p t to set a certain task state to sleeping for t seconds if it is currently running
 * 
 * @param t
 */
void sleep_474(int t) {
    TCB* task = tasks[get_running_task()];
    task->sleep_time = t;
    task->state = TaskState::SLEEPING;
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
        TCB* task = tasks[i];
        if (task == nullptr) {
        break;
        }
        if (task->state == TaskState::READY) {
        task->state = TaskState::RUNNING;
        task->run();
        if (task->state != TaskState::SLEEPING) {
            task->state = TaskState::READY;
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
        TCB* task = tasks[i];
        if (task == nullptr) {
        continue;
        }
        if (task->state == TaskState::SLEEPING) {
        task->sleep_time -= 2;
        if (task->sleep_time < 2) {
            task->state = TaskState::READY;
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
    tasks[0] = &task1_TCB;
    tasks[1] = &task2_TCB;
    tasks[2] = &schedule_sync_TCB;
    tasks[3] = nullptr;
    taskCount = 3;
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
