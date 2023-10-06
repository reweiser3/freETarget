/*----------------------------------------------------------------
 *
 * gpio.h
 *
 * Header file for GPIO functions
 *
 *---------------------------------------------------------------*/
#ifndef _GPIO_H_
#define _GPIO_H_

#include "freETarget.h"

/*
 * Global functions
 */
void init_gpio(void);                                     // Initialize the GPIO ports
void arm_timers(void);                                    // Make the board ready
void clear_running(void);                                 // Clear the run flip flop 
unsigned int is_running(void);                            // Return a bit mask of running sensors 
void set_status_LED(char* new_state);                            // Manage the LEDs
unsigned int read_DIP(void);                              // Read the DIP switch register
unsigned int read_counter(unsigned int direction);
void stop_timers(void);                                   // Turn off the counter registers
void trip_timers(void);
bool_t read_in(unsigned int port);                        // Read the selected port
void read_timers(unsigned int* timer_counts);             // Read and return the counter registers
void drive_paper(void);                                   // Turn on the paper motor
void aquire(void);                                        // Read the clock registers
// void enable_face_interrupt();                             // Turn on the face strike interrupt
void disable_face_interrupt(void);                        // Turn off the face strike interrupt
void enable_sensor_interrupt();                           // Turn on the sensor interrupt
void disable_sensor_interrupt(void);                      // Turn off the sensor strike interrupt
void multifunction_init(void);                            // Initialize the multifunction switches
void multifunction_switch(void);                          // Handle the actions of the DIP Switch signal
void multifuction_display(void);                          // Display the MFS settings
void multifunction_wait_open(void);                       // Wait for both multifunction switches to be open
void digital_test(void);                                  // Execute the digital test
void paper_on_off(bool_t on);                             // Turn the motor on or off
void rapid_green(unsigned int state);                     // Drive the GREEN light
void rapid_red(unsigned int state);                       // Drive the RED light
void multifunction_display(void);                         // Display the MFS settings as text
void status_LED_init(unsigned int gpio_number);           // Initialize the RMT driver 

/*
 *  Port Definitions
 */
#define RUN_NORTH_LO   5                  // Address port but locations
#define RUN_EAST_LO    6
#define RUN_SOUTH_LO   7
#define RUN_WEST_LO   49
#define RUN_MASK      0x0f

#define PAPER         12                 // Paper advance drive active low
#define PAPER_ON       0
#define PAPER_OFF      1

#define STOP_N          39      // V      
#define CLOCK_START     41      // V

#define DIP_0           9
#define RED_OUT         9                  // Rapid fire RED on DIP0

#define DIP_A           38      // V
#define DIP_B           37      // V
#define DIP_C           36      // V
#define DIP_D           35      // V
#define RUN_A_MASK  0xf     // All run bits set 

#define GREEN_OUT   12                  // Rapid fire GREEN on DIP3

#define RED_MASK     1                  // Use DIP 0
#define GREEN_MASK   8                  // Use DIP 3

/*
 * Multifunction Switch Use when using DIP Switch for MFS
 */
#define HOLD1(x)    LO10((x))          // Low digit        xxxx2
#define HOLD2(x)    HI10((x))          // High digit       xxx2x
#define TAP1(x)     HLO10((x))         // High Low digit   xx2xx
#define TAP2(x)     HHI10((x))         // High High digit  x2xxx
#define HOLD12(x)   HHH10((x))         // Highest digit    2xxxx

/*
 * DIP Switch Use. 
 */
//                      From DIP             
#define CALIBRATE       (DIP_A)         // 1 Go to Calibration Mode
#define DIP_SW_A        (DIP_B)         // 2 When CALIBRATE is asserted, use lower trip point
#define DIP_SW_B        (DIP_C)         // 4 When CALIBRATE is asserted, use higher trip point
#define VERBOSE_TRACE   (DIP_D)         // 8 Show the verbose software trace



#define FACE_SENSOR  19

/*
 *  MFS Uset
 */
#define POWER_TAP     0                   // DIP A/B used to wake up
#define PAPER_FEED    1                   // DIP A/B used as a paper feed
#define LED_ADJUST    2                   // DIP A/B used to set LED brightness
#define PAPER_SHOT    3                   // DIP A/B Advance paper one cycle
#define PC_TEST       4                   // DIP A/B used to trigger fake shot
#define ON_OFF        5                   // DIP A/B used to turn the target ON or OFF
#define MFS_SPARE_6   6
#define MFS_SPARE_7   7
#define MFS_SPARE_8   8
#define TARGET_TYPE   9                   // Sent target type with score

#define NO_ACTION     0                   // DIP usual function
#define RAPID_RED     1                   // Rapid Fire Red Output
#define RAPID_GREEN   2                   // Rapid Fire Green Output

#define J10_1      VCC
#define J10_2       14                    // TX3
#define J10_3       15                    // RX3
#define J10_4       19                    // RX1
#define J10_5       18                    // TX1
#define J10_6      GND

#endif
