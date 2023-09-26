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
 * Function Prototypes
 */
void init_timer(void);                                          // Initialize the timers
void set_motor_time(unsigned int duration, unsigned int cycles);// Duration in milliseconds
unsigned int timer_new(volatile unsigned long* new_timer, unsigned long start); // Start a new timer
unsigned int timer_delete(volatile unsigned long* old_timer);   // Remove a timer
void delay(unsigned long time_delay);                           // Waste time
void freeETarget_timer(void *pvParameters);                     // 1ms timer task

/*
 *  Definitions
 */

#endif
