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
 * @function Prototypes
 */
void    self_test(unsigned int test);
void    show_sensor_status(unsigned int sensor_status); // Display the sensor status as text
void    blink_fault(unsigned int fault_code);           // Blink a fault
void    POST_version(void);                             // Show the version string
bool    POST_counters(void);                            // Verify the counter operation
void    POST_trip_point(void);                          // Display the set point
void    set_trip_point(int x);                          // Calibrate the trip point
bool  do_dlt(unsigned int level);                     // Diagnostics Log and Trace

#define T_HELP         0        // Help test
#define T_DIGITAL      1        // Digital test
#define T_PAPER        2        // Advance paper backer
#define T_LED          3        // Test the LED PWM
#define T_STATUS       4        // Send colours across the status LEDs
#define T_TEMPERATURE  5        // Read temperature
#define T_VREF         6        // Cycle the VREF outputs 
#define T_AIN          7        // Analog Inpt 
#define T_TIMER        8        // Toggle the timer controls
#define T_PCNT         9        // Read the PCNT registers 
#define T_ISR         10        // Test the Timer ISR
#define T_SENSOR      11        // Read the sensor input
#define T_AUX_SERIAL  12        // AUX Serial Port loopback

/*
 * LED status messages
 */

#define LED_RESET         "   "             // Force them all off
#define LED_READY         "G  "             // The shot is ready to go
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
#define LED_FAULT          "--R"            // Generic fault
#define LED_NORTH_FAILED   "RRR"            // North sensor failed
#define LED_EAST_FAILED    "GRR"            // East sensor failed
#define LED_SOUTH_FAILED   "BRR"            // South sensor failed
#define LED_WEST_FAILED    "WRR"            // 3 West sensor failed

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

/*
 *  Variables
 */
extern const char* which_one[4];
#endif
