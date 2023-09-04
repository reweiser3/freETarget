
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
#define DAC_FS    2048    // DAC Full Scale Voltage
#define V_REF     2.048   // DAC Reference Voltage
#define DAC_ADDR  0x60    // DAC I2C address
#define DAC_WRITE 0x58    // Single write

/*
 *  Functions
 */
void dac_init(void);      // Initialize the DAC
void dac_write(unsigned int channel, float value);
