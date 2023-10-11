
/******************************************************************************
 *
 * @file: dac.h
 * 
 * Common interface to the DAC
 * 
 *****************************************************************************
 *
 * See: https://ww1.microchip.com/downloads/en/DeviceDoc/22187E.pdf
 *
 *****************************************************************************/

/*
 *  Definitions
 */
#define DAC_ADDR  0x60    // DAC I2C address
#define DAC_WRITE 0x58    // Single write
#define DAC_LOW   0       // Channel 0
#define DAC_HIGH  1       // Channel 1

/*
 *  Functions
 */
void dac_init(int not_used);      // Initialize the DAC
void dac_write(unsigned int channel, float value);
