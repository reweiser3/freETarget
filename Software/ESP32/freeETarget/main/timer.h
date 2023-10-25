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
void freeETimer_timer_pause(void);                                                  // Stop the timer
void freeETimer_timer_start(void);                                                  // Start the timer
void set_motor_time(unsigned int duration, unsigned int cycles);                    // Duration in milliseconds
unsigned int timer_new(volatile unsigned long* new_timer, unsigned long duration);  // Start a new timer
unsigned int timer_delete(volatile unsigned long* old_timer);                       // Remove a timer
void freeETarget_synchronous( void *pvParameters);                                  // Synchronou scheduler

/*
 *  Definitions
 */
#define timer_delay(t) while((t) != 0) continue;
#endif
