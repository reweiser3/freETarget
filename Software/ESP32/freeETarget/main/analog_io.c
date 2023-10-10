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
#include "driver/adc.h"

void set_vset_PWM(unsigned int pwm);
                    

/*----------------------------------------------------------------
 * 
 * @function: adc_init()
 * 
 * @brief:  Initialize the ADC channel
 * 
 * @return: None
 * 
 *----------------------------------------------------------------
 *
 * The ADC channel is initialized and the handle set up
 * 
 * https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
 * 
 *--------------------------------------------------------------*/  

#define ADC_ATTENUATION  ADC_ATTEN_DB_11  //ADC Attenuation

void adc_init
(
    unsigned int adc_channel,   // What ADC channel are we accessing
    unsigned int adc_gpio       // What GPIO is used
)
{
  unsigned int adc;             // Which ADC (1/2)
  unsigned int channel;         // Which channel attached to the ADC (0-10)

  adc = ADC_CH(adc_channel);    // What ADC are we on
  channel = adc_channel % 10;   // What channel are we using

/*
 * Setup the channel
 */
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    switch (adc)
    {
      case 1: 
        ESP_ERROR_CHECK(adc1_config_channel_atten(channel, ADC_ATTENUATION));
        break;
      
      case 2:
        ESP_ERROR_CHECK(adc2_config_channel_atten(channel, ADC_ATTENUATION));
        break;
    }

/*
 *  Ready to go
 */
    return;
 }

 /*----------------------------------------------------------------
 * 
 * @function: adc_read()
 * 
 * @brief:  Read a value from teh ADC channel
 * 
 * @return: None
 * 
 *----------------------------------------------------------------
 *
 * The ADC channel is initialized and the handle set up
 * 
 * https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32/api-reference/peripherals/adc.html
 * 
 *--------------------------------------------------------------*/ 
unsigned int adc_read
(
  unsigned int adc_channel          // What input are we reading?
)
{
  unsigned int adc;             // Which ADC (1/2)
  unsigned int channel;         // Which channel attached to the ADC (0-10)
           int raw;             // Raw value from the ADC

  adc = ADC_CH(adc_channel);    // What ADC are we on
  channel = adc_channel % 10;   // What channel are we using

/*
 *  Read the appropriate channel
 */
  switch (adc)
  {
    case 1: 
      raw = adc1_get_raw(channel);
      break;

    case 2: 
      adc2_get_raw(channel, ADC_WIDTH_BIT_DEFAULT, &raw);
      break;
  }

/*
 *  Done
 */
  return raw;
}

/*----------------------------------------------------------------
 * 
 * @function: set_LED_PWM()
 * @function: set_LED_PWM_now()
 * 
 * @brief: Program the PWM value
 * 
 * @return: None
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
 * @function: read_feedback(void)
 * 
 * @brief: return the reference voltage
 * 
 * @return: ADC value of the reference voltage
 * 
 *--------------------------------------------------------------*/
unsigned int read_reference(void)
{
  return 0; // analogRead(V_REFERENCE);
}

/*----------------------------------------------------------------
 * 
 * @function: revision(void)
 * 
 * @brief: Return the board revision
 * 
 * @return: Board revision level
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
  int revision;

/* 
 *  Read the resistors and determine the board revision
 */
  revision = adc_read(BOARD_REV) >> (12-4);
  printf("adc voltage %d", revision);
  revision =   version[0]; // analogRead(ANALOG_VERSION) * 16 / 1024];

/*
 * Nothing more to do, return the board revision
 */
  return revision;
}

/*----------------------------------------------------------------
 * 
 * @function: temperature_C()
 * 
 * @brief: Read the temperature sensor and return temperature in degrees C
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
  temp_buffer[0] = 0xAB;
  temp_buffer[1] = 0xCD;
  temp_buffer[2] = 0xDE;
  temp_buffer[3] = 0xF0;
  i2c_read( TEMP_IC,  &temp_buffer, 6);

/*
 *  Return the temperature in C
 */
  raw = (temp_buffer[0] << 8) + temp_buffer[1];
  printf(" t_c raw  %d\r\n", raw);
  t_c = -42.0 + (175.0 * (float)raw / 65535.0);
  raw = (temp_buffer[3] << 8) + temp_buffer[4];
  printf(" rh raw  %d\r\n", raw);
  rh  =  100.0 * (float)raw / 65535.0;

  return t_c;

}

/*----------------------------------------------------------------
 * 
 * @function: humidity_RH()
 * 
 * @brief: Return the previoudly read humidity
 * 
 * @return: Humidity in RH (0-100%)
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
 * @funciton: set_vset_PWM()
 * 
 * @brief: Set the PWM value to the hardware
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
 * @function: set_vref()
 * 
 * @brief: Set the refererence voltage for the comparitor
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
  float        volts            // Voltage to set output
)
{
  unsigned char buffer[10];
  int raw;

  raw = (int)(volts * 1000.0) & 0x0FFF;                 // Volts to  mV (1lsb = 1mV)

/*
 * Write to the selected DAC
 */
  buffer[0] = (0xb << 3) + (channel << 1) + 0x00;  // Multi write + channel + LDAC = 0 (immediate)
  buffer[1] = 0x80                                 // VREF = 2.048 internal
                     | 0x00                        // PD = Normal mode
                     | 0x00                        // Gain = 1
                     | ((raw >> 8) & 0x0f);        // Top 12 bits of the voltage
  buffer[2] = (raw & 0xff);
  i2c_write( DAC_IC, &buffer, 3 );

/*
 *  All done, return
 */
  return;

}
