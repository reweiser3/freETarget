/*-------------------------------------------------------
 * 
 * file: timer_ISR.c
 * 
 * Timer interrupt file
 * 
 *-------------------------------------------------------
 *
 * The timer interrupt is used to generate internal timers 
 * and poll the sensor inputs looking for shot detection
 * 
 * See:
 * https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/timer.html
 * 
 * ----------------------------------------------------*/
#include "stdbool.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "gpio.h"
#include "json.h"
#include "diag_tools.h"
#include "stdio.h"
#include "driver\gpio.h"
#include "driver\timer.h"


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
static unsigned long* timers[N_TIMERS];         // Active timer list
static unsigned int isr_state;                  // What sensor state are we in 
static unsigned int isr_timer;                  // Interrupt timer 

/*
 *  Function Prototypes
 */
static bool IRAM_ATTR freeETarget_timer_isr_callback(void *args);
   

/*-----------------------------------------------------
 * 
 * @function: freeETarget_timer_init
 * 
 * @brief:    Initialize the timer interrupt
 * 
 * @return:   None
 * 
 *-----------------------------------------------------
 *
 * The FreeETarget software uses the FreeRTOS system calls
 * to generate the cycle times needed to run the software
 * Unfortunatly, the FreeRTOS cycle time is 10 ms which is
 * too slow (infrequent) to manage the shot sensors 
 * correctly.  For this reason the sensor polling is done
 * by a 1 ms timer interrupt directly from the operating
 * system
 * 
 *-----------------------------------------------------*/

#define TIMER_DIVIDER         (16)                    //  Hardware timer clock divider
#define TIMER_SCALE           (1000/ TIMER_DIVIDER)   // convert counter value to seconds
#define ONE_MS                (80 * TIMER_SCALE)      // 1 ms timer interrupt

  timer_config_t config = {
      .clk_src      = RMT_CLK_SRC_APB,
      .divider      = TIMER_DIVIDER,
      .counter_dir  = TIMER_COUNT_UP,
      .counter_en   = TIMER_PAUSE,
      .alarm_en     = TIMER_ALARM_EN,
      .auto_reload  = 1,
  }; // default clock source is APB

void freeETarget_timer_init(void)
{
  timer_init(TIMER_GROUP_0, TIMER_1, &config);
  timer_set_counter_value(TIMER_GROUP_0, TIMER_1, 0);                   // Start the timer at 0
  timer_set_alarm_value(TIMER_GROUP_0, TIMER_1, ONE_MS);   // Trigger on this value
  timer_enable_intr(TIMER_GROUP_0, TIMER_1);                            // Interrupt associated with this interrupt
  timer_isr_callback_add(TIMER_GROUP_0, TIMER_1, freeETarget_timer_isr_callback, NULL, 0);
  timer_start(TIMER_GROUP_0, TIMER_1);

/*
 *  Timer running. return
 */
  return;
}

/*-----------------------------------------------------
 * 
 * @function: freeETarget_timer_isr_callback
 * 
 * @brief:    High speed synchronous task
 * 
 * @return:   None
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
static bool IRAM_ATTR freeETarget_timer_isr_callback(void *args)
{
  BaseType_t high_task_awoken = pdFALSE;
  unsigned int pin;                             // Value read from the port
  static unsigned int  i;                       // Iteration counter

/*
 * Decide what to do if based on what inputs are present
 */
  pin = is_running();                         // Read in the RUN bits

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
      if ( (pin == RUN_MASK)                    // We have all of the inputs
          || (isr_timer == 0) )                 // or ran out of time.  Read the timers and restart
       { 
        aquire();                               // Read the counters
        isr_timer = json_min_ring_time;         // Reset the timer
        isr_state = PORT_STATE_DONE;            // and wait for the all clear
      }
      break;
      
    case PORT_STATE_DONE:                       // Waiting for the ringing to stop
      if ( pin != 0 )                           // Something got latched
      {
        isr_timer = json_min_ring_time;
        stop_timers();                          // Reset and try later
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
 * Return from interrupts
 */
  return high_task_awoken == pdTRUE; // return whether we need to yield at the end of ISR
}

/*-----------------------------------------------------
 * 
 * @function: timer_new()
 *            timer_delete()
 * 
 * @brief:    Add or remove timers
 *  
 * @return:   TRUE if the operation was a success
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
  unsigned long  duration           // Duration of the timer
)
{
  unsigned int i;

  for (i=0;  i != N_TIMERS; i++ )   // Look through the space
  {
    if ( (timers[i] == 0)           // Got an empty timer slot
      || (timers[i] == new_timer) ) // or it already exists
    {
      timers[i] = new_timer;        // Add it in
      *timers[i] = duration;
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
/*-----------------------------------------------------
 * 
 * @function: freeETarget_synchronous
 * 
 * @brief:    Synbchronous task scheduler
 *  
 * @return:   None
 * 
 *-----------------------------------------------------
 *
 * This is a simple task scheduler to run tasks at a 
 * controlled rate.
 * 
 *-----------------------------------------------------*/

void freeETarget_synchronous
(
    void *pvParameters              // Select IPV4 or 6
)
{
  static int cycle_count;
  static int cycle_count_500ms;
  static int toggle;

  cycle_count = 0;
  cycle_count_500ms =0;
  toggle = 0;

  while (1)
  {
    cycle_count_500ms = cycle_count % 50;
    if ( cycle_count_100ms == 0 )
    {
      commit_status_LEDs( toggle );
      toggle = !toggle;
    }
    cycle_count++;
    vTaskDelay(1);
  }

}