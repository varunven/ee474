#define CLOCK_RATE 16000000

#define DEMO_NUMBER 5

#if DEMO_NUMBER == 1
  #define RR
  #define TASK_1
  #define TASK_2
  #define SPEAKER
#elif DEMO_NUMBER == 2
  #define SRRI
  #define TASK_1
  #define TASK_2
  #define SPEAKER
#elif DEMO_NUMBER == 3
  #define DDS
  #define TASK_1
  #define TASK_2
  #define SPEAKER
#elif DEMO_NUMBER == 4
  #define SRRI
  #define TASK_1
  #define TASK_2
  #define TASK_3
  #define SPEAKER
  #define SEVEN_SEGMENT
#elif DEMO_NUMBER == 5
  #define DDS
  #define TASK_4
  #define SPEAKER
  #define SEVEN_SEGMENT
#elif DEMO_NUMBER == 6
#endif

#ifdef RR
  using task = void(*)(uint32_t);
  #define TASK_DEF(name, desc) void name(uint32_t millis)
  #define TASK(name) void name(uint32_t millis)
#endif
#ifdef SRRI
  using task = void(*)();
  #define TASK_DEF(name,desc) void name()
  #define TASK(name) void name()
#endif
#ifdef DDS
  using task = void(*)();
  enum TaskState { READY, RUNNING, SLEEPING, DEAD };

  struct TCB {
    int id_code;
    char desc[20];
    int times_started;
    TaskState state;
    int32_t sleep_time;
    task run;
  };
  uint8_t uuid = 0;
  #define TASK_DEF(name, desc) void name();\
    TCB name##_TCB = {\
      uuid++,\
      desc,\
      0,\
      TaskState::READY,\
      0,\
      *name\
    }
    #define TASK(name) void name()
#endif

void schedulerSetup();
void schedulerUpdate();

TASK_DEF(schedule_sync, "Syncs the scheduler");

TASK_DEF(task1, "Led Flash");
TASK_DEF(task2, "Close Encounters");
TASK_DEF(seven_seg_updater, "7 seg updater");
TASK_DEF(task3_counter, "Task 3 counter");
TASK_DEF(task4_sound, "Task 4 sound");
TASK_DEF(task4_countdown, "Task 4 countdown");

#ifdef RR
  TASK(1);
  TASK(2);

  void schedulerSetup() {}
  void schedulerUpdate() {
    while(1) {
      uint32_t ms = millis();
      task1(ms);
      task2(ms);
      delayMicroseconds((ms + 1) * 1000 - micros());
    }
  }
#endif

#ifdef SRRI
  task tasks[10];

  enum TaskState { READY, RUNNING, SLEEPING };
  static TaskState states[10];
  static int sleeping[10];

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

  void schedulerSetup() {
    // Initialize the timer
    TCCR0A = (1 << WGM01); // CTC mode
    TCCR0B = (1 << CS02) ; // 1024 Prescaler
    OCR0A = 117; // Full period of the clock every 500Hz/2ms
    TIMSK0 = (1 << OCIE0A); // Enable Interrupts 
  }

  ISR(TIMER0_COMPA_vect) {
    sFlag = SchedulerState::DONE;
  }

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

  TASK(schedule_sync) {
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
#endif

//data-driven scheduler
#ifdef DDS
  TCB* tasks[10];
  uint8_t taskCount;
  uint8_t currentTask;

  TCB* deadTasks[10];
  uint8_t deadTaskCount;

  uint8_t get_running_task() {
    for(uint8_t i = 0; i < 10; i++) {
      if (tasks[i]->state == TaskState::RUNNING) {
        return i;
      }
    }
    return 0xFF;
  }

  void remove_index_from_array(TCB** array, uint8_t size, uint8_t index) {
    for (uint8_t i = index; i < size; i++) {
      array[i] = array[i + 1];
    }
    array[size - 1] = nullptr;
  }

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
      return;
    }

    remove_index_from_array(deadTasks, deadTaskCount--, deadTaskIndex);
    task->state = TaskState::READY;
    TCB* sync = tasks[taskCount - 1];
    tasks[taskCount - 1] = task;
    tasks[taskCount++] = sync;
  }

  void sleep_474(int t) {
    TCB* task = tasks[get_running_task()];
    task->sleep_time = t;
    task->state = TaskState::SLEEPING;
  }

  enum SchedulerState { PENDING, DONE };
  volatile SchedulerState sFlag = SchedulerState::PENDING;

  void schedulerSetup() {
    // Initialize the timer
    TCCR0A = (1 << WGM01); // CTC mode
    TCCR0B = (1 << CS02) ; // 1024 Prescaler
    OCR0A = 117; // Full period of the clock every 500Hz/2ms
    TIMSK0 = (1 << OCIE0A); // Enable Interrupts 
  }

  ISR(TIMER0_COMPA_vect) {
    sFlag = SchedulerState::DONE;
  }

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

  TASK(schedule_sync) {
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
#endif

#ifdef TASK_1 
  TASK(task1) {
    #ifdef RR
      int on = (millis / 250) % 4;
      if (on == 0) {
        PORTL |= (1 << PL0);
      } else {
        PORTL &= ~(1 << PL0);
      }
    #else
      bool on = PORTL & (1 << PL0);

      if (on) {
        PORTL &= ~(1 << PL0);
        sleep_474(750);
      } else {
        PORTL |= (1 << PL0);
        sleep_474(250);
      }
    #endif
  }
#endif

#ifdef SPEAKER
  // Sets the output rate for TIMER4
  // Input: hertz - the hertz to drive TIMER4 at. If hertz is zero, the timer is disabled.
  void setTimer4Hertz(int hertz);

  int notes[] = {293, 329, 261, 130, 196};

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
#endif

#ifdef TASK_2
  TASK(task2) {
    #ifdef RR
      int second = (millis / 1000) % (5 + 4);
      if (second < 5) {
        setTimer4Hertz(notes[second]);
      } else {
        setTimer4Hertz(0);
      }
    #else
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
    #endif
  }
#endif

#ifdef SEVEN_SEGMENT
  #define DS4 1 << PF7
  #define DS3 1 << PF6
  #define DS2 1 << PF5
  #define DS1 1 << PF4
  #define DS_MASK DS4 | DS3 | DS2 | DS1

  uint16_t display = 0;

  uint16_t sevenSegmentToDigit[2][4] = {{DS1, DS2, DS3, DS4}, {1, 10, 100, 1000}};
  uint8_t digitToOutput[] = {0b00111111, 0b00000110, 0b01011011, 0b01001111, 0b01100110, 0b01101101, 0b01111101, 0b00000111, 0b01111111, 0b01101111};
  TASK(seven_seg_updater) {
    static uint8_t led = 0;

    uint8_t value = (display % (sevenSegmentToDigit[1][led] * 10)) / sevenSegmentToDigit[1][led];

    PORTF = (DS_MASK) ^ sevenSegmentToDigit[0][led];
    PORTK = digitToOutput[value];

    led++;
    led %= 4;
    sleep_474(2);
  }
#endif

#ifdef TASK_3
  TASK(task3_counter) {
    display += 1;
    sleep_474(100);
  }
#endif

#ifdef TASK_4
  TASK(task4_sound) {
    static int note = 0;
    if (note == 5) {
      setTimer4Hertz(0);
      note = 0;
      task_self_quit();
      task_start(&task4_countdown_TCB);
      return;
    }

    setTimer4Hertz(notes[note]);
    display = notes[note];
    note++;
    sleep_474(1000);
  }

  TASK(task4_countdown) {
    static uint8_t countdown = 40;
    if (countdown == 0) {
      task_self_quit();
      task_start(&task4_sound_TCB);
      countdown = 40;
      return;
    }

    display = countdown --;
    sleep_474(100);
  }
#endif

void setup() {
  Serial.begin(9600);
  cli();
  schedulerSetup();
  sei();

  #ifdef TASK_1
    // Task 1 setup
    DDRL |= (1 << PL0);
  #endif
  #ifdef SPEAKER
    enableTimer4();
  #endif
  #ifdef SEVEN_SEGMENT
    DDRK = 0xFF; // All ports used
    DDRF = DS4 | DS3 | DS2 | DS1;
  #endif

  #if DEMO_NUMBER == 2 
    tasks[0] = *task1_TCB;
    tasks[1] = *task2_TCB;
    tasks[2] = *schedule_sync;
    tasks[3] = nullptr;
  #endif

  #if DEMO_NUMBER == 3
    tasks[0] = &task1_TCB;
    tasks[1] = &task2_TCB;
    tasks[2] = &schedule_sync_TCB;
    tasks[3] = nullptr;
    taskCount = 3;
  #endif

  #if DEMO_NUMBER == 4
    tasks[0] = *task1;
    tasks[1] = *task2;
    tasks[2] = *task3_counter;
    tasks[3] = *seven_seg_updater;
    tasks[4] = *schedule_sync;
    tasks[5] = nullptr;
  #endif

  #if DEMO_NUMBER == 5
    tasks[0] = &task4_sound_TCB;
    tasks[1] = &seven_seg_updater_TCB;
    tasks[2] = &schedule_sync_TCB;
    tasks[3] = nullptr;
    taskCount = 3;
    deadTasks[0] = &task4_countdown_TCB;
    deadTasks[1] = nullptr;
    deadTaskCount = 1;
  #endif
}

void loop() {
  schedulerUpdate();
}