/*----------------------------------------------------------------
 * 
 * freETarget.h
 * 
 * Software to run the Air-Rifle / Small Bore Electronic Target
 * 
 *----------------------------------------------------------------
 *
 *
 */
#ifndef _FREETARget_H
#define _FREETARget_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SOFTWARE_VERSION "\"0.5.0 October 20, 2023\""


#define REV_500    500   // ESP32

#define INIT_DONE       0xabcd                    // Initialization complete signature
#ifndef true
#define true        (1==1)
#define false       (0==1)
#endif
#define CLOCK_TEST   false


/*
 * Options
 */
#define SAMPLE_CALCULATIONS  (1==0)                   // Trace the COUNTER values
#define COMPENSATE_RISE_TIME (1==1)                   // Use PCNT4-7 to compensate for rise time

/*
 * Oscillator Features
 */
#define OSCILLATOR_MHZ   10.0                         // 10,000 cycles in 1 ms
#define CLOCK_PERIOD  (1.0/OSCILLATOR_MHZ)            // Seconds per bit
#define ONE_SECOND    (100)                           // 10 ms delay per LSB
#define MIN_DELAY     (1)                             // Minimum timeout 10ms
#define FULL_SCALE      0xffffffff                    // Full scale timer


#define SHOT_TIME     ((int)(json_sensor_dia / 0.33)) // Worst case delay (microseconds) = sensor diameter / speed of sound)
#define SHOT_STRING   20                              // Allow a maximum of SHOT_STRING for rapid fire

#define HI(x) (((x) >> 8 ) & 0x00ff)                  // High nibble
#define LO(x) ((x) & 0x00ff)                          // Low nibble
#define HHH10(x) (((x) / 10000 ) % 10)                // Highest digit    2xxxx
#define HHI10(x) (((x) / 1000 ) % 10)                 // High High digit  x2xxx
#define HLO10(x) (((x) / 100 ) % 10)                  // High Low digit   xx2xx
#define HI10(x)  (((x) / 10 ) % 10)                   // High digit       xxx2x
#define LO10(x)  ((x) % 10)                           // Low digit        xxxx2

#define N       0                                     // Index to North Timer
#define E       1                                     // Index to East Timer
#define S       2                                     // Index to South Timer
#define W       3                                     // Index to West Timer
#define MISS    4                                     // Timer was a miss

#define PI 3.14159269
#define PI_ON_4 (PI / 4.0d)
#define PI_ON_2 (PI / 2.0d)

/* 
 * FreeETarget functions
 */
void rapid_enable(unsigned int enable);               // Rapid fire enable state
void hello(void);                                     // Say Hello World
void freeETarget_init(void);                          // Get the target software ready
void bye(unsigned int);                               // Shut down and say goodbye
void hello(void);                                     // Wake up and say hello
void tabata_enable(unsigned int enable);              // Arm the Tabata counters

/* 
 * freeRTOS Definitions 
 */
extern const TickType_t json_delay;                   // Poll the serial port at 10Hz


/*
 *  Types
 */
typedef unsigned char byte_t;

struct shot_r
{
  unsigned int shot_number;     // Current shot number
  double       x;               // X location of shot
  double       y;               // Y location of shot
  unsigned int timer_count[8];  // Array of timer values 4 in hardware and 4 in software
  unsigned int face_strike;     // Recording of face strike
  unsigned int sensor_status;   // Triggering register
  unsigned long shot_time;      // Shot time since start of after tabata start
};

typedef struct shot_r shot_record_t;
extern shot_record_t record[SHOT_STRING];      //Array of shot records


/*
 *  Global Variables
 */
extern double  s_of_sound;
extern const char* names[];
extern const char to_hex[];
extern unsigned int face_strike;
extern const char nesw[];             // Cardinal Points
extern unsigned int is_trace;         // Tracing level(s)
extern unsigned int this_shot;        // Index into the shot array
extern unsigned int shot_number;
#endif
