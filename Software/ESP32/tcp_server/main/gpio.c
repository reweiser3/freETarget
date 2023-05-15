/*-------------------------------------------------------
 * 
 * gpio.c
 * 
 * General purpose GPIO driver
 * 
 * ----------------------------------------------------*/
#include "freETarget.h"
#include "gpio.h"
#include "timer.h"
#include "diag_tools.h"
#include "stdio.h"
#include "json.h"
#include "token.h"
#include "compute_hit.h"
#include "nonvol.h"
#include "serial_io.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "driver/gpio.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"

static void sw_state(unsigned int action);// Do something with the switches
static void send_fake_score(void);        // Send a fake score to the PC

static unsigned int dip_mask;             // Used if the MFS2 uses the DIP_0 or DIP_3
static char aux_spool[128];               // Spooling buffer from the AUX port
static char json_spool[64];               // Spool for JSON
static unsigned int  aux_spool_in, aux_spool_out; // Pointer to the spool
static unsigned int  json_spool_in, json_spool_out; // Pointer to the spool
static long power_save;

/*-----------------------------------------------------
 * 
 * function: read_port
 * 
 * brief: Read 8 bits from a port
 * 
 * return: Eight bits returned from the port
 * 
 *-----------------------------------------------------
 *
 * To make the byte I/O platform independent, this
 * function reads the bits in one-at-a-time and collates
 * them into a single byte for return
 * 
 *-----------------------------------------------------*/
int port_list[] = { D0, D1, D2, D3, D4, D5, D6, D7};

unsigned int read_port(void)
{
  int i;
  int return_value = 0;

/*
 *  Loop and read in all of the bits
 */
  for (i=0; i != 8; i++)
    {
    return_value <<= 1;
    return_value |= gpio_get_level(port_list[i]) & 1;
    }

 /*
  * Return the result 
  */
  return (return_value & 0x00ff);
}

/*-----------------------------------------------------
 * 
 * function: read_counter
 * 
 * brief: Read specified counter register
 * 
 * return: 16 bit counter register
 * 
 *-----------------------------------------------------
 *
 * Set the address bits and read in 16 bits
 * 
 *-----------------------------------------------------*/

int direction_register[] = {NORTH_HI, NORTH_LO, EAST_HI, EAST_LO, SOUTH_HI, SOUTH_LO, WEST_HI, WEST_LO};

unsigned int read_counter
  (
  unsigned int direction         // What direction are we reading?
  )
{
  int i;
  unsigned int return_value_LO, return_value_HI;     // 16 bit port value
  
/*
 *  Reset all of the address bits
 */
  for (i=0; i != 8; i++)
  {
    gpio_set_level(direction_register[i], 1);
  }
  gpio_set_level(RCLK,  1);   // Prepare to read
  
/*
 *  Set the direction line to low
 */
  gpio_set_level(direction_register[direction * 2 + 0], 0);
  return_value_HI = read_port();
  gpio_set_level(direction_register[direction * 2 + 0], 1);
  
  gpio_set_level(direction_register[direction * 2 + 1], 0);
  return_value_LO = read_port();
  gpio_set_level(direction_register[direction * 2 + 1], 1);

/*
 *  All done, return
 */
  return (return_value_HI << 8) + return_value_LO;
}

/*-----------------------------------------------------
 * 
 * function: is_running
 * 
 * brief: Determine if the clocks are running
 * 
 * return: TRUE if any of the counters are running
 * 
 *-----------------------------------------------------
 *
 * Read in the running registers, and return a 1 for every
 * register that is running.
 * 
 *-----------------------------------------------------*/

unsigned int is_running (void)
{
  return 0;
}

/*-----------------------------------------------------
 * 
 * function: arm_timers
 * 
 * brief: Strobe the control lines to start a new cycle
 * 
 * return: NONE
 * 
 *-----------------------------------------------------
 *
 * The counters are armed by
 * 
 *   Stopping the current cycle
 *   Taking the counters out of read
 *   Making sure the oscillator is running
 *   Clearing the counters
 *   Enabling the counters to run again
 * 
 *-----------------------------------------------------*/
void arm_timers(void)
{
  gpio_set_level(CLOCK_START, 0);   // Make sure Clock start is OFF
  gpio_set_level(STOP_N, 0);        // Reset the flip flop to stop the timers
  gpio_set_level(RCLK,   0);        // Set READ CLOCK to LOW
  gpio_set_level(QUIET,  1);        // Arm the counter
  gpio_set_level(CLR_N,  0);        // Reset the counters 
  gpio_set_level(CLR_N,  1);        // Remove the counter reset 
  gpio_set_level(STOP_N, 1);        // Let the counters run
  
  return;
}

void clear_running(void)          // Reset the RUN flip Flop
{
  gpio_set_level(STOP_N, 0);        // Reset RUN outputs on the Flip Flop
  gpio_set_level(STOP_N, 1);        // Set the RUN outputs to active

  return;
}

/*
 *  Stop the oscillator
 */
void stop_timers(void)
{
  gpio_set_level(STOP_N,0);   // Stop the counters
  gpio_set_level(QUIET, 0);   // Kill the oscillator 
  return;
}

/*
 *  Trip the counters for a self test
 */
void trip_timers(void)
{
  gpio_set_level(CLOCK_START, 0);
  gpio_set_level(CLOCK_START, 1);     // Trigger the clocks from the D input of the FF
  gpio_set_level(CLOCK_START, 0);

  return;
}

/*-----------------------------------------------------
 * 
 * function: read_DIP
 * 
 * brief: READ the jumper block setting
 * 
 * return: TRUE for every position with a jumper installed
 * 
 *-----------------------------------------------------
 *
 * The DIP register is read and formed into a word.
 * The word is complimented to return a 1 for every
 * jumper that is installed.
 * 
 * OR in the json_dip_switch to allow remote testing
 * OR in  0xF0 to allow for compile time testing
 *-----------------------------------------------------*/
unsigned int read_DIP
(
  unsigned int dip_mask
)
{
  unsigned int return_value = 0;

  return_value |= 0xF0;             // COMPILE TIME

  return return_value;
}  

/*-----------------------------------------------------
 * 
 * function: set_LED
 * 
 * brief:    Set the state of all the LEDs
 * 
 * return:   None
 * 
 *-----------------------------------------------------
 *
 * The state of the LEDs can be turned on or off 
 * 
 * -1 '-' Leave alone
 *  0 '.' Turn LED off
 *  1 '*' Turn LED on
 *  
 *  The macro L(RDY, X, Y) defines 
 * 
 *-----------------------------------------------------*/
void set_LED
  (
    int state_RDY,        // State of the Rdy LED
    int state_X,          // State of the X LED
    int state_Y           // State of the Y LED
    )
{ 
  switch (state_RDY)
  {
    case 0:
    case '.':
        gpio_set_level(LED_RDY, 1 );
        break;
    
    case 1:
    case '*':
        gpio_set_level(LED_RDY, 0 );
        break;
  }
  
  switch (state_X)
  {
    case 0:
    case '.':
        gpio_set_level(LED_X, 1 );
        break;
    
    case 1:
    case '*':
        gpio_set_level(LED_X, 0 );
        break;
  }

  switch (state_Y)
  {
    case 0:
    case '.':
        gpio_set_level(LED_Y, 1 );
        break;
    
    case 1:
    case '*':
        gpio_set_level(LED_Y, 0 );
        break;
  }
  return;  
  }

/* 
 *  HAL Discrete IN
 */
bool_t read_in(unsigned int port)
{
  return gpio_get_level(port);
}

/*-----------------------------------------------------
 * 
 * function: read_timers
 * 
 * brief:   Read the timer registers
 * 
 * return:  All four timer registers read and stored
 * 
 *-----------------------------------------------------
 *
 * Force read each of the timers
 * 
 *-----------------------------------------------------*/
void read_timers
  (
    unsigned int* timer_ptr
  )
{
  unsigned int i;

  for (i=N; i<=W; i++)
  {
    *(timer_ptr + i) = read_counter(i);
  }

  return;
}

/*-----------------------------------------------------
 * 
 * function: drive_paper
 * 
 * brief:    Turn on the witness paper motor for json_paper_time
 * 
 * return:  None
 * 
 *-----------------------------------------------------
 *
 * The function turns on the motor for the specified
 * time.  The motor is cycled json_paper_step times
 * to drive a stepper motor using the same circuit.
 * 
 * Use an A4988 to drive te stepper in place of a DC
 * motor
 * 
 * There is a hardare change between Version 2.2 which
 * used a transistor and 3.0 that uses a FET.
 * The driving circuit is reversed in the two boards.
 * 
 * DC Motor
 * Step Count = 0
 * Step Time = 0
 * Paper Time = Motor ON time
 *
 * Stepper
 * Paper Time = 0
 * Step Count = X
 * Step Time = Step On time
 * 
 *-----------------------------------------------------*/

void drive_paper(void)
{
  unsigned int s_time, s_count;              // Step time and count
  volatile unsigned long paper_time;

/*
 * Set up the count or times based on whether a DC or stepper motor is used
 */

  s_time = json_paper_time;                       // On time.
  if ( json_step_time != 0 )                      // Non-zero means it's a stepper motor
  {
    s_time = json_step_time;                      // the one we use
  }

  s_count = 1;                                    // Default to one cycle (DC or Stepper Motor)
  if ( json_step_count != 0 )                     // Non-zero means it's a stepper motor
  {
    s_count = json_step_count;                    // the one we use
  }

  if ( s_time == 0 )                              // Nothing to do if the time is zero.
  {
    return;
  }
  
  if ( DLT(DLT_CRITICAL) )
  {
    printf("Advancing paper: %dms", s_time);
  }

/*
 * Drive the motor on and off for the number of cycles
 * at duration
 */
  timer_new(&paper_time, 0);              // Create the timer

  while ( s_count )
  {
    paper_on_off(true);                   // Motor ON
    paper_time = s_time; 
    while (paper_time )
    {
      continue;
    }
    paper_on_off(false);                  // Motor OFF
    paper_time = 5;
    while ( paper_time )
    {
      continue;
    }
    s_count--;                            // Repeat for the steps
  }

  timer_delete(&paper_time);              // Finished with the timer
  
 /*
  * All done, return
  */
  return;
 }

/*-----------------------------------------------------
 * 
 * function: paper_on_off
 * 
 * brief:    Turn the withness paper motor on or off
 * 
 * return:  None
 * 
 *-----------------------------------------------------
 *
 * The witness paper motor changed polarity between 2.2
 * and Version 3.0.
 * 
 * This function reads the board revision and controls 
 * the FET accordingly
 * 
 *-----------------------------------------------------*/

void paper_on_off                               // Function to turn the motor on and off
(
  bool_t on                                     // on == true, turn on motor drive
)
{
  if ( on == true )
  {
    gpio_set_level(PAPER, PAPER_ON);            // Turn it on
  }
  else
  {
    gpio_set_level(PAPER, PAPER_OFF);            // Turn it off
  }

/*
 * No more, return
 */
  return;
}


/*-----------------------------------------------------
 * 
 * function: face_ISR
 * 
 * brief:    Face Strike Interrupt Service Routint
 * 
 * return:   None
 * 
 *-----------------------------------------------------
 *
 * Sensor #5 is attached to the digital input #19 and
 * is used to generate an interrrupt whenever a face
 * strike has been detected.
 * 
 * The ISR simply counts the number of cycles.  Anything
 * above 0 is an indication that sound was picked up
 * on the front face.
 * 
 *-----------------------------------------------------*/
 void face_ISR(void)
 {
  face_strike++;      // Got a face strike

  if ( DLT(DLT_CRITICAL) )
  {
    printf("\r\nface_ISR(): %d", face_strike);
  }

  return;
 }

 /*
 * Common function to indicate a fault // Cycle LEDs 5x
 */
void blink_fault
  (                                        
  unsigned int fault_code                 // Fault code to blink
  )
{
  unsigned int i;

  for (i=0; i != 3; i++)
  {
    set_LED(fault_code & 4, fault_code & 2, fault_code & 1);  // Blink the LEDs to show an error
    delay(ONE_SECOND/4);
    fault_code = ~fault_code;
    set_LED(fault_code & 4, fault_code & 2, fault_code & 1);                    // Blink the LEDs to show an error
    delay(ONE_SECOND/4);
    fault_code = ~fault_code;
  }

 /*
  * Finished
  */
  return;
}

/*-----------------------------------------------------
 * 
 * function: multifunction_init
 * 
 * brief:    Use the multifunction switches during starup
 * 
 * return:   None
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
      gpio_set_level(DIP_0, 1);
      dip_mask = RED_MASK;
  }

  if (  (HOLD2(json_multifunction2) == RAPID_RED)
      || (HOLD2(json_multifunction2) == RAPID_GREEN ) )
  {
      gpio_set_level(DIP_3, 1);
      dip_mask |= GREEN_MASK;
  }

/*
 * Continue to read the DIP switch
 */
  dip = read_DIP(dip_mask) & 0x0f;      // Read the jumper header

  if ( dip == 0 )                       // No jumpers in place
  { 
    return;                             // Carry On
  }

  if ( CALIBRATE )                      // Calibration jumper in?
  {
    set_trip_point(0);
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
 * function: multifunction_switch
 * 
 * brief:    Carry out the functions of the multifunction switch
 * 
 * return:   Switch state
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
                           
void multifunction_switch(void)
 {
    unsigned int  action;               // Action to happen
    unsigned long now;
    
    if ( CALIBRATE )
    {
      return;                           // Not used if in calibration mode
    }

/*
 * Figure out what switches are pressed
 */
   action = 0;                         // No switches pressed
   if ( DIP_SW_A != 0 )
   {
     action += 1;                     // Remember how we got here
   }
   if ( DIP_SW_B != 0 )
   {
     action += 2;
   }

/*
 * Special case of a target type, ALWAYS process this switch even if it is closed
 */
   if ( HOLD1(json_multifunction) == TARGET_TYPE ) 
   {
     sw_state(HOLD1(json_multifunction));
     action &= ~1;
     action += 4;
   }
   else if ( HOLD2(json_multifunction) == TARGET_TYPE ) 
   {
     sw_state(HOLD2(json_multifunction));
     action &= ~2;
     action += 8;
   }

   if ( action == 0 )                 // Nothing to do
   {
     return;
   }
   
/*
 * Delay for one second to detect a tap
 * Check to see if the switch has been pressed for the first time
 */
  now = millis();
  while ( (millis() - now) <= ONE_SECOND )
  {
    if ( DIP_SW_A )
    {
      set_LED(L('-', '*', '-'));
    }
    else
    {
      set_LED(L('-', '.', '-'));
    }
    if ( DIP_SW_B )
    {
      set_LED(L('-', '-', '*'));
    }
    else
    {
      set_LED(L('-', '-', '.'));
    }
  }
  
  if ( (DIP_SW_A == 0 )
        && (DIP_SW_B == 0 ) )             // Both switches are open? (tap)
   {
      if ( action & 1 )
      {
        sw_state(TAP1(json_multifunction));
      }
      if ( action & 2 )
      {
        sw_state(TAP2(json_multifunction));
      }
   }
   
/*
 * Look for the special case of both switches pressed
 */
  if ( (DIP_SW_A) && (DIP_SW_B) )         // Both pressed?
  {
    sw_state(HOLD12(json_multifunction));
  }
      
/*
 * Single button pressed manage the target based on the configuration
 */
  else
  {
    if ( DIP_SW_A )
    {
      sw_state(HOLD1(json_multifunction));
    }
    if ( DIP_SW_B )
    {
      sw_state(HOLD2(json_multifunction));
    }
  }
  
/*
 * All done, return the GPIO state
 */
  multifunction_wait_open();      // Wait here for the switches to be open

  set_LED(LED_READY);
  return;
}


/*-----------------------------------------------------
 * 
 * function: multifunction_switch helper functions
 * 
 * brief:    Small functioins to work with the MFC
 * 
 * return:   Switch state
 * 
 *-----------------------------------------------------
 *
 * The MFC software above has been organized to use helper
 * functions to simplify the construction and provide
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
  
  char s[128];                          // Holding string 
  
  if ( DLT(DLT_CRITICAL) )
  {
    printf("Switch action: %d", action);
  }

  switch (action)
  {
    case POWER_TAP:
      set_LED_PWM_now(json_LED_PWM);      // Yes, a quick press to turn the LED on
      delay(ONE_SECOND/2),
      set_LED_PWM_now(0);                 // Blink
      delay(ONE_SECOND/2);
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
      bye();                              // Stay in the Bye state until a wake up event comes along
      break;
      
    case LED_ADJUST:
      json_LED_PWM += 10;                 // Bump up the LED by 10%
      if ( json_LED_PWM > 100 )
      {
        json_LED_PWM = 0;                 // Force to zero on wrap around
      }
      set_LED_PWM_now(json_LED_PWM);      // Set the brightness
      nvs_set_i32(my_handle, NONVOL_LED_PWM, json_LED_PWM);   
      sprintf(s, "\r\n{\"LED_BRIGHT\": %d}\n\r", json_LED_PWM);
      serial_to_all(s, ALL);  
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
 * function: multifunction_display
 * 
 * brief:    Display the MFS settings as text
 * 
 * return:   None
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
static char* mfs_text[] = { "WAKE_UP", "PAPER_FEED", "ADJUST_LED", "PAPER_SHOT", "PC_TEST",  "POWER_ON_OFF",   "6", "7", "8", "TARget_TYPE"};

 //                              0           1            2             3            4             5            6    7    8          9
static char* mfs2_text[] = { "DEFAULT", "RAPID_RED", "RAPID_GREEN",    "3",         "4",          "5",   "      6", "7", "8",       "9"};

void multifunction_display(void)
{
  char s[128];                          // Holding string

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

/*-----------------------------------------------------
 * 
 * function: digital_test()
 * 
 * brief:    Exercise the GPIO digital ports
 * 
 * return:   None
 * 
 *-----------------------------------------------------
 *
 * Read in all of the digial ports and report the 
 * results
 * 
 *-----------------------------------------------------*/
void digital_test(void)
{
  int i;
  double       volts;         // Reference Voltage
  
/*
 * Read in the fixed digital inputs
 */
  printf("\r\nTime: %4.2fs", (float)(micros()/1000000));
  printf("\r\nBD Rev: %d", revision());       
  printf("\r\nDIP: 0x%02X", read_DIP(0)); 
  gpio_set_level(STOP_N, 0);
  gpio_set_level(STOP_N, 1);                        // Reset the fun flip flop
  printf("\r\nRUN FlipFlop: 0x%02X", is_running());   
  printf("\r\nTemperature: %dC", temperature_C());
  printf("\r\nSpeed of Sound: %4.2fmm/us", speed_of_sound(temperature_C(), json_rh));
  printf("\r\nV_REF: %4.f Volts", volts);
  printf("\r\n");

/*
 * Read the port pins and report
 */
  i=0;

 /*
  * Blink the LEDs and exit
  */
   POST_LEDs();
   return;
}


/*----------------------------------------------------------------
 * 
 * function: aquire()
 * 
 * brief: Aquire the data from the counter registers
 * 
 * return: Nothing
 * 
 *----------------------------------------------------------------
 *
 *  This function reads the values from the counters and saves
 *  saves them into the record structure to be reduced later 
 *  on.
 *
 *--------------------------------------------------------------*/
void aquire(void)
 {
/*
 * Pull in the data amd save it in the record array
 */
  if ( DLT(DLT_CRITICAL) )
  {
    printf("Aquiring shot: %d", this_shot);
  }
  stop_timers();                                    // Stop the counters
  read_timers(&record[this_shot].timer_count[0]);   // Record this count
  record[this_shot].shot_time = FULL_SCALE - in_shot_timer; // Capture the time into the shot
  record[this_shot].face_strike = face_strike;      // Record if it's a face strike
  record[this_shot].sensor_status = is_running();   // Record the sensor status
  record[this_shot].shot_number = shot_number++;    // Record the shot number and increment
  this_shot = (this_shot+1) % SHOT_STRING;          // Prepare for the next shot

/*
 * MAll done for now
 */
  return;
}

/*----------------------------------------------------------------
 * 
 * function: send_fake_score
 * 
 * brief: Send a fake score to the PC for testing
 * 
 * return: Nothing
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
    
  shot.x = random(-json_sensor_dia/2.0, json_sensor_dia/2.0);
  shot.y = 0;
  shot.shot_number++;
  send_score(&shot);

  return;
} 

/*----------------------------------------------------------------
 * 
 * function: rapid_red()
 *           rapid_green()
 * 
 * brief: Set the RED and GREEN lights
 * 
 * return: Nothing
 * 
 *----------------------------------------------------------------
 *
 *  If MFS2 has enabled the rapid fire lights then allow the 
 *  value to be set
 *
 *--------------------------------------------------------------*/

void rapid_red
(
  unsigned int state          // New state for the RED light
) 
{
  if ( HOLD1(json_multifunction2) == RAPID_RED )
  {
      gpio_set_level(DIP_0, state);
  }
  if ( HOLD2(json_multifunction2) == RAPID_RED )
  {
      gpio_set_level(DIP_3, state);
  }

  return;
}

void rapid_green
(
  unsigned int state          // New state for the RED light
) 
{
  if ( HOLD1(json_multifunction2) == RAPID_GREEN )
  {
      gpio_set_level(DIP_0, state);
  }
  if ( HOLD2(json_multifunction2) == RAPID_GREEN )
  {
      gpio_set_level(DIP_3, state);
  }

  return;
}
