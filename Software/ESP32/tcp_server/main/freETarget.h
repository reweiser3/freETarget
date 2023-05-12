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

#define SOFTWARE_VERSION "\"0.0.0 May 12, 2023\""

#define REV_100    100
#define REV_210    210
#define REV_220    220
#define REV_290    290
#define REV_300    300   // Third Generation
#define REV_310    310   // Onboard Hi Pass Filter
#define REV_320    320   // No USB adapter
#define REV_400    400   // ESP32

#define INIT_DONE       0xabcd                    // Initialization complete signature
#define true        (1==1)
#define false       (0==1)
/*
 * Options
 */
#define SAMPLE_CALCULATIONS false                 // Trace the COUNTER values


/*
 * Three way Serial Port
 */
#define AUX_SERIAL         Serial3    // Auxilary Connector

/*
 * Oscillator Features
 */
#define OSCILLATOR_MHZ   8.0                          // 8000 cycles in 1 ms
#define CLOCK_PERIOD  (1.0/OSCILLATOR_MHZ)            // Seconds per bit
#define ONE_SECOND      1000L                         // 1000 ms delay 
#define ONE_SECOND_US   1000000u                      // One second in us
#define SECONDS       (millis()/1000)                 // Elapsed time in seconds
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

/*
 *  Types
 */
typedef unsigned int bool_t;
typedef unsigned char byte_t;

struct shot_r
{
  unsigned int shot_number;     // Current shot number
  double       x;               // X location of shot
  double       y;               // Y location of shot
  unsigned int timer_count[4];  // Array of timer values
  unsigned int face_strike;     // Recording of face strike
  unsigned int sensor_status;   // Triggering register
  unsigned long shot_time;      // Shot time since start of after tabata start
};

typedef struct shot_r shot_record_t;

struct GPIO {
  byte_t port;
  char* gpio_name;
  byte_t in_or_out;
  byte_t value;
};

typedef struct GPIO GPIO_t;


/*
 *  Global Variables
 */
extern double  s_of_sound;
extern const char* names[];
extern const char to_hex[];
extern unsigned int face_strike;
extern const char nesw[];             // Cardinal Points

/*
 *  Factory settings via Arduino monitor
 */
/*
#define FACTORY        {"NAME_ID":1, "TRGT_1_RINGx10":1550, "ECHO":2}
#define FACTORY_BOSS   {"NAME_ID":1, "TRGT_1_RINGx10":1550, "ECHO":2}
#define FACTORY_MINION {"NAME_ID":2, "TRGT_1_RINGx10":1550, "ECHO":2}
#define SERIAL_NUMBER  {"NAME_ID":1, "TRGT_1_RINGx10":1550, "SN":1234, "ECHO":2}
#define LONG_TEST      {"SENSOR":231, "Z_OFFSET":5, "STEP_TIME":50, "STEP_COUNT":0, "NORTH_X":0, "NORTH_Y":0, "EAST_X":0, "EAST_Y":0, "SOUTH_X":0, "SOUTH_Y":0, "WEST_X":0, "WEST_Y":0, "LED_BRIGHT":50, "NAME_ID":0, "ECHO":9}
*/

#endif
