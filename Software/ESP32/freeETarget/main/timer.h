/*----------------------------------------------------------------
 *
 * gpio.h
 *
 * Header file for GPIO functions
 *
 *---------------------------------------------------------------*/
#ifndef _TIMER_H_
#define _TIMER_H_

/*
 * @function Prototypes
 */
void freeETarget_timer_init(void);                                                  // Initialize the timers
void set_motor_time(unsigned int duration, unsigned int cycles);                    // Duration in milliseconds
unsigned int timer_new(volatile unsigned long* new_timer, unsigned long duration);  // Start a new timer
unsigned int timer_delete(volatile unsigned long* old_timer);                       // Remove a timer

/*
 *  Definitions
 */
#define timer_delay(t) while((t) != 0) continue;
#endif
