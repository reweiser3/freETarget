/*----------------------------------------------------------------
 *
 * pwm.j
 *
 * Header file for GPIO functions
 *
 *---------------------------------------------------------------*/
#ifndef _PWM_H_
#define _PWM_H_

#include "freETarget.h"

/*
 * Global functions
 */
void pwm_init(ledc_channel_config_t* pwm_channel, int gpio_uses);        // Initialize the PWM port
void pwm_set(void);                                       // Set a new PWM duty cycle


#endif
