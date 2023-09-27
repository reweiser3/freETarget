/*----------------------------------------------------------------
 *
 * diag_tools.h
 *
 * Debug and test tools 
 *
 *---------------------------------------------------------------*/
#ifndef _DIAG_TOOLS_H_
#define _DIAG_TOOLS_H_
#include "gpio.h"

/*
 * Function Prototypes
 */
void    self_test(unsigned int test);
void    show_sensor_status(unsigned int sensor_status); // Display the sensor status as text
void    blink_fault(unsigned int fault_code);           // Blink a fault
void    POST_version(void);                             // Show the version string
void    POST_LEDs(void);                                // Verify the LED operation
bool_t  POST_counters(void);                            // Verify the counter operation
void    POST_trip_point(void);                          // Display the set point
void    set_trip_point(int x);                          // Calibrate the trip point
bool_t  do_dlt(unsigned int level);                     // Diagnostics Log and Trace

#define T_HELP         0       // Help test
#define T_DIGITAL      1       // Digital test
#define T_PAPER        2       // Advance paper backer
#define T_LED          3       // Test the LED PWM

#define T_TRIGGER      2       // Test microphone trigger
#define T_CLOCK        3       // Trigger clock internally

#define T_SPIRAL       7       // Generate sprial pattern
#define T_GRID         8       // Generate grid pattern
#define T_ONCE         9       // Generate single calculation @ 45North
#define T_PASS_THRU   10       // Serial port pass through
#define T_SET_TRIP    11       // Set the microphone trip point
#define T_XFR_LOOP    12       // Transfer loopback
#define T_SERIAL_PORT 13       // Hello World n

#define T_FACE        15       // Test the face detector
#define T_WIFI        16       // WiFi Test
#define T_NONVOL      17       // Dump Nonvol
#define T_SHOT        18       // Send shot record
#define T_WIFI_STATUS 19       // Obtain the WiFi Status
#define T_WIFI_BROADCAST 20
#define T_SWITCH      25       // Test the switches
#define T_S_OF_SOUND  26       // Test Speed of Sound algorithm
#define T_TOKEN       27       // Token Ring Test
#define T_LED_CYCLE   28       // Blink the 3 LEDs

/*
 * LED status messages
 */

#define LED_RESET         "   "             // Force them all off
#define LED_READY         "G--"             // The shot is ready to go
#define LED_READY_OFF     " --"             // Turn off the READY light
#define LED_TABATA_ON     "--G"             // Tabata is ready to go, leave the others alone
#define LED_TABATA_OFF    "-- "             // Tabata is turned off, leave the others alone
#define LED_RAPID_ON      "--G"             // Rapidfire course of fire on
#define LED_RAPID_OFF     "-- "             // Rapidfire course of fire off
#define LED_HIT           "-G-"             // A shot has been detected
#define LED_MISS          "-R-"             // Last shot was a miss
#define LED_WIFI_SEND     "--B"             // There is something going over the WiFi
#define LED_WIFI_DONE     "-- "             // Finished sending
#define LED_HELLO_WORLD   "RWB"             // Hello World

// Fault Codes
#define LED_FAULT          "R--"            // Generic fault
#define LED_NORTH_FAILED   "RRR"            // North sensor failed
#define LED_EAST_FAILED    "RRG"            // East sensor failed
#define LED_SOUTH_FAILED   "RRB"            // South sensor failed
#define LED_WEST_FAILED    "RRW"            // 3 West sensor failed

#define LED_FAIL_A         "RGR"            // 
#define LED_FAIL_B         "RGG"            // 
#define LED_FAIL_C         "RGB"            // 
#define LED_FAIL_D         "RGW"            // 

#define LED_FAIL_E         "RBR"            // 
#define LED_FAIL_F         "RBG"            // 
#define LED_FAIL_G         "RBB"            // 
#define LED_FAIL_H         "RBW"            // 

#define LED_FAIL_I         "RWR"            // 
#define LED_FAIL_J         "RWG"            // 
#define LED_FAIL_K         "RWB"            // 
#define LED_FAIL_L         "RWW"            // 

/*
 * Tracing 
 */
#define DLT(level)      ( do_dlt(level) )
#define DLT_NONE          0                       // No DLT messages displayed
#define DLT_CRITICAL      0x80                    // Display messages that will compromise the target
#define DLT_APPLICATION   0x01                    // Application level messages displayed
#define DLT_DIAG          0x02                    // Diagnostics messages displayed
#define DLT_INFO          0x04                    // Informational messages

#endif
