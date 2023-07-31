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
 * https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#pcnt
 * 
 ***************************************************************************/
#include <stdio.h>
#include "driver/pulse_cnt.h"
#include "esp_err.h"
#include "gpio_define.h"
#include "pcnt.h"

/*
 *  Working variables
 */
static pcnt_unit_config_t          unit_config[8];         // Single unit configuration
static pcnt_unit_handle_t          pcnt_unit[8];
static pcnt_glitch_filter_config_t filter_config[8];       // Glitch Filter

static pcnt_chan_config_t          chan_a_config[8];       // Counter Configuration A
static pcnt_channel_handle_t       pcnt_chan_a[8];

static pcnt_chan_config_t          chan_b_config[8];       // Counter Configuration B
static pcnt_channel_handle_t       pcnt_chan_b[8];

/*************************************************************************
 * 
 * function: pcnt_init()
 * 
 * description:  Set the pulse counter control
 * 
 * return:  Nope
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
    PCNT_struct_t*  pcnt       // pcnt Control
)
{
    int this_unit;

/*
 * Setup the unit
 */
    this_unit = pcnt->pcnt_unit; 
    unit_config[this_unit].low_limit = 0;
    unit_config[this_unit].high_limit = 0xffff;
    pcnt_unit[this_unit] = NULL;
    pcnt_new_unit(&unit_config[this_unit], &pcnt_unit[this_unit]);

/*
 *  Setup the glitch filter
 */
    filter_config[this_unit].max_glitch_ns = 10;
    pcnt_unit_set_glitch_filter(pcnt_unit[this_unit], &filter_config[this_unit]);

/*
 *  Setup the channel.  Only Channel A is used.  B is left idle
 */
    pcnt_chan_a[this_unit] = NULL;
    chan_a_config[this_unit].edge_gpio_num = pcnt->pcnt_signal;     // Counter
    chan_a_config[this_unit].level_gpio_num = pcnt->pcnt_control;   // Enable 
    pcnt_new_channel(pcnt_unit[this_unit], &chan_a_config[this_unit], &pcnt_chan_a[this_unit]);

    pcnt_chan_b[this_unit] = NULL;
    chan_b_config[this_unit].edge_gpio_num = pcnt->pcnt_signal;
    chan_b_config[this_unit].level_gpio_num = pcnt->pcnt_control;
    pcnt_new_channel(pcnt_unit[this_unit], &chan_b_config[this_unit], &pcnt_chan_b[this_unit]);

/*
 *  Setup the control.  Count only when the control is HIGH.
 */
//                                Channel                       Rising Edge                        Falling Edge
    pcnt_channel_set_edge_action( pcnt_chan_a[this_unit], PCNT_CHANNEL_EDGE_ACTION_INCREASE, PCNT_CHANNEL_EDGE_ACTION_HOLD);  // Counter
//                                Channel                        When High                          When Low
    pcnt_channel_set_level_action(pcnt_chan_a[this_unit], PCNT_CHANNEL_LEVEL_ACTION_KEEP, PCNT_CHANNEL_LEVEL_ACTION_HOLD); // Control
    pcnt_channel_set_edge_action( pcnt_chan_b[this_unit], PCNT_CHANNEL_EDGE_ACTION_HOLD,  PCNT_CHANNEL_EDGE_ACTION_HOLD);       // Not Used
    pcnt_channel_set_level_action(pcnt_chan_b[this_unit], PCNT_CHANNEL_LEVEL_ACTION_HOLD, PCNT_CHANNEL_LEVEL_ACTION_HOLD);    // Not Used

/*
 *  All done, return
 */
    return;
}
