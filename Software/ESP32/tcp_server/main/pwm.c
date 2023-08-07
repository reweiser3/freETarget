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
 * See: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/ledc.html
 * 
 ***************************************************************************/
#include "driver/ledc.h"
#include "esp_err.h"
#include "led_strip.h"
#include "led_strip_types.h"
#include "gpio_define.h"

#define PWM_TIMER              LEDC_TIMER_0
#define PWM_MODE               LEDC_LOW_SPEED_MODE
#define PWM_CHANNEL            LEDC_CHANNEL_0
#define PWM_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define PWM_DUTY_100           ((1 << PWM_DUTY_RES) - 1)   // 100% duty cycle
#define PWM_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

static int pwm_ready = 0;       // Set to 1 when the hardware is programmed

ledc_timer_config_t led_timer = {
    .speed_mode       = PWM_MODE,
    .timer_num        = PWM_TIMER,
    .duty_resolution  = PWM_DUTY_RES,
    .freq_hz          = PWM_FREQUENCY,  // Set output frequency at 5 kHz
    .clk_cfg          = LEDC_AUTO_CLK
    };

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
    ledc_channel_config_t* pwm_channel      // PWM Control
)
{
/*
 * Configure the timer channel
 */
    if ( !pwm_ready )
    {
        ledc_timer_config(&led_timer);     // Setup the timer
        pwm_ready = 1;
    }
/*
 * Configure the output port
 */
    pwm_channel->gpio_num       = pwm_channel->gpio_num;
    pwm_channel->speed_mode     = PWM_MODE;
    pwm_channel->channel        = PWM_CHANNEL;
    pwm_channel->timer_sel      = PWM_TIMER;
    pwm_channel->intr_type      = LEDC_INTR_DISABLE;
    pwm_channel->duty           = 0;                    // Set duty to 0%
    pwm_channel->hpoint         = 0;

    ledc_channel_config(pwm_channel);

/*
 *  Initalize the output
 */
    ledc_set_duty(PWM_MODE, pwm_channel->channel, pwm_channel->duty);
    ledc_update_duty(PWM_MODE, pwm_channel->channel);
/*
 *  All done, 
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
    ledc_channel_t channel,   // Channel being operated on
    int percent               // New duty cycle percentage
)
{
    // Set duty cycle
    ledc_set_duty(PWM_MODE, channel, percent);

    // Update duty to apply the new value
    ledc_update_duty(PWM_MODE, channel);
}
