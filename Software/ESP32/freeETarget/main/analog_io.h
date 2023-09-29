/*----------------------------------------------------------------
 *
 * analog_io.h
 *
 * Header file for Analog IO functions
 *
 *---------------------------------------------------------------*/
#ifndef _ANALOG_IO_H_
#define _ANALOG_IO_H_

/*
 * Global functions
 */
void init_analog_io(void);                  // Setup the analog hardware
unsigned int read_reference(void);          // Read the feedback channel
double temperature_C(void);                 // Temperature in degrees C
unsigned int revision(void);                // Return the board revision
void compute_vset_PWM(double value);        // Reference voltage control loop

void set_LED_PWM(int percent);              // Ramp the PWM duty cycle
void set_LED_PWM_now(unsigned int percent); // Set the PWM duty cycle
void set_vset_PWM(unsigned int value);      // Value to write to PWM

/*
 *  Port Definitions
 */
#define V_REFERENCE  0          // Reference Input
#define V_12_LED     8          // 12 Volt LED input
#define K_12     ((10000.0d + 2200.0d)/ 2200.0d) // Resistor divider
#define ANALOG_VERSION 5        // Analog Version Input
#define LED_PWM      0          // PWM mapped to PWM channel 0
#define vset_PWM     8          // Reference Voltage Control
#define MAX_ANALOG  0x3ff       // Largest analog input
#define MAX_PWM      0xff       // PWM is an 8 bit port

#define TO_VOLTS(x) ( ((double)(x) * 5.0) / 1024.0 )

#define TEMP_IC   (0x9E >> 1)


#endif
