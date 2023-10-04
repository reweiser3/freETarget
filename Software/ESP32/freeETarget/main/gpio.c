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
#include "nvs.h"
#include "nonvol.h"
#include "serial_io.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"
#include "driver\gpio.h"
#include "driver\ledc.h"
#include "timer.h"
#include "token.h"
#include "analog_io.h"
#include "esp_timer.h"
#include "sdkconfig.h"
#include "led_strip.h"
#include "led_strip_types.h"
#include "pcnt.h"
#include "pwm.h"
#include "gpio_define.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "../managed_components/espressif__led_strip/src/led_strip_rmt_encoder.h"

/*
 * Function prototypes
 */
static void sw_state(unsigned int action);// Do something with the switches
static void send_fake_score(void);        // Send a fake score to the PC

/* 
 *  Typedefs
 */
typedef struct status_struct {
  int red;                                // Bits to send to the LED
  int green;
  int blue;
  int blink;                              // TRUE if blinking enabled
  }  status_struct_t;

/* 
 * Variables
 */
status_struct_t status[3];
static unsigned int dip_mask;             // Used if the MFS2 uses the DIP_0 or DIP_3
static long power_save;

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

unsigned int read_counter
  (
  unsigned int direction         // What direction are we reading?
  )
{
  return 0;
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
  unsigned int i;
  unsigned int x;

  x = 0;
  for (i=N; i <= W; i++)      // Look at all of the counters
  {
    if ( pcnt_read(i) != 0 )  // Timer running?
    {
      x |= (1 << i);          // Remember it
    }
  }
  
  return x;                   // Return the running mask
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
  gpio_set_level(STOP_N, 0);      // Reset the timer
  pcnt_clear(N);
  pcnt_clear(E);                  // Reset the counters
  pcnt_clear(S);
  pcnt_clear(W);
  gpio_set_level(STOP_N, 1);      // Then enable it
  
  return;
}

/*
 *  Stop the oscillator and 
 */
void stop_timers(void)
{
  gpio_set_level(STOP_N, 0);      // Reset the timer

  return;
}

/*
 *  Trip the counters for a self test
 */
void trip_timers(void)
{
  gpio_set_level(STOP_N, 1);            // Let the flipflops go
  gpio_set_level(CLOCK_START, 1);       // and trigger the output 
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
 *-----------------------------------------------------*/

unsigned int read_DIP(void)
{
  unsigned int return_value = 0;

  if (gpio_get_level(DIP_A) != 0)
  {
   return_value |= 1;
  }

  if (gpio_get_level(DIP_B) != 0)
  {
    return_value |= 2;
  }

  if (gpio_get_level(DIP_C) != 0)
  {
    return_value |= 4;
  }

  if (gpio_get_level(DIP_D) != 0)
  {
    return_value |= 8;
  }

  return return_value;
}  


/*-----------------------------------------------------
 * 
 * function: init_status_LED
 * 
 * brief:    Initialize the LED driver
 * 
 * return:   None
 * 
 *-----------------------------------------------------
 *
 * The status LED driver makes use of the remote control
 * transmitter supported in the ESP32
 * 
 *-----------------------------------------------------*/
#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)

rmt_channel_handle_t led_chan = NULL;
rmt_tx_channel_config_t tx_chan_config = {
    .clk_src           = RMT_CLK_SRC_DEFAULT, // select source clock
    .mem_block_symbols = 64, // increase the block size can make the LED less flickering
    .resolution_hz     = RMT_LED_STRIP_RESOLUTION_HZ,
    .trans_queue_depth = 1, // set the number of transactions that can be pending in the background
};

rmt_encoder_handle_t led_encoder = NULL;
led_strip_encoder_config_t encoder_config = {
        .resolution = RMT_LED_STRIP_RESOLUTION_HZ
};

void status_LED_init
(
  unsigned int led_gpio   // What GPIO is used for output
)
{
  tx_chan_config.gpio_num = led_gpio;
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));
  ESP_ERROR_CHECK(rmt_new_led_strip_encoder(&encoder_config, &led_encoder));
  ESP_ERROR_CHECK(rmt_enable(led_chan));
  return;
}

/*-----------------------------------------------------
 * 
 * function: set_status_LED
 * 
 * brief:    Set the state of all the status LEDs
 * 
 * return:   None
 * 
 *-----------------------------------------------------
 *
 * The state of the LEDs can be turned on or off 
 * 
 * 'R' - Set the LED to Red'
 * '-' - Leave the LED alone
 *  
 *-----------------------------------------------------*/

rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };

  
unsigned char led_strip_pixels[3 * 3];

void set_status_LED
  (
    char* new_state       // New LED colours
  )
{ 
  int i;

/*
 * Decode the calling string into a list of pixels
 */
  i=0;
  while (*new_state != 0)
  {
    if ( *new_state != '-' )
    {
      status[i].blink = 0;
      status[i].red = 0;
      status[i].green = 0;
      status[i].blue = 0;          // Turn off the LED
      switch (*new_state)
      {
        case 'r':               // RED LED
          status[i].blink = 1;    // Turn on Blinking
        case 'R':
          status[i].red   = 0xff;
          break;

        case 'g':               // GREEN LED
          status[i].blink = 1;    // Turn on Blinking
        case 'G':
          status[i].green = 0xff;
          break;

        case 'b':
          status[i].blink = 1;
        case 'B':
          status[i].blue  = 0xff;
          break;

        case 'w':
          status[i].blink = 1;
        case 'W':
          status[i].red   = 0xff;
          status[i].green = 0xff;
          status[i].blue  = 0xff;
          break;

        case ' ':             // The LEDs are already off
          break;
      }
    }
    i++;
    new_state++;
  }

/*
 *  Send out the new settings
 */
  for (i=0; i < 3; i++)
  {
    led_strip_pixels[i * 3 + 0] = status[i].green;
    led_strip_pixels[i * 3 + 1] = status[i].blue;
    led_strip_pixels[i * 3 + 2] = status[i].red;
  }
    
  ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));

/*
 * All done, return
 */
  return;

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
    *(timer_ptr + i) = pcnt_read(i);
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
    unsigned long wdt;
    
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
  timer_new(&wdt, ONE_SECOND);
  while ( wdt != 0 )
  {
    if ( DIP_SW_A )
    {
      set_status_LED("--G");
    }
    else
    {
      set_status_LED("-- ");
    }
    if ( DIP_SW_B )
    {
      set_status_LED("-G-");
    }
    else
    {
      set_status_LED("- -");
    }
  }
  timer_delete(&wdt);
  
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

  set_status_LED(LED_READY);
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
      bye(0);                            // Stay in the Bye state until a wake up event comes along
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
  double       volts;         // Reference Voltage
  volts = 0.0;
/*
 * Read in the fixed digital inputs
 */
  printf("\r\nTime: %4.2fs", (float)(esp_timer_get_time()/1000000));
  printf("\r\nBD Rev: %d", revision());  
  printf("\r\nDIP: 0x%02X", read_DIP()); 
  gpio_set_level(STOP_N, 0);
  gpio_set_level(STOP_N, 1);                        // Reset the fun flip flop
  printf("\r\nRUN FlipFlop: 0x%02X", is_running());   
  printf("\r\nTemperature: %4.2fdC", temperature_C());
  printf("\r\nSpeed of Sound: %4.2fmm/us", speed_of_sound(temperature_C(), json_rh));
  printf("\r\nV_REF: %4.f Volts", volts);
  printf("\r\n");

 /*
  * Blink the LEDs and exit
  */
//   POST_LEDs();
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
  record[this_shot].shot_time = 0;//FULL_SCALE - in_shot_timer; // Capture the time into the shot
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
    
  shot.x = 0 ; // esp_random() % (json_sensor_dia/2.0);
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
      gpio_set_level(DIP_C, state);
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
      gpio_set_level(DIP_B, state);
  }
  if ( HOLD2(json_multifunction2) == RAPID_GREEN )
  {
      gpio_set_level(DIP_A, state);
  }

  return;
}
