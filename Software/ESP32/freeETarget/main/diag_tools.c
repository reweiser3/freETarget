
/*******************************************************************************
 *
 * diag_tools.c
 *
 * Debug and test tools 
 *
 ******************************************************************************/

#include "freETarget.h"
#include "gpio.h"
#include "diag_tools.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "nonvol.h"
#include "analog_io.h"
#include "token.h"            // Time provided by the token ring
#include "json.h"
#include "stdio.h"
#include "esp_random.h"
#include "compute_hit.h"
#include "math.h"
#include "serial_io.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"

#include "timer.h"
#include "esp_timer.h"
#include "dac.h"

const char* which_one[4] = {"North:", "East:", "South:", "West:"};

#define TICK(x) (((x) / 0.33) * OSCILLATOR_MHZ)   // Distance in clock ticks
#define RX(Z,X,Y) (16000 - (sqrt(sq(TICK(x)-s[(Z)].x) + sq(TICK(y)-s[(Z)].y))))
#define GRID_SIDE 25                              // Should be an odd number
#define TEST_SAMPLES ((GRID_SIDE)*(GRID_SIDE))

static int   get_int(void);         // Get an integer from the user
static float get_float(void);       // Get a float from the user

/*******************************************************************************
 *
 * function: void self_test
 *
 * brief: Execute self tests based on the jumper settings
 * 
 * return: None
 *
 *******************************************************************************
 *   
 *   This function is a large case statement with each element
 *   of the case statement 
 ******************************************************************************/
unsigned int tick;
void self_test
(
  unsigned int test                 // What test to execute
)
{
  unsinged int i;

  
/*
 * Figure out what test to run
 */
  switch (test)
  {
/*
 * Test 0, Display the help
 */
    default:                // Undefined, show the tests
    case T_HELP:                
      printf("\r\n 1 - Digital inputs");
      printf("\r\n 2 - Advance paper backer");
      printf("\r\n 3 - LED brightness test");

      printf("\r\n 3 - Counter values (internal trigger)");



      printf("\r\n 5 - Face strike test");
      printf("\r\n 6 - WiFi test");
      printf("\r\n 7 - Count on the LEDs");
      printf("\r\n");
      break;

/*
 * Test 1, Display GPIO inputs
 */
    case T_DIGITAL: 
      digital_test();
      break;

/*
 * Test 2, Advance the paper
 */
    case T_PAPER:
      printf("\r\nAdvancing paper %d ms", json_paper_time);
      paper_on_off(true);
      vTaskDelay(ONE_SECOND * json_paper_time / 1000);
      paper_on_off(false);
      printf(" done\r\n");
      break;

/*
 * Test 3, Set the LED bightness
 */
    case T_LED:
      printf("\r\nCycling the LED");
      for (i=0; i <= 100; i += 5)
      {
        
      }
      paper_on_off(true);
      vTaskDelay(ONE_SECOND * json_paper_time / 1000);
      paper_on_off(false);
      printf(" done\r\n");
      break;
  }
 
 /* 
  *  All done, return;
  */
    return;
}

/*******************************************************************************
 * 
 * function: POST_version()
 * 
 * brief: Show the Version String
 * 
 * return: None
 * 
 *******************************************************************************
 *
 *  Common function to show the version. Routed to the selected
 *  port(s)
 *  
 *******************************************************************************/
 void POST_version(void)
 {
  char str[64];
  sprintf(str, "\r\nfreETarget %s\r\n", SOFTWARE_VERSION);
  serial_to_all(str, ALL);
/*
 * All done, return
 */
  return;
}
 
/*----------------------------------------------------------------
 * 
 * function: POST_LEDs()
 * 
 * brief: Show the LEDs are working
 * 
 * return: None
 * 
 *----------------------------------------------------------------
 *
 *  Cycle the LEDs to show that the board has woken up and has
 *  freETarget software in it.
 *  
 *--------------------------------------------------------------*/

 void POST_LEDs(void)
 {
  if ( DLT(DLT_CRITICAL) )
  {
    printf("POST LEDs");
  }

  set_LED("R  ");
  delay(ONE_SECOND/4);
  set_LED(" R ");
  delay(ONE_SECOND/4);
  set_LED("  R");
  delay(ONE_SECOND/4);
  set_LED("G  ");
  
  return;
 }

/*----------------------------------------------------------------
 * 
 * function: void POST_counters()
 * 
 * brief: Verify the counter circuit operation
 * 
 * return: None
 * 
 *----------------------------------------------------------------
 *
 *  Trigger the counters from inside the circuit board and 
 *  read back the results and look for an expected value.
 *  
 *  Return TRUE if the complete circuit is working
 *  
 *  Test 1, Arm the circuit and make sure there are no random trips
 *          This test will fail if the sensor cable harness is not attached
 *  Test 2, Arm the circuit amd make sure it is off (No running counters
 *  Test 3: Trigger the counter and make sure that all sensors are triggered
 *  Test 4: Stop the clock and make sure that the counts have stopped
 *  Test 5: Verify that the counts are correctia
 *  
 *--------------------------------------------------------------*/
 #define POST_counteres_cycles 10 // Repeat the test 10x
 #define CLOCK_TEST_LIMIT 500    // Clock should be within 500 ticks
 
 bool_t POST_counters(void)
 {
   unsigned int  i, j;            // Iteration counter
   unsigned long random_delay;    // Delay duration
   unsigned int  sensor_status;   // Sensor status
   int           x;               // Time difference (signed)
   bool_t        test_passed;     // Record if the test failed
   
/*
 * The test only works on V2.2 and higher
 */
  
  if ( DLT(DLT_CRITICAL) )
  {
    printf("POST_counters()");
  }

  test_passed = true;                 // And assume that it will pass
  
/*
 * Test 1, Arm the circuit and see if there are any random trips
 */
  stop_timers();                      // Get the circuit ready
  arm_timers();                       // Arm it. 
  vTaskDelay(1);                      // Wait a millisecond  
  sensor_status = is_running();       // Remember all of the running timers
  if ( (sensor_status != 0) && DLT(DLT_CRITICAL) )
  {
    printf("\r\nFailed Clock Test. Spurious trigger:"); show_sensor_status(sensor_status);
    return false;                     // No point in any more tests
  }
  
/*
 * Loop and verify the opertion of the clock circuit using random times
 */
  for (i=0; i!= POST_counteres_cycles; i++)
  {
    
/*
 *  Test 2, Arm the circuit amd make sure it is off
 */
    stop_timers();                    // Get the circuit ready
    arm_timers();
    delay(1);                         // Wait for a bit
    
    for (j=N; j <= W; j++ )           // Check all of the counters
    {
      if ( (read_counter(j) != 0) && DLT(DLT_CRITICAL) )     // Make sure they stay at zero
      {
        printf("Failed Clock Test. Counter free running: %c", nesw[j]);
        test_passed =  false;         // return a failed test
      }   
    }
    
 /*
  * Test 3: Trigger the counter and make sure that all sensors are triggered
  */
    stop_timers();                      // Get the circuit ready
    arm_timers();
    delay(1);  
    random_delay = 0; // Pick a random delay time in us
    trip_timers();
    sensor_status = is_running();       // Remember all of the running timers

    timer_new(&random_delay, esp_random() % 6000);
    while ( random_delay )
    {
      continue;
    }
    timer_delete(&random_delay);
    stop_timers();
    if ( (sensor_status != 0x0F) && DLT(DLT_CRITICAL) )      // The circuit was triggered but not all
    {                                 // FFs latched
      printf("Failed Clock Test. sensor_status:"); show_sensor_status(sensor_status);
      test_passed = false;
    }

/*
 * Test 4: Stop the clock and make sure that the counts have stopped
 */
    random_delay *= 8;                // Convert to clock ticks
    for (j=N; j <= W; j++ )           // Check all of the counters
    {
      x  = read_counter(j);
      if ( (read_counter(j) != x) && DLT(DLT_CRITICAL) )
      {
        printf("Failed Clock Test. Counter did not stop: %c", nesw[j]); show_sensor_status(sensor_status);
        test_passed = false;          // since there is delay  in
      }                               // Turning off the counters
 
/*
 * Test 5: Verify that the counts are correct
 */
      x =x - random_delay;
      if( x < 0 )
      {
        x = -x;
      }
      
      if ( (x > CLOCK_TEST_LIMIT) && DLT(DLT_CRITICAL) )     // The time should be positive and within limits
      { 
        printf("Failed Clock Test. Counter: %c  Is: %d  Should be: %d   Delta: %d", nesw[j], read_counter(j), (int)random_delay, x);
        test_passed = false;          // since there is delay  in
      }                               // Turning off the counters
    }
  }
  
/*
 * Got here, the test completed successfully
 */
  set_LED("GGG");
  return test_passed;
}
  
/*----------------------------------------------------------------
 * 
 * function: void POST_trip_point()
 * 
 * brief: Display the trip point
 * 
 * return: None
 *----------------------------------------------------------------
 *
 *  Run the set_trip_point function once
 *  
 *--------------------------------------------------------------*/
 void POST_trip_point(void)
 {
   if ( DLT(DLT_APPLICATION) )
   {
    printf("POST trip point");
   }
   
   set_trip_point(0);                // Show the trip point once (20 cycles used for blinking values)
   set_LED(LED_RESET);              // Show test test Ending
   return;
 }
 
/*----------------------------------------------------------------
 * 
 * function: set_trip_point
 * 
 * brief:  Prompt the user for a voltage
 * 
 * return: None
 *----------------------------------------------------------------
 *
 * The user is promted or a channel number and voltage.
 * 
 * These are validated and sent to the DAC driver for output
 *  
 *--------------------------------------------------------------*/
#define C_MAX   3             // V_REF set to 2.048 volts
#define V_MAX   2.047         // Maximum voltage setting

void set_trip_point(int x)
{
  int   channel;              // DAC channel
  float value;                // Voltage to write

/*
 * Prompt for the settings
 */
  printf("\r\nChannel (0-3): ");
  channel = get_int();
  if ( (channel < 0 ) || (channel > C_MAX))
  {
    printf("/r/nInvalid channel");
    return;
  }

  printf("\r\nVoltage (0-2.047): ");
  value = get_float(); 
  if ( (value < 0 ) || (value > V_MAX))
  {
    printf("/r/nInvalid Voltage");
    return;
  }

/*
 *  Output the value
 */  
  dac_write(channel, value);

/*
  * Return
  */
  return;
}

/*----------------------------------------------------------------
 *
 * function: show_sensor_status()
 *
 * brief:    Show which sensor flip flops were latched
 *
 * return:   Nothing
 * 
 *----------------------------------------------------------------
 * 
 * The sensor state NESW or .... is shown for each latch
 * The clock values are also printed
 *   
 *--------------------------------------------------------------*/

void show_sensor_status
  (
  unsigned int   sensor_status
  )
{
  unsigned int i;
  
  printf(" Latch:");

  for (i=N; i<=W; i++)
  {
    if ( sensor_status & (1<<i) )   printf("%c", nesw[i]);
    else                            printf(".");
  }
#if (0)
  if ( shot != 0 )
  {
    printf(" Timers:");

    for (i=N; i<=W; i++)
    {
      printf(" %c:%d", nesw[i], shot->timer_count[i]); 
    }
  }
#endif

  printf("  Face Strike: %d", face_strike);
  
//  printf("  V_Ref:%4.2f",TO_VOLTS(analogRead(V_REFERENCE)));
  
  printf("  Temperature: %4.2f", temperature_C());
  
  printf("  Switch:");
  
  if ( DIP_SW_A == 0 )
  {
    printf("--");
  }
  else
  {
    printf("A1");
  }
  printf(" ");
  if ( DIP_SW_B == 0 )
  {
    printf("--");
  }
  else
  {
    printf("B2");
  }

  if ( ((sensor_status & 0x0f) == 0x0f)
    && (face_strike != 0) )
  {
    printf(" PASS");
    delay(ONE_SECOND);                // Wait for click to go away
  }    

/*
 * All done, return
 */

  return;
}
/*----------------------------------------------------------------
 *
 * function: do_dlt
 *
 * brief:    Check for a DLT log and print the time
 *
 * return:   TRUE if the DLT should be printed
 * 
 *----------------------------------------------------------------
 * 
 * is_trace is compared to the log level and if valid the
 * current time stamp is printed
 * 
 * DLT_CRItiCAL levels are always printed
 *   
 *--------------------------------------------------------------*/
bool_t do_dlt
  (
  unsigned int level
  )
{ 
  if ((level & (is_trace | DLT_CRITICAL)) == 0 )
  {
    return false;      // Send out if the trace is higher than the level 
  }

  printf("\n\r%10.6fs ",esp_timer_get_time()/1000000.0 );

  return true;
}

/*----------------------------------------------------------------
 *
 * @function: get_int
 *
 * @brief:    Get an Int from the user
 *
 * @return:   Signed integer from the user
 * 
 *----------------------------------------------------------------
 * 
 * Poll the serial ports and parse the input to extract the number
 * Illegal characters are ignored
 *   
 *--------------------------------------------------------------*/
static int get_int(void)
{
  int  return_value;              // Number returned to user
  char ch;                        // Working character
  int  is_negative;               // Remember if we saw a -

  return_value = 0;
  is_negative  = 0;

/*
 * See if anything is waiting and if so, add it in
 */
  while (1)
  {
    if ( serial_available(ALL) == 0 )
    {
      continue;
    } 

    ch = serial_getch(ALL);
/*
 * Parse the stream
 */
    switch (ch)
    {        
      case '\n':                            // New line, 
      case '\r':
        if ( is_negative )
        {
          return_value = -return_value;
        }
        return return_value;                // return

      case 0x08:                            // Backspace
        return_value /= 10;                 // Discard the last entry
        break;

        
      case '-':
        is_negative = true;
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        ch -= '0';
        return_value = (return_value * 10) + ch;
        break;
    }
  }

/*
 * All done
 */
  return 0;
}  

/*----------------------------------------------------------------
 *
 * @function: get_float
 *
 * @brief:    Get a float from the user
 *
 * @return:   Signed float from the user
 * 
 *----------------------------------------------------------------
 * 
 * Poll the serial ports and parse the input to extract the number
 * Illegal characters are ignored
 *   
 *--------------------------------------------------------------*/
static float get_float(void)
{
  float  return_value;            // Number returned to user
  int  mantissa;                  // Fractional portion
  char ch;                        // Working character
  int  is_negative;               // Remember if we saw a -
  int  is_float;                  // TRUE if we have a float

  return_value = 0;
  is_negative  = 0;
  mantissa     = 1;
  is_float     = 0;

/*
 * See if anything is waiting and if so, add it in
 */
  while (1)
  {
    if ( serial_available(ALL) == 0 )
    {
      continue;
    } 

    ch = serial_getch(ALL);
/*
 * Parse the stream
 */
    switch (ch)
    {        
      case '\n':                            // New line, 
      case '\r':
        return_value /= (float)mantissa;
        if ( is_negative )
        {
          return_value = -return_value;
        }
        return return_value;                // return

      case 0x08:                            // Backspace
        return_value /= 10;                 // Discard the last entry
        if ( is_float && (mantissa != 1 ) )
        {
          mantissa /= 10;
        }
        break;

      case '-':
        is_negative = true;
        break;
  
      case '.':
        is_float = true;
        mantissa = 1;
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        ch -= '0';
        return_value = (return_value * 10.0) + (float)ch;
        if ( is_float )
        {
          mantissa *= 10;
        }
        break;
    }
  }

/*
 * All done
 */
  return 0;
}  
/*----------------------------------------------------------------
 *
 * @function: get_text
 *
 * @brief:    Get an strng from the user
 *
 * @return:   Number of characters read
 * 
 *----------------------------------------------------------------
 * 
 * Poll the serial ports and parse the input to extract the text
 *   
 *--------------------------------------------------------------*/
static int get_text
(
  char* dest,
  int   size
)
{
  int  in_count;                  // Character count returned to user
  char ch;                        // Working character

  if ( (dest == NULL) || (size == 0) )
  {
    return 0;                     // Return if the numbers are bad
  }

  in_count = 0;
  *dest = 0;                      // Null terminate

/*
 * See if anything is waiting and if so, add it in
 */
  while (1)
  {
    if ( serial_available(ALL) == 0 )
    {
      continue;
    } 

    ch = serial_getch(ALL);
/*
 * Parse the stream
 */
    switch (ch)
    {        
      case '\n':                            // New line, 
      case '\r':
        return in_count;

      case 0x08:                            // Backspace
        if ( in_count != 0 )
        {
          dest--;                           // Go back one
          *dest = 0;                        // and null terminate
          in_count--;
        }
        break;

      default:
        *dest = ch;
        dest++;
        in_count++;
        if ( in_count == size )
        {
          return in_count;
        }
        break;
    }
  }

/*
 * All done
 */
  return 0;
}  
