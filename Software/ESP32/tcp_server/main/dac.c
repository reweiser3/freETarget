
/******************************************************************************
 *
 * file: dac.c
 *
 * Use the external DAC to set the reference voltages
 *
 *****************************************************************************
 *
 * See: https://ww1.microchip.com/downloads/en/DeviceDoc/22187E.pdf
 *
 *****************************************************************************/

#include "freETarget.h"
#include "gpio.h"
#include "diag_tools.h"
#include "nvs.h"
#include "nonvol.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "analog_io.h"
#include "token.h"            // Time provided by the token ring
#include "json.h"
#include "stdio.h"
#include "esp_random.h"
#include "compute_hit.h"
#include "math.h"
#include "serial_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"

#include "timer.h"
#include "esp_timer.h"

/*
 *  Definitions
 */
#define DAC_FS    2048    // DAC Full Scale Voltage
#define DAC_ADDR  0x60    // DAC I2C address
#define DAC_WRITE 0x58    // Single write

/*----------------------------------------------------------------
 *
 * @function: init_dac()
 *
 * @brief:    Initialize the DAC channels
 * 
 * @return: None
 *
 *----------------------------------------------------------------
 *   
 * Set the output based on the JSON settings
 *  
 *--------------------------------------------------------------*/
void init_init(void)
{
    dac_write(DAC_LOW, json_dac_low);
    dac_write(DAC_HIGH, json_dac_high);
    return;
}


/*----------------------------------------------------------------
 *
 * @function: dac_write()
 *
 * @brief:    Write a value (in mV) to the DAC
 * 
 * @return: None
 *
 *----------------------------------------------------------------
 *   
 *   This function sets the DACs to the desired value
 *  
 *--------------------------------------------------------------*/
void dac_write
(
  unsigned int channel,             // What register are we writing to
  unsigned int value                // What value are we setting it to
)
{
  unsigned byte data[3];            // Bytes to send to the I2C
  unsigned int  scaled_value;       // Value (12 bits) to the DAC

  scaled_value = 1024 * value / DAC_FS;

  data[0] = DAC_WRITE + ((channel & 0x3) << 1) + 0; // Write, channel, update now
  data[1] = 0x80                    // Internal 2.048 Volts
                + 0x00              // Normal Power Down
                + 0x00              // Gain x 1
                + scaled_value >> 8;// Top 4 bits of the setting
  data[2] = scaled_value & 0xff;    // Bottom 4 bits of the setting

  control

  i2c_write(DAC_ADDR, data, sizeof(data) );

 /* 
  *  All done, return;
  */
    return;
}
