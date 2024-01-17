/*----------------------------------------------------------------
 * 
 * mfs.c       
 * 
 * Manage the multifunction switches
 * 
 *-------------------------------------------------------------*/
#include "driver/gpio.h"
#include "stdio.h"
#include "nvs.h"

#include "freETarget.h"
#include "diag_tools.h"
#include "gpio.h"
#include "compute_hit.h"
#include "analog_io.h"
#include "json.h"
#include "nonvol.h"
#include "serial_io.h"

/*
 *  Macros
 */
#define LONG_PRESS (ONE_SECOND)
#define TAP_A   1
#define TAP_B   2
#define HOLD_A  3
#define HOLD_B  4
#define HOLD_AB 5

#define HOLD1(x)    LO10((x))          // Low digit        xxxx2
#define HOLD2(x)    HI10((x))          // High digit       xxx2x
#define TAP1(x)     HLO10((x))         // High Low digit   xx2xx
#define TAP2(x)     HHI10((x))         // High High digit  x2xxx
#define HOLD12(x)   HHH10((x))         // Highest digit    2xxxx

/*
 *  MFS Use
 */
#define POWER_TAP     0                   // DIP A/B used to wake up
#define PAPER_FEED    1                   // DIP A/B used as a paper feed
#define LED_ADJUST    2                   // DIP A/B used to set LED brightness
#define PAPER_SHOT    3                   // DIP A/B Advance paper one cycle
#define PC_TEST       4                   // DIP A/B used to trigger fake shot
#define ON_OFF        5                   // DIP A/B used to turn the target ON or OFF
#define MFS_SPARE_6   6
#define MFS_SPARE_7   7
#define MFS_SPARE_8   8
#define TARGET_TYPE   9                   // Sent target type with score

#define NO_ACTION     0                   // DIP usual function
#define RAPID_RED     1                   // Rapid Fire Red Output
#define RAPID_GREEN   2                   // Rapid Fire Green Output

/*
 * Function Prototypes 
 */
static void send_fake_score(void);
static void sw_state(unsigned int action);      // Carry out the MFS function

/*
 * Variables 
 */
static unsigned int dip_mask;      // Output to the DIP port if selected
static int switch_A = 0;
static int switch_B = 0;           // Count how long the switch has been closed

/*-----------------------------------------------------
 * 
 * @function: multifunction_init
 * 
 * @brief:    Use the multifunction switches during starup
 * 
 * @return:   None
 * 
 *-----------------------------------------------------
 * 
 * Read the jumper header and modify the initialization
 * 
 *-----------------------------------------------------*/
 void multifunction_init(void)
 {
  unsigned int dip;

/*
 * Check to see if the DIP switch has been overwritten
 */
  if ( (HOLD1(json_multifunction2) == RAPID_RED) 
        || (HOLD1(json_multifunction2) == RAPID_GREEN))
  {
      gpio_set_level(DIP_A, 1);
      dip_mask = RED_MASK;
  }

  if (  (HOLD2(json_multifunction2) == RAPID_RED)
      || (HOLD2(json_multifunction2) == RAPID_GREEN ) )
  {
      gpio_set_level(DIP_C, 1);
      dip_mask |= GREEN_MASK;
  }

/*
 * Continue to read the DIP switch
 */
  dip = read_DIP();                     // Read the jumper header

  if ( dip == 0 )                       // No jumpers in place
  { 
    return;                             // Carry On
  }

  if ( DIP_SW_A && DIP_SW_B )           // Both switches closed?
  {
    factory_nonvol(false);              // Initalize the nonvol but do not calibrate
  }

  else
  {
    if ( DIP_SW_A )                     // Switch A pressed
    {
      is_trace = 10;                    // Turn on tracing
    }
  
    if ( DIP_SW_B )                     // Switch B pressed
    {

    }
  }
  
/*
 * The initialization override has been finished
 */
  return;
}

 
/*-----------------------------------------------------
 * 
 * @function: multifunction_switch
 * 
 * @brief:    Carry out the functions of the multifunction switch
 * 
 * @return:   Switch state
 * 
 *-----------------------------------------------------
 *
 * The actions of the DIP switch will change depending on the 
 * mode that is programmed into it.
 * 
 * For some of the DIP switches, tapping the switch
 * turns the LEDs on, and holding it will carry out 
 * the alternate activity.
 * 
 * MFS_TAP1\": \"%s\",\n\r\"MFS_TAP2\": \"%s\",\n\r\"MFS_HOLD1\": \"%s\",\n\r\"MFS_HOLD2\": \"%s\",\n\r\"MFS_HOLD12\": \"%s\",\n\r", 
 * Special Cases
 * 
 * Both switches pressed, Toggle the Tabata State
 * Either switch set for target type switch
 *-----------------------------------------------------*/
void multifunction_switch_tick(void)
{
  if ( DIP_SW_A )
  {
    switch_A++;            // Add 1 every time the switch is closed
    if ( switch_A < LONG_PRESS )
    {
      set_status_LED("-G-");
    }
    else
    {
      set_status_LED("-W-");
    }
  }

  if ( DIP_SW_B )
  {
    switch_B++;
    if ( switch_B < LONG_PRESS )
    {
      set_status_LED("--G");
    }
    else
    {
      set_status_LED("--W");
    }
  }
/*
 *  Return to the scheduler 
 */
  return;
}

void multifunction_switch(void)
{
  int action;

/*
 * Do nothing if the switches are closed
 */
  if ( (DIP_SW_A) || (DIP_SW_B) )
  {
    return;
  }
printf("here");
/*
 * Figure out what switches are pressed
 */
  action = 0;                         // No switches pressed
  if (switch_A != 0) 
  {
    action = TAP_A;
    if ( switch_A >= LONG_PRESS  )
    {
      action = HOLD_A;
    }
  }
  if (switch_B != 0) 
  {
    action = TAP_B;
    if ( switch_B >= LONG_PRESS  )
    {
      action = HOLD_B;
    }
  }
  if ( (switch_A != 0) && (switch_B != 0 ) )
  {
    action = HOLD_AB;
  }
   
/*
 * Special case of a target type, ALWAYS process this switch even if it is closed
 */
   if ( HOLD1(json_multifunction) == TARGET_TYPE ) 
   {
     sw_state(HOLD1(json_multifunction));
   }
   else if ( HOLD2(json_multifunction) == TARGET_TYPE ) 
   {
     sw_state(HOLD2(json_multifunction));
   }


/*
 *  Decode taps and holds and execute the switch selection
 */
  DLT(DLT_DIAG, printf("Switch state: %d", action);)
  switch (action)
  {
    case TAP_A:
      sw_state(TAP1(json_multifunction));
      break;

    case TAP_B:
      sw_state(TAP2(json_multifunction));
      break;

    case HOLD_A:
      sw_state(HOLD1(json_multifunction));
      break;

    case HOLD_B:
      sw_state(HOLD2(json_multifunction));
      break;

    case HOLD_AB:
      sw_state(HOLD12(json_multifunction));
      break;
  }
  
/*
 * All done, return the GPIO state
 */
  multifunction_wait_open();
  switch_A = 0;
  switch_B = 0;                 // Reset the timers
  set_status_LED("-  ");
  return;
}


/*-----------------------------------------------------
 * 
 * @function: multifunction_switch helper functions
 * 
 * @brief:    Small functioins to work with the MFC
 * 
 * @return:   Switch state
 * 
 *-----------------------------------------------------
 *
 * The MFC software above has been organized to use helper
 * @functions to simplify the construction and provide
 * consistency in the operation.
 * 
 *-----------------------------------------------------*/

/*
 * Carry out an action based on the switch state
 */
static void sw_state 
    (
    unsigned int action
    )
{     
  unsigned int led_step;

  char s[128];                          // Holding string 
  
  DLT(DLT_DIAG, printf("Switch action: %d", action);)

  switch (action)
  {
    case POWER_TAP:
      set_LED_PWM_now(json_LED_PWM);      // Yes, a quick press to turn the LED on
      vTaskDelay(ONE_SECOND/2),
      set_LED_PWM_now(0);                 // Blink
      vTaskDelay(ONE_SECOND/2);
      set_LED_PWM_now(json_LED_PWM);      // and leave it on
      power_save = (long)json_power_save * 60L * (long)ONE_SECOND; // and resets the power save time
      json_power_save += 30;      
      sprintf(s, "\r\n{\"LED_PWM\": %d}\n\r", json_power_save);
      serial_to_all(s, ALL);  
      break;
        
    case PAPER_FEED:                      // The switch acts as paper feed control
      paper_on_off(true);                 // Turn on the paper drive
      while ( (DIP_SW_A || DIP_SW_B) )    // Keep it on while the switches are pressed 
      {
        continue; 
      }
      paper_on_off(false);                // Then turn it off
      break;

   case PAPER_SHOT:                       // The switch acts as paper feed control
      drive_paper();                      // Turn on the paper drive
      while ( (DIP_SW_A || DIP_SW_B) )    // Keep it on while the switches are pressed 
      {
        continue; 
      }
      break;
      
   case PC_TEST:                         // Send a fake score to the PC
      send_fake_score();
      break;
      
   case ON_OFF:                          // Turn the target off
      bye();                             // Stay in the Bye state until a wake up event comes along
      break;
      
    case LED_ADJUST:
      led_step = 5;
      while ( (DIP_SW_A || DIP_SW_B) )    // Keep it on while the switches are pressed 
      {
        json_LED_PWM += led_step;         // Bump up the LED by 5%
        if ( json_LED_PWM > 100 )
        {
          json_LED_PWM = 100;
          led_step = -5;                 // Force to zero on wrap around
        }
        if ( json_LED_PWM <0 )
        {
          json_LED_PWM = 0;
          led_step = +5;                 // Force to zero on wrap around
        }
        set_LED_PWM_now(json_LED_PWM);   // Set the brightness
        sprintf(s, "\r\n{\"LED_BRIGHT\": %d}\n\r", json_LED_PWM);
        serial_to_all(s, ALL);
        vTaskDelay(ONE_SECOND/4);
      }
      nvs_set_i32(my_handle, NONVOL_LED_PWM, json_LED_PWM);
      nvs_commit(my_handle);
      break;

    case TARGET_TYPE:                     // Over ride the target type if the switch is closed
      json_target_type = 0;
      if (HOLD1(json_multifunction) == TARGET_TYPE) // If the switch is set for a target type
      {
        if ( DIP_SW_A )
        {
          json_target_type = 1;           // 
        }
      }
      if (HOLD2(json_multifunction) == TARGET_TYPE) 
      {
        if ( DIP_SW_B )
        {
          json_target_type = 1;
        }
      }
      break;
      
    default:
      break;
  }

/*
 * All done, return
 */
  return;
}

/*
 * Wait here for the switches to be opened
 */
void multifunction_wait_open(void)
{
  while (1)
  {
    if ( (DIP_SW_A == 0 )
        && (DIP_SW_B == 0) ) 
    {
      return;
    }

    if ( (HOLD1(json_multifunction) == TARGET_TYPE ) 
      && (DIP_SW_B == 0) )
    {
      return;
    }
    
    if ( ( HOLD2(json_multifunction) == TARGET_TYPE) 
      && ( DIP_SW_A == 0 ) )
    {
      return;
    }
  }
  
  return;
}

/*-----------------------------------------------------
 * 
 * @function: multifunction_display
 * 
 * @brief:    Display the MFS settings as text
 * 
 * @return:   None
 * 
 *-----------------------------------------------------
 *
 * The MFS is encoded as a 3 digit packed BCD number
 * 
 * This function unpacks the numbers and displayes it as
 * text in a JSON message.
 * 
 *-----------------------------------------------------*/
 //                             0            1            2             3            4             5            6    7    8          9
static char* mfs_text[] = { "WAKE_UP", "PAPER_FEED", "ADJUST_LED", "PAPER_SHOT", "PC_TEST",  "POWER_ON_OFF",   "6", "7", "8", "TARGET_TYPE"};

 //                              0           1            2             3            4             5            6    7    8          9
static char* mfs2_text[] = { "DEFAULT", "RAPID_RED", "RAPID_GREEN",    "3",         "4",          "5",   "      6", "7", "8",       "9"};

void multifunction_display(void)
{
  char s[256];                          // Holding string

  sprintf(s, "\"MFS_TAP1\": \"%s\",\n\r\"MFS_TAP2\": \"%s\",\n\r\"MFS_HOLD1\": \"%s\",\n\r\"MFS_HOLD2\": \"%s\",\n\r\"MFS_HOLD12\": \"%s\",\n\r", 
  mfs_text[TAP1(json_multifunction)], mfs_text[TAP2(json_multifunction)], mfs_text[HOLD1(json_multifunction)], mfs_text[HOLD2(json_multifunction)], mfs_text[HOLD12(json_multifunction)]);
  serial_to_all(s, ALL);  

  sprintf(s, "\"MFS_CONFIG\": \"%s\",\n\r\"MFS_DIAG\": \"%s\",\n\r", 
  mfs2_text[HOLD1(json_multifunction2)], mfs2_text[HOLD2(json_multifunction2)]);
  serial_to_all(s, ALL);  
  
/*
 * All done, return
 */
  return;
}

/*----------------------------------------------------------------
 * 
 * @function: send_fake_score
 * 
 * @brief: Send a fake score to the PC for testing
 * 
 * @return: Nothing
 * 
 *----------------------------------------------------------------
 *
 *  This function reads the values from the counters and saves
 *  saves them into the record structure to be reduced later 
 *  on.
 *
 *--------------------------------------------------------------*/
static void send_fake_score(void) 
{ 
  static   shot_record_t shot;
    
  shot.x = 0 ; // esp_random() % (json_sensor_dia/2.0);
  shot.y = 0;
  shot.shot_number++;
  send_score(&shot);

  return;
} 