
/*******************************************************************************
 *
 * diag_tools.c
 *
 * Debug and test tools 
 * 
 * See
 * https://www.espressif.com/sites/default/files/documentation/esp32-s3_technical_reference_manual_en.pdf
 *
 ******************************************************************************/

#include "stdbool.h"
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
#include "pwm.h"
#include "analog_io.h"
#include "gpio_define.h"
#include "driver\gpio.h"
#include "pcnt.h"

const char* which_one[4] = {"North", "East", "South", "West"};

#define TICK(x) (((x) / 0.33) * OSCILLATOR_MHZ)   // Distance in clock ticks
#define RX(Z,X,Y) (16000 - (sqrt(sq(TICK(x)-s[(Z)].x) + sq(TICK(y)-s[(Z)].y))))
#define GRID_SIDE 25                              // Should be an odd number
#define TEST_SAMPLES ((GRID_SIDE)*(GRID_SIDE))

static int   get_int(void);         // Get an integer from the user
static float get_float(void);       // Get a float from the user

/*******************************************************************************
 *
 * @function: void self_test
 *
 * @brief: Execute self tests based on the jumper settings
 * 
 * @return: None
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
  int   i;
  float x, volts;

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
      printf("\r\n 4 - Status LED driver");
      printf("\r\n 5 - Temperature and sendor test");
      printf("\r\n 6 - DAC test");

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
      for (i=0; i != 10; i++)
      {
        paper_on_off(true);
        timer_new(&i, 500); 
        while(i != 0)
        {
          continue;
        }
        paper_on_off(false);
        vTaskDelay(50);
      }
      printf(" done\r\n");
      break;

/*
 * Test 3, Set the LED bightness
 */
    case T_LED:
      printf("\r\nCycling the LED");
      for (i=0; i <= 100; i++)
      {
        pwm_set(LED_PWM, i);       
        vTaskDelay(ONE_SECOND/10);
      }
      for (i=100; i >= 0; i--)
      {
        pwm_set(LED_PWM,i);       
        vTaskDelay(ONE_SECOND/10);
      }
      printf(" done\r\n");
      break;

/*
 * Test 4, Set status LEDs
 */
    case T_STATUS:
      printf("\r\nSending out status");
      while(1)
      {
      set_status_LED("R--");
      vTaskDelay(ONE_SECOND);
      
      set_status_LED("RG-");
      vTaskDelay(ONE_SECOND);

      set_status_LED("RGB");
      vTaskDelay(ONE_SECOND);
      
      set_status_LED("WWW");
      vTaskDelay(ONE_SECOND);
      }
      printf(" done\r\n");
      break;

/*
 * Test 5, Temperature
 */
    case T_TEMPERATURE:
    while(1)
    {
      printf("\r\nTemperature: %f", temperature_C());
      printf("\r\nHumidity: %f\r\n", humidity_RH());
    }
      break;

/*
 * Test 6, V_REF
 */
    case T_VREF:
      printf("\r\nRamping DACs 0 & 1 ");
      volts = 0.0;
      while(1)
      {
        set_VREF();
        volts += 0.0005;
        if ( volts > 2.048 )
        {
          volts = 0;
        }
      }
      printf("done");
      break;

/*
 * Test 7, Analog In
 */
    case T_AIN:
      printf("\r\nAnalog Input ");
      printf("\r\n12V %5.3f", v12_supply());
      printf("\r\nBoard Rev %d", revision());
      break;

/*
 * Test 8, Timer Control
 */
    case T_TIMER:
      printf("\r\nTimer Control ");
      i=0;
      while (1)
      {
        gpio_set_level(CLOCK_START, i&1);
        gpio_set_level(STOP_N, (~i) & 1);
        gpio_set_level(STOP_N, i & 1);
        i++;

      }

#if(0)
      volatile unsigned int* gpio_out;
      gpio_out = 0x60004004;
      volatile unsigned int* gpio_in;
      gpio_in = 0x6000403C;
      register unsigned int old,new, temp, a, b, c, d, z;
      new = (1<<21);
      old = (*gpio_in) & 15;       // Read the por

while(1)
{
      *gpio_out ^= new;
      temp = (*gpio_in) & 15;       // Read the por
      temp ^= old;          // Look for differeces
      if ( temp != 0 )
      {
        if ( temp & 1 ) a = *gpio_out;
        if ( temp & 2 ) b = *gpio_out;
        if ( temp & 4 ) c = *gpio_out;
        if ( temp & 8 ) d = *gpio_out;
        old ^= temp;
      }

//        vTaskDelay(ONE_SECOND/100);
      }
      printf("done");
      break;
#endif

/*
 * Test 9, PCNT test
 */
    case T_PCNT:
      printf("\r\nRead PCNT\r\n");
      arm_timers();
      trip_timers();
      for (i=0; i != 4; i++)
      {
        printf("%s: %d    ", which_one[i], pcnt_read(i));
      }
      printf("\r\ndone");
      break;

/*
 * Test 10, Timer Interrupt Test
 */
    case T_ISR:
      printf("\r\nStarting Timers\r\n");
      freeETarget_timer_init();
      printf("\r\ndone");
      break;

/*
 *  Test 11: Sensor Trigger
 */
    case T_SENSOR:
      POST_counters();
      break;

/*
 *  Test 12: Delete NONVOL
 */
    case T_NONVOL:
      POST_counters();
      break;
  }
 /* 
  *  All done, return;
  */
    return;
}

/*******************************************************************************
 * 
 * @function: POST_version()
 * 
 * @brief: Show the Version String
 * 
 * @return: None
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
 * @function: void POST_counters()
 * 
 * @brief: Verify the counter circuit operation
 * 
 * @return: None
 * 
 *----------------------------------------------------------------
 *
 *  Trigger the counters from inside the circuit board and 
 *  read back the results and look for an expected value.
 *  
 *  Return TRUE if the complete circuit is working
 *  
 *  Test 1, Make sure the 10MHz clock is running
 *  Test 2, Clear the flip flops and make sure the run latches are clear
 *  Test 3, Trigger the flip flops and make sure the run latche are set
 *  
 *--------------------------------------------------------------*/
 bool POST_counters(void)
 {
   unsigned int i;                  // Iteration counter
   bool         test1, test2, test3;// Record if the test failed
   
/*
 * The test only works on V2.2 and higher
 */
  if ( DLT(DLT_CRITICAL) )
  {
    printf("POST_counters()");
  }

  test1 = false;                 // Start of assuming a fail
  test2 = false;
  test3 = false;
  
  set_status_LED("YRR");
  vTaskDelay(ONE_SECOND);
  for (i=0; i != 1000; i++)           // Clock Running
  {
    if ( gpio_get_level(REF_CLK) != 0 )
    {
      set_status_LED("G--");
      test1 = true;
      break;
    }
  }
  if ( test1 == false )
  {
      set_status_LED("R--");
      DLT(DLT_CRITICAL);
      printf("REF_CLK not running");
      vTaskDelay(5*ONE_SECOND);
  }

  gpio_set_level(CLOCK_START, 1);  
  gpio_set_level(STOP_N, 0);
  gpio_set_level(STOP_N, 1);        // Latch empty when stopped
  set_status_LED("-YR");
  vTaskDelay(ONE_SECOND);
  if ( is_running() == 0  )
  {
    test2 = true;
    set_status_LED("-G-");
  }    
  if ( test2 == false )
  {
      set_status_LED("-R-");
      DLT(DLT_CRITICAL);
      printf("Stuck bit in run latch: %02X", (~is_running()) & 0x00ff);
      vTaskDelay(5*ONE_SECOND);
  }      

  set_status_LED("--Y");
  vTaskDelay(ONE_SECOND);
  gpio_set_level(STOP_N, 0);
  gpio_set_level(STOP_N, 1);
  gpio_set_level(CLOCK_START, 1);
  gpio_set_level(CLOCK_START, 0);
  gpio_set_level(CLOCK_START, 1);
  if ( is_running() == 0xFF  )
  {
    set_status_LED("--G");
    test3 = true;
  }
  else
  {
    set_status_LED("--R");
    DLT(DLT_CRITICAL);
    printf("Failed to start clock in run latch: %02X", is_running());
    vTaskDelay(5*ONE_SECOND);
  }

/*
 * Got here, the test completed successfully
 */
  return test1 && test2 && test3;
}

/*----------------------------------------------------------------
 *
 * @function: show_sensor_status()
 *
 * @brief:    Show which sensor flip flops were latched
 *
 * @return:   Nothing
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
    vTaskDelay(ONE_SECOND);                // Wait for click to go away
  }    

/*
 * All done, return
 */

  return;
}
/*----------------------------------------------------------------
 *
 * @function: do_dlt
 *
 * @brief:    Check for a DLT log and print the time
 *
 * @return:   TRUE if the DLT should be printed
 * 
 *----------------------------------------------------------------
 * 
 * is_trace is compared to the log level and if valid the
 * current time stamp is printed
 * 
 * DLT_CRItiCAL levels are always printed
 *   
 *--------------------------------------------------------------*/
bool do_dlt
  (
  unsigned int level
  )
{ 
  if ((level & (is_trace | DLT_CRITICAL)) == 0 )
  {
    return false;      // Send out if the trace is higher than the level 
  }

  printf("\n\r%10.6fs: ",esp_timer_get_time()/1000000.0 );

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
