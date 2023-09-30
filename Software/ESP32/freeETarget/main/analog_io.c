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
 * See TI Documentation for LM75
 * http://www.ti.com/general/docs/suppproductinfo.tsp?distId=10&gotoUrl=http%3A%2F%2Fwww.ti.com%2Flit%2Fgpn%2Flm75b
 *
 * The output of the LM75 is a signed nine bit number 
 * -55C < temp < 125C
 * 
 *--------------------------------------------------------------*/
 #define RTD_SCALE      (0.5)   // 1/2C / LSB

double temperature_C(void)
{
  double return_value;
  int raw;                // Allow for negative temperatures

  raw = 0xffff;
 #if (0)

/*
 *  Point to the temperature register
 */
  Wire.beginTransmission(TEMP_IC);
  Wire.write(0);
  Wire.endTransmission();

/*
 * Read in the temperature register
 */
  Wire.requestFrom(TEMP_IC, 2);
  raw = Wire.read();
  raw <<= 8;
  raw += Wire.read();
  raw >>= 7;
  
  if ( raw & 0x0100 )
    {
    raw |= 0xFF00;      // Sign extend
    }

#endif 

/*
 *  Return the temperature in C
 */
  return_value =  (double)(raw) * RTD_SCALE ;
  
#if (SAMPLE_CALCULATIONS )
  return_value = 23.0;
#endif
    
  return return_value;

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
 * funciton: compute_vset_PWM()
 * 
 * brief: Use a control loop to figure out the PWM setting
 * 
 *----------------------------------------------------------------
 *
 * The value is previously set when the VREF value is set by the
 * JSON driver.
 *
 *--------------------------------------------------------------*/
 void compute_vset_PWM
  (
  double vset                      // Desired control voltage
  )
{
  int vref_raw;                     // Raw VREF value read from ADC
  int vref_desired;                 //
  int vref_error;                   // Difference between desired and raw
  unsigned int pwm;                 // Value written to the PWM
  int cycle_count;
  char s[128];
  char svref[15], svset[15];
/*
 * Compute the initial values
 */
  vref_desired = (int)(json_vset * 1023.0 / 5.0);// Scaled desired VREF
  pwm = json_vset_PWM;              // Starting value
  set_vset_PWM(json_vset_PWM);
  
/*
 * Loop until the error converges
 */
  cycle_count = 0;
  while ( 1 )
  {
    vref_raw = 0; // analogRead(V_REFERENCE);
    vref_error = vref_desired - vref_raw;
    pwm -= vref_error/4;
    if ( pwm > 255 )
    {
      pwm = 255;
    }
    sprintf(s, "\r\nDesired: %s VREF: %s as read %d wanted %d error %d PWM %d", svset, svref, vref_raw, vref_desired, vref_error, pwm);
    serial_to_all(s, ALL);
    
    if ( abs(vref_error) <= 2 )
    {
      break;
    }

    set_vset_PWM(pwm);
    vTaskDelay(2*ONE_SECOND);
    cycle_count++;
    if ( cycle_count > 100 )
    {
      sprintf(s, "\r\nvset_PWM exceeded.  Set value using CAL function and try again");
      serial_to_all(s, ALL);
      return;
    }
  }

 /*
  * Got the right control value, sa it and exit
  */
    serial_to_all("\r\nDone\r\n", ALL);
    json_vset_PWM = pwm;
    nvs_set_i32(my_handle, NONVOL_VSET, json_vset_PWM);
    return;
} 
 
