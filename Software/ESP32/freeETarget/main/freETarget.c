/*----------------------------------------------------------------
 * 
 * freETarget        
 * 
 * Software to run the Air-Rifle / Small Bore Electronic Target
 * 
 *-------------------------------------------------------------*/
#include "esp_timer.h"
#include "driver\gpio.h"
#include "esp_random.h"
#include "stdio.h"
#include "math.h"
#include "nvs.h"
#include "mpu_wrappers.h"
#include "freETarget.h"
#include "gpio.h"
#include "compute_hit.h"
#include "analog_io.h"
#include "json.h"
#include "nonvol.h"
#include "mechanical.h"
#include "diag_tools.h"
#include "timer.h"
#include "token.h"
#include "serial_io.h"
#include "dac.h"
#include "pcnt.h"


/*
 *  Function Prototypes
 */
static long   tabata(bool reset_time);// Tabata state machine
static unsigned int set_mode(void);     // Set the target running mode
static unsigned int arm(void);          // Arm the circuit for a shot
static unsigned int wait(void);         // Wait for the shot to arrive
static unsigned int reduce(void);       // Reduce the shot data
static unsigned int finish(void);       // Finish uip and start over  
static bool discard_shot(void);         // In TabataThrow away the shot
static void freeETarget_task(void);
extern void gpio_init(void);

/*
 *  FreeRTOS Settings
 */
const TickType_t json_delay        = (ONE_SECOND/10);   // Poll the serial port at 10Hz
const TickType_t timer_delay       = (ONE_SECOND/1000); // Poll the timer every ms
const TickType_t freeETarget_delay = (ONE_SECOND/1000); // Run FreeTimer every ms

/*
 *  Variables
 */

shot_record_t record[SHOT_STRING];      //Array of shot records
unsigned int this_shot;                 // Index into the shot array
unsigned int last_shot;                 // Last shot processed.

double        s_of_sound;               // Speed of sound
unsigned int  shot = 0;                 // Shot counter
unsigned int  face_strike = 0;          // Miss Face Strike interrupt count
unsigned int  is_trace = 0;             // Turn off tracing
unsigned long rapid_on = 0;             // Duration of rapid fire event


unsigned int  rapid_count = 0;          // Number of shots to be expected in Rapid Fire
unsigned int  tabata_state;             // Tabata state
unsigned int  shot_number;              // Shot Identifier
volatile unsigned long  in_shot_timer;  // Time inside of the shot window

static volatile unsigned long  keep_alive;        // Keep alive timer
static volatile unsigned long  state_timer;       // Free running state timer
       volatile unsigned long  power_save;        // Power save timer

const char* names[] = { "TARGET",                                                                                           //  0
                        "1",      "2",        "3",     "4",      "5",       "6",       "7",     "8",     "9",      "10",    //  1
                        "DOC",    "DOPEY",  "HAPPY",   "GRUMPY", "BASHFUL", "SNEEZEY", "SLEEPY",                            // 11
                        "RUDOLF", "DONNER", "BLITZEN", "DASHER", "PRANCER", "VIXEN",   "COMET", "CUPID", "DUNDER",          // 18  
                        "ODIN",   "WODEN",   "THOR",   "BALDAR",                                                            // 26
                        0};
                  
const char    nesw[]   = "NESW";                  // Cardinal Points
const char    to_hex[] = "0123456789ABCDEF";      // Quick Hex to ASCII

/*----------------------------------------------------------------
 * 
 * @function: freeETarget_init()
 * 
 * @brief: Initialize the board and prepare to run
 * 
 * @return: None
 * 
 *--------------------------------------------------------------*/

void freeETarget_init(void)
{    
/*
 *  Setup the serial port
 */
  serial_io_init();
  POST_version();                         // Show the version string on all ports
  gpio_init();  
  set_status_LED(LED_HELLO_WORLD);        // Hello World
  vTaskDelay(ONE_SECOND);
  read_nonvol();
  
/*
 *  Set up the port pins
 */
  timer_new(&keep_alive,    (unsigned long)json_keep_alive * ONE_SECOND); // Keep alive timer
  timer_new(&state_timer,   0);                                           // Free running state timer
  timer_new(&in_shot_timer, FULL_SCALE);                                  // Time inside of the shot window
  timer_new(&power_save,    (unsigned long)(json_power_save) * (long)ONE_SECOND * 60L);// Power save timer

/*
 *  Finish setting up special IO
 */
  set_VREF();
 
/*
 * Initialize variables
 */
  tabata(true);                          // Reset the Tabata timers

/*
 * Run the power on self test
 */
  if ( POST_counters() == false )         // If the timers fail
  {
    DLT(DLT_CRITICAL);
    printf("POST_counters failed\r\n");   // Failed the test
    vTaskDelay(2*ONE_SECOND);
  }
  
/*
 * Initialize the WiFi or token ring if available
 */
//   esp01_init();                         // Prepare the WiFi channel if installed
   
/*
 * Ready to go
 */ 
  show_echo();
  set_LED_PWM(json_LED_PWM);
  set_status_LED(LED_READY);              // to a client, then the RDY light is steady on
  serial_flush(ALL);                      // Get rid of everything
  this_shot = 0;                          // Clear out any junk
  last_shot = 0;
  DLT(DLT_CRITICAL);
  printf("Initialization complete");

/*
 * Start the tasks running
 */
  return;
}

/*----------------------------------------------------------------
 * 
 * @function: freeEtarget_task
 * 
 * @brief: Main control loop
 * 
 * @return: None
 * 
 *----------------------------------------------------------------
 */
#define START      0                  // Set the operating mode
#define WAIT       (START+1)          // ARM the circuit and wait for a shot
#define REDUCE     (WAIT+1)           // Reduce the data and send the score

unsigned int state = START;
unsigned int  sensor_status;          // Record which sensors contain valid data
unsigned int  location;               // Sensor location 

char* loop_name[] = {"SET_MODE", "ARM", "WAIT", "AQUIRE", "REDUCE", "FINISH" };

void freeETarget_target_loop(void* arg)
{
  while(1)
  {
/*
 * First thing, handle polled devices
 */
    tabata(false);                  // Update the Tabata state

/*
 * Cycle through the state machine
 */
  
    switch (state)
    {
      default:
      case START:    // Start of the loop
        set_mode();
        arm();
        state = WAIT;
        break;
    
      case WAIT:
        if ( wait() == REDUCE )
        {
          reduce();
          state = START;
        }
        break;
    }
  
/*
 * End of the loop. timeout till the next time
 */
    vTaskDelay(1);
  }
}

/*----------------------------------------------------------------
 * 
 * @function: set_mode()
 * 
 * @brief: Set up the modes for the next string of shots
 * 
 * @return: Exit to the ARM state
 * 
 *----------------------------------------------------------------
 *
 * The shot cycle is about to start.
 * 
 * This initializes the variables.
 * 
 * The illumination LED is set depending on whether or not 
 * the Tabata or Rapid fire feature is enabled
 *
 *--------------------------------------------------------------*/
 unsigned int set_mode(void)
 {
  unsigned int i;

  rapid_on  = 0;                    // Turn off the timer
    
  for (i=0; i != SHOT_STRING; i++)
  {
    record[i].face_strike = 100;    // Disable the shot record
  }

  if ( json_tabata_enable || json_rapid_enable ) // If the Tabata or rapid fire is enabled, 
  {
    set_LED_PWM_now(0);             // Turn off the LEDs
    set_status_LED(LED_TABATA_ON);  // Just turn on X
    json_rapid_enable = 0;          // Disable the rapid fire
  }                                 // Until the session starts
  else
  {
    set_LED_PWM(json_LED_PWM);      // Keep the LEDs ON
  }

/*
 * Proceed to the ARM state
 */
  return WAIT;                      // Carry on to the target
 }
    
/*----------------------------------------------------------------
 * 
 * @function: arm()
 * 
 * @brief:  Arm the circuit and check for errors
 * 
 * @return: Exit to the WAIT state if the circuit is ready
 *         Stay in the ARM state if a hardware fault was detected
 * 
 *----------------------------------------------------------------
 *
 * The shot cycle is about to start.
 * 
 * This initializes the variables.
 * 
 * The illumination LED is set depending on whether or not 
 * the Tabata or Rapid fire feature is enabled
 *
 *--------------------------------------------------------------*/
static unsigned long blink = 0;
unsigned int arm(void)
{
  face_strike = 0;                  // Reset the face strike count
  if ( json_send_miss )
  {
//    enable_face_interrupt();        // Turn on the face strike interrupt
  }

  stop_timers();
  arm_timers();                     // Arm the counters

  sensor_status = is_running();     // and immediatly read the status
  if ( sensor_status == 0 )         // After arming, the sensor status should be zero
  { 
    if ( DLT(DLT_APPLICATION) )
    {
      printf("Waiting...");
    }  
    return WAIT;                   // Fall through to WAIT
  }

/*
 * The sensors are tripping, display the error
 */
  if ( sensor_status & 0x80  )
  {
    printf("\r\n{ \"Fault\": \"NORTH_HI\" }");
    set_status_LED(LED_NORTH_FAILED);           // Fault code North
    vTaskDelay(ONE_SECOND);
  }
  if ( sensor_status & 0x40  )
  {
    printf("\r\n{ \"Fault\": \"EAST_HI\" }");
    set_status_LED(LED_EAST_FAILED);           // Fault code East
    vTaskDelay(ONE_SECOND);
  }
  if ( sensor_status & 0x20)
  {
    printf("\n\r{ \"Fault\": \"SOUTH_HI\" }");
    set_status_LED(LED_SOUTH_FAILED);         // Fault code South
    vTaskDelay(ONE_SECOND);
  }
  if ( sensor_status & 0x10)
  {
    printf("\r\n{ \"Fault\": \"WEST_HI\" }");
    set_status_LED(LED_WEST_FAILED);         // Fault code West
    vTaskDelay(ONE_SECOND);
  }
  if ( sensor_status & 0x08  )
  {
    printf("\r\n{ \"Fault\": \"NORTH_LO\" }");
    set_status_LED(LED_NORTH_FAILED);           // Fault code North
    vTaskDelay(ONE_SECOND);
  }
  if ( sensor_status & 0x04  )
  {
    printf("\r\n{ \"Fault\": \"EAST_LO\" }");
    set_status_LED(LED_EAST_FAILED);           // Fault code East
    vTaskDelay(ONE_SECOND);
  }
  if ( sensor_status & 0x02)
  {
    printf("\n\r{ \"Fault\": \"SOUTH_LO\" }");
    set_status_LED(LED_SOUTH_FAILED);         // Fault code South
    vTaskDelay(ONE_SECOND);
  }
  if ( sensor_status & 0x01)
  {
    printf("\r\n{ \"Fault\": \"WEST_LO\" }");
    set_status_LED(LED_WEST_FAILED);         // Fault code West
    vTaskDelay(ONE_SECOND);
  }
/*
 * Got an error, try to arm again
 */
  return WAIT;
}
   
/*----------------------------------------------------------------
 * 
 * @function: wait()
 * 
 * @brief: Wait here for a shot to be fired
 * 
 * @return: Exit to the WAIT state if the circuit is ready
 *         Stay in the ARM state if a hardware fault was detected
 * 
 *----------------------------------------------------------------
 *
 * This loop is executed indefinitly until a shot is detected
 * 
 * In this loop check
 *    - State of the WiFi interface
 *
 *--------------------------------------------------------------*/
unsigned int wait(void)
{

/*
 * Check to see if the time has run out if there is a rapid fire event in progress
 */
  if ( (json_rapid_enable == 1)         // The rapid fire timer has been turned on
          && (rapid_on != 0) )          // And there is a defined interval
  {
    if (rapid_on)           // Do this until the timer expires
    {
      set_LED_PWM_now(0);

      if ( DLT(DLT_APPLICATION) )
      {
        printf("Rapid fire complete");
      }
      return REDUCE;                   // Finish this rapid fire cycle
    }
    else
    {
      set_LED_PWM_now(json_LED_PWM);   // make sure the lighs are on
    }
  }

/*
 * See if any shots have arrived
 */
  if ( this_shot != last_shot )
  {
    return REDUCE;
  }
  
/*
 * All done, keep waiting
 */
  return WAIT;
}


/*----------------------------------------------------------------
 * 
 * @function: reduce()
 * 
 * @brief: Loop through one or more shots and present the score
 * 
 * @return: Stay in the reduce state if a follow through timer is active 
 *         Jump to ARM state if more shots are ecpected
 * 
 *----------------------------------------------------------------
 *
 * This function runs in foreground and loops through the 
 * shot structure whenever there are shots in hardware to be
 * reduced.
 * 
 * In the case of rapid fire, the shot processing stops when
 * the requisite number of shots have been received.
 *
 *--------------------------------------------------------------*/
unsigned int reduce(void)
{
/*
 * See if any shots are allowed to be processed
 */
  if ( discard_shot() )                                       // Tabata is on but the shot is invalid
  {
    last_shot = this_shot;
    send_miss(&record[last_shot]);
    return START;                                              // Throw out any shots while dark
  }
  
/*
 * Loop and process the shots
 */
  while (last_shot != this_shot )
  {   
    if ( DLT(DLT_APPLICATION) )
    {
//      printf("Reducing shot: %d \n\rTrigger: ", last_shot);
      show_sensor_status(record[last_shot].sensor_status);
    }

    location = compute_hit(&record[last_shot]);                 // Compute the score
    if ( location != MISS )                                     // Was it a miss or face strike?
    {
      if ( (json_rapid_enable == 0) && (json_tabata_enable = 0))// If in a regular session, hold off for the follow through time
      {
        vTaskDelay(ONE_SECOND * json_follow_through);
      }
      send_score(&record[last_shot]);
      rapid_red(0);
      rapid_green(1);                                           // Turn off the RED and turn on the GREEN

      if ( (json_paper_time + json_step_time) != 0 )            // Has the witness paper been enabled?
      {
        if ( ((json_paper_eco == 0)                             // ECO turned off
            || ( sqrt(sq(record[this_shot].x) + sq(record[this_shot].y)) < json_paper_eco )) ) // Outside the black
        {
          drive_paper();                                        // to follow through.
        }
      } 
    }
    else
    {
      if ( DLT(DLT_APPLICATION) )
      {
        printf("Shot miss...\r\n");
      }
      set_status_LED(LED_MISS);
      send_miss(&record[last_shot]);
      rapid_green(0);
      rapid_red(1);                                             // Show a miss
    }

/* 
 *  Check to see if we should stop sending scores
 */
    if ( rapid_count != 0 )                                     // Decriment shots remaining
    {
      rapid_count--;                                            // in rapid fire
    }
    
/*
 * Increment and process the next
 */
    if ( json_rapid_enable != 0 )                               // In a rapid fire cycle
    {
      if ( rapid_count == 0 )                                   // And the shots used up?
      {
        last_shot = this_shot;                                  // Stop processing
        break;
      }
    }
    
    last_shot = (last_shot+1) % SHOT_STRING;
  }

/*
 * All done, Exit to FINISH if the timer has expired
 */
  if ( state_timer == 0 )
  {
    return START;
  } 
  else
  {
    return WAIT;
  }
}

/*----------------------------------------------------------------
 * 
 * @function: tabata_enable
 * 
 * @brief:    Start or stop a tabata session
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * {"TABATA_WARN_ON": 1, "TABATA_WARN_OFF":5, "TABATA_ON":7, "TABATA_REST":30, "TABATA_ENABLE":1}
 * {"TABATA_WARN_ON": 2, "TABATA_WARN_OFF":2, "TABATA_ON":7, "TABATA_REST":45, "TABATA_ENABLE":1}
 * {"TABATA_ENABLE":0}
 *--------------------------------------------------------------*/

#define TABATA_OFF          0         // No tabata cycles at all
#define TABATA_REST         1         // Tabata is doing nothing (typically 60 seconds)
#define TABATA_WARNING      2         // Time the warning LED is on (typically 2 seconds)
#define TABATA_DARK         3         // Time the warning LED is off before the shot (typically 2 seconds)
#define TABATA_ON           4         // Time the TABATA lED is on (typically 5 seconds)

 void tabata_enable
  (
    unsigned int enable     // Rapid fire enable state
  )
 {
/*
 * If enabled, set up the timers
 */
  if ( enable != 0 )
  {
    set_LED_PWM_now(0);                                           // Turn off the LEDs (maybe turn them on later)
  }
  else
  {
    set_LED_PWM_now(json_LED_PWM);                                // Turn on the LED to start the cycle
  }
  
  tabata_state = TABATA_OFF;                                      // Reset back to the beginning
  json_tabata_enable = enable;                                    // And enable

  if ( DLT(DLT_APPLICATION) )
  {
    if ( enable )
    {
      printf("Starting Tabata.  Time: %d", json_tabata_on);
    }
    else
    {
      printf("Tabata disabled");
    }
  }
  
/*
 * All done, return
 */
  return;
 }

/*----------------------------------------------------------------
 * 
 * @function: tabata
 * 
 * @brief:   Implement a Tabata timer for training
 * 
 * @return:  Time that the current TABATA state == TABATA_ON
 * 
 *----------------------------------------------------------------
 *
 * A Tabata timer is used to train for specific durations or time
 * of events.  For example in shooting the target should be aquired
 * and shot within a TBD seconds.
 * 
 * This function turns on the LEDs for a configured time and
 * then turns them off for another configured time and repeated
 * for a configured number of cycles
 * 
 * OFF - 2 Second Warning - 2 Second Off - ON 
 * 
 * Test JSON 
 * {"TABATA_WARN_ON": 1, "TABATA_WARN_OFF":5, "TABATA_ON":7, "TABATA_REST":30, "TABATA_ENABLE":1}
 * {"TABATA_WARN_ON": 2, "TABATA_WARN_OFF":2, "TABATA_ON":7, "TABATA_REST":45, "TABATA_ENABLE":1}
 * 
 *-------------------------------------------------------------*/

static uint16_t old_tabata_state = ~TABATA_OFF;

static long tabata
  (
  bool reset_time                   // TRUE if starting timer
  )
{
  char s[32];

  
/*
 * Reset the variables based on the arguements
 */
   if ( (json_tabata_enable == 0) || reset_time )   // Reset the timer
   {
      tabata_state = TABATA_OFF;
   }

/*
 * Execute the state machine
 */
  if ( (old_tabata_state != tabata_state ) && DLT(DLT_APPLICATION) )
  {
    printf("Tabata State: %d Duration: %d ", tabata_state, (int)(state_timer / ONE_SECOND));
  }
  
  switch (tabata_state)
  {
      case (TABATA_OFF):                  // The tabata is not enabled
        if ( json_tabata_enable != 0 )    // Just switched to enable. 
        {
          state_timer = 30 * ONE_SECOND;
          set_LED_PWM_now(0);             // Turn off the lights
          sprintf(s, "{\"TABATA_STARTING\":%d}\r\n", (30));
          serial_to_all(s, ALL);
          tabata_state = TABATA_REST;
        }
        break;
        
      case (TABATA_REST):                // OFF, wait for the time to expire
        if (state_timer == 0)             // Don't do anything unless the time expires
        {
          state_timer = json_tabata_warn_on * ONE_SECOND;
          set_LED_PWM_now(json_LED_PWM);  //     Turn on the lights
          sprintf(s, "{\"TABATA_WARN\":%d}\r\n", json_tabata_warn_on);
          serial_to_all(s, ALL);
          tabata_state = TABATA_WARNING;
        }
        break;
        
   case (TABATA_WARNING):                 // Warning time in seconds
        if ( (state_timer % 50) == 0 )
        {
          if ( ((state_timer / 50) & 1) == 0 )
          {
            set_LED_PWM_now(0);
          }
          else
          {
          set_LED_PWM_now(json_LED_PWM);
          }
        }
        if (state_timer == 0)         // Don't do anything unless the time expires
        {
          state_timer = json_tabata_warn_off * ONE_SECOND;
          set_LED_PWM_now(0);             // Turn off the lights
          sprintf(s, "{\"TABATA_DARK\":%d}\r\n", json_tabata_warn_off);
          serial_to_all(s, ALL);
          tabata_state = TABATA_DARK;
        }
        break;
      
    case (TABATA_DARK):                   // Dark time in seconds
        if (state_timer == 0 )            // Don't do anything unless the time expires
        {
          in_shot_timer = FULL_SCALE;     // Set the timer on
          state_timer = json_tabata_on * ONE_SECOND;
          set_LED_PWM_now(json_LED_PWM);           // Turn on the lights
          sprintf(s, "{\"TABATA_ON\":%d}\r\n", json_tabata_on);
          serial_to_all(s, ALL);
          tabata_state = TABATA_ON;
        }
        break;
      
    case (TABATA_ON):                     // Keep the LEDs on for the tabata time
        if ( state_timer == 0 )           // Don't do anything unless the time expires
        {
          state_timer = ((long)(json_tabata_rest - json_tabata_warn_on - json_tabata_warn_off) * ONE_SECOND);
          sprintf(s, "{\"TABATA_OFF\":%d}\r\n", (json_tabata_rest - json_tabata_warn_on - json_tabata_warn_off));
          serial_to_all(s, ALL);
          set_LED_PWM_now(0);             // Turn off the LEDs
          tabata_state = TABATA_REST;
        }
        break;
  }

 /* 
  * All done.  Return the current time if in the TABATA_ON state
  */
    old_tabata_state = tabata_state;
    return 0;
 }


/*----------------------------------------------------------------
 * 
 * @function: discard_shot
 * 
 * @brief:    Determine if the shot is outside of the valid time
 * 
 * @return:   TRUE if the shot is not allowed
 * 
 *----------------------------------------------------------------
 *  
 *--------------------------------------------------------------*/

static bool discard_shot(void)
{
  if ( (json_rapid_enable != 0)                 // Rapid Fire
      &&  (rapid_count == 0) )                  // No shots remaining
  {
    last_shot = this_shot;
    return true;                                // Discard any new shots    
  }

  if ( (json_tabata_enable != 0)                // Tabata cycle
    && ( tabata_state != TABATA_ON ) )          // Lights not on
  {
    last_shot = this_shot;                      // Discard new shots
    return true;
  }
  
  return false;
}

/*----------------------------------------------------------------
 * 
 * @function: rapid_auto
 * 
 * @brief:    Start or stop a rapid fire session
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * Turn the automatic rapid fire on or off
 * 
 * Test Message
 * {"RAPID_AUTO":1}
 * 
 *--------------------------------------------------------------*/
 void rapid_auto
  (
    unsigned int enable               // Automatic enable state
  )
 {
  rapid_enable(false);                // Rapid Enable is always turned off if Auto is ON
  
  if ( DLT(DLT_APPLICATION) )
  {
    if ( enable )
    {
      printf("Rapid Fire armed"); 
    }
    else
    {
      printf("Rapid Fire dis-armed"); 
    }
  }
  
/*
 * All done, return
 */
  return;
 }


/*----------------------------------------------------------------
 * 
 * @function: rapid_enable
 * 
 * @brief:    Start or stop a rapid fire session
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * The end of the rapid fire session (rapid_on) is computed
 * by adding the user entered time {"RAPID_ON":xx) to the current
 * time to get the end time in milliseconds
 * 
 * Test Message
 * {"RAPID_TIME":20, "RAPID_COUNT":10, "RAPID_WAIT": 5, "RAPID_ENABLE":1}
 * {"RAPID_TIME":10, "RAPID_COUNT":10, "RAPID_AUTO":1, "RAPID_ENABLE":1}
 * 
 *--------------------------------------------------------------*/
 #define RANDOM_INTERVAL 100    // 100 signals random time, %10 is the duration

void rapid_enable
(
    unsigned int enable     // Rapid fire enable state
)
{
  char str[32];
  unsigned int random_wait;

/*
 * If enabled, set up the timers
 */
  if ( enable != 0 )
  {
    set_LED_PWM_now(0);                                           // Turn off the LEDs (maybe turn them on later)
    rapid_red(1);                                                 // Turn on the RED light
    rapid_green(0);                                               // Turn off the green light
    
    if ( json_rapid_wait != 0 )
    {      
      if  ( json_rapid_wait >= RANDOM_INTERVAL )                  // > Random Interval
      {
        random_wait = esp_random() % (json_rapid_wait % 100);     // Use bottom two digits for the time
        sprintf(str, "\r\n{\"RAPID_WAIT\":%d}", random_wait);
        serial_to_all(str, ALL);
        vTaskDelay(random_wait * ONE_SECOND);
      }
      else
      {
        sprintf(str, "\r\n{\"RAPID_WAIT\":%d}", json_rapid_wait); // Use this time 
        serial_to_all(str, ALL);
        vTaskDelay(json_rapid_wait * ONE_SECOND);
      }
    }
    state_timer = (long)json_rapid_time * 1000L;                  // Duration of the event in ms
    in_shot_timer = FULL_SCALE;                                   // Set the shot timer
    rapid_count = json_rapid_count;                               // Number of expected shots
    shot_number = 1;
  }
  else
  {
    rapid_on = 0;
    rapid_red(0);                                                 // Turn off the RED light
  }
   
  set_LED_PWM_now(json_LED_PWM);                                  // Turn on the LED to start the cycle
  
  json_rapid_enable = enable;

  if ( DLT(DLT_APPLICATION) )
  {
    if ( enable )
    {
      printf("Starting Rapid Fire.  Time: %d", json_rapid_time);
    }
    else
    {
      printf("Rapid Fire disabled");
    }
  }
  
/*
 * All done, return
 */
  return;
 }

 /*----------------------------------------------------------------
 * 
 * @function: bye
 * 
 * @brief:    Go into power saver
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * This function allows the user to remotly shut down the unit
 * when not in use.
 * 
 * This is called every second from the synchronous scheduler
 * 
 *--------------------------------------------------------------*/
void bye(void)
{
/*
 * The BYE function does not work if we are a token ring.
 */
  if ( json_token != TOKEN_NONE )
  {
    return;
  }

  if ( json_power_save != 0 ) 
  {
    if ( power_save != 0 )                         // Time in minutes
    {
      power_save--;
    }
    else
    {
      power_save = (unsigned long)json_power_save * (unsigned long)ONE_SECOND * 60L;

/*
 * Say Good Night Gracie!
 */
      serial_to_all("{\"GOOD_BYE\":0}", ALL);
      tabata_enable(false);             // Turn off any automatic cycles 
      rapid_enable(false);
      set_LED_PWM(0);                   // Going to sleep 
/*
 * Loop waiting for something to happen
 */ 
    serial_flush(ALL);                // Purge the com port
  
      while( (DIP_SW_A == 0)            // Wait for the switch to be pressed
        && (DIP_SW_B == 0)            // Or the switch to be pressed
        && ( serial_available(ALL) == 0)// Or a character to arrive
        && ( is_running() == 0) )     // Or a shot arrives
      {
          vTaskDelay(1);              // Give control back to the scheduler
//    esp01_receive();                // Keep polling the WiFi to see if anything 
      }                               // turns up
  
      hello();                          // Back in action
  
      while ( (DIP_SW_A == 1)           // Wait here for both switches to be released
            || ( DIP_SW_B == 1 ) )
      {
        vTaskDelay(1);              // Give control back to the scheduler
      }  
    }
  }
/*
 * Come out of sleep
 */
  return;
}

/*----------------------------------------------------------------
 * 
 * @function: hello
 * 
 * @brief:    Come out of power saver
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * This function Puts the system back in service
 * 
 *--------------------------------------------------------------*/
void hello(void)
{
  char str[128];

  sprintf(str, "{\"Hello_World\":0}");
  serial_to_all(str, ALL);
  
/*
 * Woken up again
 */  
  set_LED_PWM_now(json_LED_PWM);
  power_save = json_power_save * (unsigned long)ONE_SECOND * 60L;   // and reset the power save time
  
  return;
}

/*----------------------------------------------------------------
 * 
 * @function: send_keep_alive
 * 
 * @brief:    Send a keep alive over the TCPIP
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * This is called every second to send out the keep alive to the 
 * TCPIP server
 * 
 *--------------------------------------------------------------*/
void send_keep_alive(void)
{
  char str[32];
  static int keep_alive_count = 0;
  static int keep_alive = 0;

  if ( (json_keep_alive != 0)
      && (keep_alive == 0) )              // Time in seconds
  {
    sprintf(str, "{\"KEEP_ALIVE\":%d}", keep_alive_count++);
    serial_to_all(str, TCPIP);
    keep_alive = json_keep_alive;
  }

  return;
}


/*----------------------------------------------------------------
 * 
 * @function: polled_target_test
 * 
 * @brief:    Abbreviated state machine to test the target aquisition
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * This arms the target and waits for a shot to be fired.  Once 
 * the shot has been received, it is displayed on the console for
 * analysis.
 * 
 * This function polls the sensors to make sure that the 
 * 
 *--------------------------------------------------------------*/
 void polled_target_test(void)
 {

  int i;
  int target_count;                     // How many cycles we have passed
  int timer_count[8];                  // Space for the 8 counters

  target_count = 1;

  printf("\r\nPolled target shot test\r\n");
  freeETimer_timer_pause();             // Kill the background timer interrupt

/*
 * Stay here watching the counters
 */
  while (1)
  {
    arm_timers();
    printf("\r\nArmed\r\n");
    while(is_running() != 0xff)
    {
      continue;
    }
    stop_timers();
    printf("\r\nStopped %d   ", target_count);
    read_timers(&timer_count[0]);
    for (i=0; i != 8; i++)
    {
      printf("%s:%5d  ", which_one[i], timer_count[i]);
    }
    target_count++;
    vTaskDelay(10);
  }

/*
 * Nothing more to do
 */
  return;
 }

 /*----------------------------------------------------------------
 * 
 * @function: interrupt_target_test
 * 
 * @brief:    Abbreviated state machine to test the target aquisition
 * 
 * @return:   Nothing
 * 
 *----------------------------------------------------------------
 *
 * This arms the target and waits for a shot to be fired.  Once 
 * the shot has been received, it is displayed on the console for
 * analysis.
 * 
 * This function polls the sensors to make sure that the 
 * 
 *--------------------------------------------------------------*/
extern int isr_state;
 void interrupt_target_test(void)
 {

  int i;

  printf("\r\nInterrupt target shot test: this: %d last %d\r\n", this_shot, last_shot);

/*
 * Stay here watching the counters
 */
  while (1)
  {
    while ( this_shot != last_shot )    // While we have a queue o shots
    {
      printf("\r\n");
      for (i=0; i != 8; i++)
      {
        printf("%s:%5d  ", which_one[i], record[last_shot].timer_count[i]);
      }
      last_shot = (last_shot+1) % SHOT_STRING;
    }
    vTaskDelay(1);
  }

/*
 * Nothing more to do
 */
  return;
 }