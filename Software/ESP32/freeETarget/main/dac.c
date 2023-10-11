/******************************************************************************
 *
 * file: dac.c
 *
 * Use the external DAC to set the reference voltages
 *
 *****************************************************************************
 *
 * See: https://www.microchip.com/en-us/product/MCP4728#document-table
 * https://ww1.microchip.com/downloads/aemDocuments/documents/OTH/ProductDocuments/DataSheets/22187E.pdf
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
#include "gpio.h"
#include "gpio_define.h"
#include "driver\gpio.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"

#include "timer.h"
#include "esp_timer.h"
#include "i2c.h"
#include "dac.h"

/*
 *  Definitions
 */
#define DAC_ADDR  0x60    // DAC I2C address
#define DAC_WRITE 0x58    // Single write

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
#define V_REF 2.048
#define DAC_FS 4095.0

void dac_write
(
  unsigned int channel,               // What register are we writing to
  float        volts                  // What value are we setting it to
)
{
  unsigned char data[10];             // Bytes to send to the I2C
  unsigned int  scaled_value;         // Value (12 bits) to the DAC

  scaled_value = ((int)(volts / V_REF * DAC_FS)) & 0xfff;  // Figure the bits to send

  if ( DLT(DLT_CRITICAL) )
  {
    printf("dac_write(channel:%d scale: %d)", channel, scaled_value);
  }

  data[0] = DAC_WRITE + ((channel & 0x3) << 1) + 0; // Write, channel, update now
  data[1] = 0x80                      // Internal 2.048 Volts
                + 0x00                // Normal Power Down
                + 0x00                // Gain x 1
                + ((scaled_value >> 8) & 0x0f);// Top 4 bits of the setting
  data[2] = scaled_value & 0xff;      // Bottom 4 bits of the setting

  i2c_write(DAC_ADDR, data, 3 );
  gpio_set_level(LDAC, 0);
  gpio_set_level(LDAC, 1);

 /* 
  *  All done, return;
  */
    return;
}
