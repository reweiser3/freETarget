/******************************************************************************
 * 
 * analog_io.c
 * 
 * General purpose Analog driver
 * 
 *****************************************************************************/

#include "freETarget.h"
#include "diag_tools.h"
#include "stdio.h"
#include "json.h"
#include "math.h"
#include "gpio.h"
#include "analog_io.h"
#include "nvs.h"
#include "nonvol.h"
#include "serial_io.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"
#include "timer.h"
#include "led_strip.h"
#include "led_strip_types.h"
#include "pwm.h"
#include "gpio_define.h"
#include "i2c.h"
#include <stdio.h>

void set_vset_PWM(unsigned int pwm);

/*----------------------------------------------------------------
 * 
 * function: set_LED_PWM()
 * function: set_LED_PWM_now()
 * 
 * brief: Program the PWM value
 * 
 * return: None
 * 
 *----------------------------------------------------------------
 *
 * json_LED_PWM is a number 0-100 %  It must be scaled 0-255
 * 
 * The function ramps the level between the current and desired
 * 
 *--------------------------------------------------------------*/
static unsigned int old_LED_percent = 0;

void set_LED_PWM_now
  (
  unsigned int new_LED_percent                            // Desired LED level (0-100%)
  )
{
  if ( new_LED_percent == old_LED_percent )
  {
    return;
  }
  
  if ( DLT(DLT_INFO) )
  {
    printf("new_LED_percent: %d  old_LED_percent: %d", new_LED_percent, old_LED_percent);
  }

  old_LED_percent = new_LED_percent;
  pwm_set(LED_PWM, new_LED_percent);  // Write the value out
  
  return;
}
  

void set_LED_PWM                                  // Theatre lighting
  (
  int new_LED_percent                            // Desired LED level (0-100%)
  )
{
  if ( DLT(DLT_INFO) )
  {
    printf("new_LED_percent: %d  old_LED_percent: %d", new_LED_percent, old_LED_percent);
  }

/*
 * Loop and ramp the LED  PWM up or down slowly
 */
  while ( new_LED_percent != old_LED_percent )  // Change in the brightness level?
  {
    //analogWrite(LED_PWM, old_LED_percent * 256 / 100);  // Write the value out
    
    if ( new_LED_percent < old_LED_percent )
    {
      old_LED_percent--;                        // Ramp the value down
    }
    else
    {
      old_LED_percent++;                        // Ramp the value up
    }

    vTaskDelay((unsigned long)ONE_SECOND/50);                       // Worst case, take 2 seconds to get there
  }
  
/*
 * All done, begin the program
 */
  return;
}


/*----------------------------------------------------------------
 * 
 * function: read_feedback(void)
 * 
 * brief: return the reference voltage
 * 
 * return: ADC value of the reference voltage
 * 
 *--------------------------------------------------------------*/
unsigned int read_reference(void)
{
  return 0; // analogRead(V_REFERENCE);
}

/*----------------------------------------------------------------
 * 
 * function: revision(void)
 * 
 * brief: Return the board revision
 * 
 * return: Board revision level
 * 
 *--------------------------------------------------------------
 *
 *  Read the analog value from the resistor divider, keep only
 *  the top 4 bits, and return the version number.
 *  
 *  The analog input is a number 0-1024 which is banded and
 *  used to look up a table of revision numbers.
 *  
 *  To accomodate unknown hardware builds, if the revision is
 *  undefined (< 100) then the last 'good' revision is returned
 *  
 *--------------------------------------------------------------*/
//                                       0      1  2  3  4  5  6  7  8  9   A   B   C   D   E   F
const static unsigned int version[] = {REV_500, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  
unsigned int revision(void)
{
  unsigned int revision;

/* 
 *  Read the resistors and determine the board revision
 */
  revision =   version[0]; // analogRead(ANALOG_VERSION) * 16 / 1024];

/*
 * Nothing more to do, return the board revision
 */
  return revision;
}

/*----------------------------------------------------------------
 * 
 * function: temperature_C()
 * 
 * brief: Read the temperature sensor and return temperature in degrees C
 * 
 *----------------------------------------------------------------
 *
 * See TI Documentation for HDC3022
 * https://www.ti.com/product/HDC3022
 *
 * A simple interrogation is used. 
 * 
 *--------------------------------------------------------------*/
#define RTD_SCALE      (0.5)   // 1/2C / LSB
#define RH_SCALE    (0.5)
static float  t_c;              // Temperature from sensor
static float  rh;               // Humidity from sensor

double temperature_C(void)
{
  int temperature;              // Temperature inside of the packet
  int humidity;                 // Humidity inside of the packet
  unsigned char temp_buffer[10];
  int raw;
/*
 * Read in the temperature and humidity together
 */
  temp_buffer[0] = 0x24;        // Trigger read on demand
  temp_buffer[1] = 0x00;
  i2c_write( TEMP_IC, &temp_buffer, 2 );
  i2c_read( TEMP_IC,  &temp_buffer, 5);

/*
 *  Return the temperature in C
 */
  raw = (temp_buffer[2] << 8) + temp_buffer[3];
  printf(" t_c raw  %d", raw);
  t_c =  (double)(raw) * RTD_SCALE;
  raw = (temp_buffer[4] << 8) + temp_buffer[5];
  printf(" rh raw  %d", raw);
  rh  =  (double)(raw) * RTD_SCALE;

  return t_c;

}

/*----------------------------------------------------------------
 * 
 * function: humidity_RH()
 * 
 * brief: Return the previoudly read humidity
 * 
 * return: Humidity in RH (0-100%)
 *----------------------------------------------------------------
 *
 * See TI Documentation for HDC3022
 * https://www.ti.com/product/HDC3022
 *
 * A simple interrogation is used. 
 * 
 *--------------------------------------------------------------*/
 #define RH_SCALE      (0.5)   // 1/2C / LSB

double humidity_RH(void)
{
  return rh;
}

/*----------------------------------------------------------------
 * 
 * funciton: set_vset_PWM()
 * 
 * brief: Set the PWM value to the hardware
 * 
 *----------------------------------------------------------------
 *
 * The value is previously set when the VREF value is set by the
 * JSON driver.
 *
 *--------------------------------------------------------------*/
 void set_vset_PWM
  (
  unsigned int value           // Value to write to PWM
  )
{
  value &= MAX_PWM;
  // analogWrite(vset_PWM, value);
  return;
}


/*----------------------------------------------------------------
 * 
 * function: set_vref()
 * 
 * brief: Set the refererence voltage for the comparitor
 * 
 *----------------------------------------------------------------
 *
 * See Microchip documentation for MCP4728
 * https://ww1.microchip.com/downloads/en/DeviceDoc/22187E.pdf
 * Figure 5-8
 * 
 *--------------------------------------------------------------*/

void set_VRef
(
  unsigned int channel,         // Channel 0-3 to control
  float        percent          // Percent of full scale
)
{
  unsigned char temp_buffer[10];
  int raw;

/*
 * Write to the selected DAC
 */
  temp_buffer[0] = 0xC0;                          // Single write
  temp_buffer[1] = 0x40 + (channel << 1) + 0x00;  // Multi write + channel + LDAC = 0 (immediate)
  raw = (int)(2047.0 * percent / 100.0);          // Convert percent to 0-2047
  temp_buffer[2] = 0x80 + 0x00 + 0x00 + (raw >> 8);
  temp_buffer[3] =                      (raw & 0xff);
  i2c_write( DAC_IC, &temp_buffer, 4 );

/*
 *  All done, return
 */

  return;

}
