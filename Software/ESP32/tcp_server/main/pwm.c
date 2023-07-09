/*************************************************************************
 * 
 * file: pwm.c
 * 
 * description:  PWM driver for LEDs and Voltag Reference
 * 
 **************************************************************************
 *
 * This file sets up the timers and routing for the PWM control
 * 
 ***************************************************************************/
#include <stdio.h>
#include "driver/ledc.h"
#include "esp_err.h"

#define PWM_TIMER              LEDC_TIMER_0
#define PWM_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (5) // Define the output GPIO
#define PWM_CHANNEL            LEDC_CHANNEL_0
#define PWM_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define PWM_DUTY_100           ((1 << PWM_DUTY_RES) - 1)   // 100% duty cycle
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

/*
*/
ledc_timer_config_t pwm_timer = {
    .speed_mode       = PWM_MODE,
    .timer_num        = PWM_TIMER,
    .duty_resolution  = PWM_DUTY_RES,
    .freq_hz          = PWM_FREQUENCY,  // Set output frequency at 5 kHz
    .clk_cfg          = PWMC_AUTO_CLK
    };


    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t pwm_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

/*************************************************************************
 * 
 * function: pwm_init()
 * 
 * description:  Set the PWM duty cycle
 * 
 * return:  Nope
 * 
 **************************************************************************
 *
 * The PWM registers are updated for the new duty cycle
 * 
 ***************************************************************************/
void pwm_init
(
    ledc_channel_config_t* pwm_channel,   // PWM Control
    int                    pwm_gpio       // GPIO PWM belongs to
)
{
/*
 * Configure the timer channel
 */
    ESP_ERROR_CHECK(ledc_timer_config(&pwm_timer));     // Setup the timer

/*
 * Configure the output port
 */
    pwm-channel->gpio_num       = pwm_gpio;
    pwm_channel->speed_mode     = LEDC_MODE;
    pwm_channel->channel        = LEDC_CHANNEL;
    pwm_channel->timer_sel      = LEDC_TIMER;
    pwm_channel->intr_type      = LEDC_INTR_DISABLE;
    pwm_channel->duty           = 0;                    // Set duty to 0%
    pwm_channel->hpoint         = 0;

    ESP_ERROR_CHECK(ledc_channel_config(&pwm_channel));

/*
 *  All done, return
 */
    return;
}

/*************************************************************************
 * 
 * function: pwm_set()
 * 
 * description:  Set the PWM duty cycle
 * 
 * return:  Nope
 * 
 **************************************************************************
 *
 * The PWM registers are updated for the new duty cycle
 * 
 ***************************************************************************/
void pwm_set
(
    int channel,        // Channel being operated on
    int percent         // New duty cycle percentage
)
{
    // Set the LEDC peripheral configuration
    example_ledc_init();
    // Set duty to 50%
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY));
    // Update duty to apply the new value
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
}
