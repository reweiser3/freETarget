/*-------------------------------------------------------
 * 
 * timer_ISR.c
 * 
 * Timer interrupt file
 * 
 * ----------------------------------------------------*/
#include "gpio.h"
#include "json.h"
#include "diag_tools.h"
#include "stdio.h"

/*
 * Definitions
 */
#define FREQUENCY 1000ul                        // 1000 Hz
#define N_TIMERS  12                            // Keep space for 8 timrs
#define PORT_STATE_IDLE 0                       // There are no sensor inputs
#define PORT_STATE_WAIT 1                       // Some sensor inputs are present, but not all
#define PORT_STATE_DONE 2                       // All of the inmputs are present

#define MAX_WAIT_TIME   10                      // Wait up to 10 ms for the input to arrive
#define MAX_RING_TIME   50                      // Wait 50 ms for the ringing to stop

/*
 * Local Variables
 */
static unsigned long* timers[N_TIMERS]; // Active timer list
static unsigned long isr_timer;         // Elapsed time counter
static unsigned int isr_state = PORT_STATE_IDLE;// Current aquisition state

/*-----------------------------------------------------
 * 
 * function: freeETarget_timer
 * 
 * brief:    High speed synchronous task
 * 
 * return:   None
 * 
 *-----------------------------------------------------
 *
 * Timer 1 samples the inputs and when all of the 
 * sendor inputs are present, the counters are
 * read and made available to the software
 * 
 * There are three data aquisition states
 * 
 * IDLE - No inputs are present
 * WAIT - Inputs are present, but we have to wait
 *        for all of the inputs to be present or
 *        timed out
 * DONE - We have read the counters but need to
 *        wait for the ringing to stop
 *        
 * There are three motor control states
 * 
 * IDLE    - Do nothing
 * RUNNING - The motor is turned on for a duration 
 * CYCLE   - Count the number of stepper motor cycles
 * 
 *-----------------------------------------------------*/

void freeETarget_timer
(
    void *pvParameters                          // 1ms Timer Task
)
{
  unsigned int pin;                             // Value read from the port
  unsigned int  i;                              // Iteration counter

  while (1)
  {
/*
 * Decide what to do if based on what inputs are present
 */
    pin = RUN_PORT & RUN_A_MASK;                 // Read in the RUN bits

/*
 * Read the timer hardware based on the ISR state
 */
    switch (isr_state)
    {
      case PORT_STATE_IDLE:                       // Idle, Wait for something to show up
        if ( pin != 0 )                           // Something has triggered
        { 
          isr_timer = MAX_WAIT_TIME;              // Start the wait timer
          isr_state = PORT_STATE_WAIT;            // Got something wait for all of the sensors tro trigger
        }
        break;
          
      case PORT_STATE_WAIT:                       // Something is present, wait for all of the inputs
        if ( (pin == RUN_A_MASK)                  // We have all of the inputs
            || (isr_timer == 0) )                 // or ran out of time.  Read the timers and restart
        { 
          aquire();                               // Read the counters
          clear_running();                        // Reset the RUN flip Flop
          isr_timer = json_min_ring_time;         // Reset the timer
          isr_state = PORT_STATE_DONE;            // and wait for the all clear
        }
        break;
      
      case PORT_STATE_DONE:                       // Waiting for the ringing to stop
        if ( pin != 0 )                           // Something got latched
        {
          isr_timer = json_min_ring_time;
          clear_running();                        // Reset and try later
        }
        else
        {
          if ( isr_timer == 0 )                   // Make sure there is no rigning
          {
            arm_timers();                         // and arm for the next time
            isr_state = PORT_STATE_IDLE;          // and go back to idle
          } 
        }
        break;
    }

/*
 * Refresh the timers
 */
    for (i=0; i != N_TIMERS; i++)
    {
      if ( (timers[i] != 0)
          && ( *timers[i] != 0 ) )
      {
        (*timers[i])--;
      }
    }

/*
 * Timeout and loop again
 */
  vTaskDelay( json_delay );
  }

}


/*-----------------------------------------------------
 * 
 * function: timer_new()
 *           timer_delete()
 * 
 * brief:    Add or remove timers
 *  
 * return:   TRUE if the operation was a success
 * 
 *-----------------------------------------------------
 *
 * The timer interrupt has the ability to manage a 
 * count down timer
 * 
 * These functions add or remove a timer from the active
 * timer list
 * 
 * IMPORTANT
 * 
 * The timers should be static variables, otherwise they
 * will overflow the available space every time they are
 * instantiated.
 *-----------------------------------------------------*/
unsigned int timer_new
(
  unsigned long* new_timer,         // Pointer to new down counter
  unsigned long  start_time         // Starting value
)
{
  unsigned int i;

  for (i=0;  i != N_TIMERS; i++ )   // Look through the space
  {
    if ( (timers[i] == 0)           // Got an empty timer slot
      || (timers[i] == new_timer) ) // or it already exists
    {
      timers[i] = new_timer;        // Add it in
      *timers[i] = start_time;
      return 1;
    }
  }

  if ( DLT(DLT_CRITICAL) )
  {
    printf("No space for timer");
  }
  
  return 0;
}

 unsigned int timer_delete
(
  unsigned long* old_timer          // Pointer to new down counter
)
{
  unsigned int i;

  for (i=0;  i != N_TIMERS; i++ )   // Look through the space
  {
    if ( timers[i] == old_timer )   // Found the existing timer
    {
      timers[i] = 0;                // Add it in
      return 1;
    }
  }

  return 0;
  
}
