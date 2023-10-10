/*************************************************************************
 * 
 * file: pcnt.c
 * 
 * description:  Pulse counter control
 * 
 **************************************************************************
 *
 * This file sets up the timers and routing for the PCNT control
 * 
 * See:
 * https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/pcnt.html
 * https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf#pcnt
 * 
 ***************************************************************************/
#include <stdio.h>
#include "driver/pulse_cnt.h"
#include "esp_err.h"
#include "led_strip.h"
#include "led_strip_types.h"
#include "driver\ledc.h"
#include "gpio_define.h"
#include "pcnt.h"
#include "diag_tools.h"

/*
 *  Working variables
 */
static pcnt_unit_config_t          unit_config[SOC_PCNT_UNITS_PER_GROUP];    // Single unit configuration
static pcnt_unit_handle_t          pcnt_unit[SOC_PCNT_UNITS_PER_GROUP];
static pcnt_glitch_filter_config_t filter_config[SOC_PCNT_UNITS_PER_GROUP];  // Glitch Filter

static pcnt_chan_config_t          chan_a_config[SOC_PCNT_UNITS_PER_GROUP];  // Counter Configuration A
static pcnt_channel_handle_t       pcnt_chan_a[SOC_PCNT_UNITS_PER_GROUP];

static pcnt_chan_config_t          chan_b_config[SOC_PCNT_UNITS_PER_GROUP];  // Counter Configuration B
static pcnt_channel_handle_t       pcnt_chan_b[SOC_PCNT_UNITS_PER_GROUP];

/*************************************************************************
 * 
 * @function: pcnt_init()
 * 
 * description:  Set the pulse counter control
 * 
 * @return:  Nope
 * 
 **************************************************************************
 *
 * The PCNT control for FreeETarget is set up as
 * 
 * Channel A Count connected to 10MHz Clock.  Count on rising edge
 * Channel A Control connected to FlipFlop.   Count on high level
 * 
 * Channel B Count disabled
 * Channel B Control disabled
 * 
 **************************************************************************/
void pcnt_init
(
    int  unit,                           // What unit to use
    int  control,                        // GPIO associated with PCNT control
    int  signal                          // GPIO associated with PCNT signal
)
{
/*
 * Setup the unit
 */
    unit_config[unit].low_limit  = -0x7fff;
    unit_config[unit].high_limit = 0x7fff;
    pcnt_unit[unit] = NULL;
    pcnt_new_unit(&unit_config[unit], &pcnt_unit[unit]);

/*
 *  Setup the glitch filter
 */
    filter_config[unit].max_glitch_ns = 10;
    pcnt_unit_set_glitch_filter(pcnt_unit[unit], &filter_config[unit]);

/*
 *  Setup the channel.  Only Channel A is used.  B is left idle
 */
    pcnt_chan_a[unit] = NULL;
    chan_a_config[unit].edge_gpio_num = signal;     // Counter
    chan_a_config[unit].level_gpio_num = control;   // Enable 
    pcnt_new_channel(pcnt_unit[unit], &chan_a_config[unit], &pcnt_chan_a[unit]);

    pcnt_chan_b[unit] = NULL;
    chan_b_config[unit].edge_gpio_num = signal;
    chan_b_config[unit].level_gpio_num = control;
    pcnt_new_channel(pcnt_unit[unit], &chan_b_config[unit], &pcnt_chan_b[unit]);

/*
 *  Setup the control.  Count only when the control is HIGH.
 */
//                                Channel                       Rising Edge                        Falling Edge
    pcnt_channel_set_edge_action( pcnt_chan_a[unit], PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);    // Counter
//                                Channel                        When High                          When Low
    pcnt_channel_set_level_action(pcnt_chan_a[unit], PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_HOLD);      // Control
//  Not Used
    pcnt_channel_set_edge_action( pcnt_chan_b[unit], PCNT_CHANNEL_EDGE_ACTION_HOLD,  PCNT_CHANNEL_EDGE_ACTION_HOLD);       // Not Used
    pcnt_channel_set_level_action(pcnt_chan_b[unit], PCNT_CHANNEL_LEVEL_ACTION_HOLD, PCNT_CHANNEL_LEVEL_ACTION_HOLD);      // Not Used

/*
 *  All done, Clear the counter and return
 */
    pcnt_unit_clear_count(pcnt_unit[unit]); 
    return;
}

/*************************************************************************
 * 
 * @function: pcnt_read()
 * 
 * description:  Read the pcnt register
 * 
 * @return:   32 bit timer count
 * 
 **************************************************************************
 *
 * The PCNT registers are 16 bits long with an overflow counter
 * 
 **************************************************************************/
#define PCNT_BASE 0x60017000+0x00030    // Pointer to start of PCNT registers

unsigned int pcnt_read
(
    int unit                   // What timer to read
)
{
    int value;

    pcnt_unit_get_count(pcnt_unit[unit], &value);

    return value;
}

/*************************************************************************
 * 
 * @function: pcnt_clear()
 * 
 * description:  Clear the pcnt register
 * `
 * @return:   Nothing
 * 
 **************************************************************************
 *
 * Clear the indicated counter
 * 
 **************************************************************************/
void pcnt_clear
(
    int unit                   // What timer to clear
)
{
    pcnt_unit_clear_count(pcnt_unit[unit]);

    return;
}
