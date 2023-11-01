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
 * function Prototypes
 */
void freeETarget_timer_init(void);                                                  // Initialize the timers
void freeETimer_timer_pause(void);                                                  // Stop the timer
void freeETimer_timer_start(void);                                                  // Start the timer
unsigned int timer_new(volatile unsigned int* new_timer, unsigned int duration);    // Start a new timer
unsigned int timer_delete(volatile unsigned int* old_timer);                        // Remove a timer
void freeETarget_synchronous(void *pvParameters);                                   // Synchronou scheduler

/*
 *  Definitions
 */
#define timer_delay(t) while((t) != 0) continue;
#endif
