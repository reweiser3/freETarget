
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
#include "stdio.h"
#include "serial_io.h"
#include "gpio_types.h"
#include "driver\gpio.h"

#include "freETarget.h"
#include "gpio.h"
#include "diag_tools.h"
#include "analog_io.h"
#include "json.h"
#include "timer.h"
#include "esp_timer.h"
#include "dac.h"
#include "pwm.h"
#include "pcnt.h"
#include "analog_io.h"
#include "gpio_define.h"
#include "WiFi.h"

const char* which_one[] = {"North_lo", "East_lo ", "South_lo", "West_lo ", "North_hi", "East_hi ", "South_hi", "West_hi "};

#define TICK(x) (((x) / 0.33) * OSCILLATOR_MHZ)   // Distance in clock ticks
#define RX(Z,X,Y) (16000 - (sqrt(sq(TICK(x)-s[(Z)].x) + sq(TICK(y)-s[(Z)].y))))
#define GRID_SIDE 25                              // Should be an odd number
#define TEST_SAMPLES ((GRID_SIDE)*(GRID_SIDE))

/*
static int   get_int(void);         // Get an integer from the user
static float get_float(void);       // Get a float from the user
*/

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
      printf("\r\n 7 - PCNT Test");
      printf("\r\n 8 - Sensor POST test");
      printf("\r\n 9 - AUX Port loopback");
      printf("\r\n10 - Polled Target Test");
      printf("\r\n11 - Interrupt Target Test");
      printf("\r\n12 - Test WiFi as an Access Point");
      printf("\r\n13 - Test WiFI as a station"); 
      printf("\r\n14 - Enable the WiFi Server");
      printf("\r\n15 - Loopback the TCPIP data");
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
      paper_test();
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
      status_LED_test();
      break;

/*
 * Test 5,  Analog In
 */
    case T_TEMPERATURE:
      analog_input_test();
      break;

/*
 * Test 6, DAC
 */
    case T_DAC:
      DAC_test();
      break;

/*
 * Test 7, PCNT test
 */
    case T_PCNT:
      pcnt_test();
      break;

/*
 *  Test 8: Sensor Trigger
 */
    case T_SENSOR:
      POST_counters();
      break;

/*
 *  Test 9: AUX Serial Port
 */
    case T_AUX_SERIAL:
      serial_port_test();
      break;

/*
 *  Test 10: Polled Target Test
 */
    case T_TARGET:
      polled_target_test();
      break;      
/*
 *  Test 11: Polled Target Test
 */
    case T_TARGET_2:
      interrupt_target_test();
      break; 

/*
 *  Test 12: Start WiFi AP
 */
    case T_WIFI_AP:
      WiFi_AP_init();
      break; 

/*
 *  Test 13: Start WiFi station
 */
    case T_WIFI_STATION:
      WiFi_station_init();
      break; 

/*
 *  Test 14: Enable the WiFi Server
 */
    case T_WIFI_SERVER:
      xTaskCreate(WiFi_tcp_server_task,    "WiFi_tcp_server",      4096, NULL, 5, NULL);
      break; 

/*
 *  Test 15: Send and receive something
 */
    case T_WIFI_STATION_LOOPBACK:
      WiFi_station_init();
      xTaskCreate(WiFi_tcp_server_task,    "WiFi_tcp_server",      4096, NULL, 5, NULL);
      xTaskCreate(tcpip_accept_poll,       "tcpip_accept_poll",    4096, NULL, 4, NULL);
      WiFi_loopback_test();
      break; 

/*
 *  Test 16: Send and receive something
 */
    case T_WIFI_AP_LOOPBACK:
      WiFi_AP_init();
      xTaskCreate(WiFi_tcp_server_task,    "WiFi_tcp_server",      4096, NULL, 5, NULL);
      xTaskCreate(tcpip_accept_poll,       "tcpip_accept_poll",    4096, NULL, 4, NULL);
      WiFi_loopback_test();
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

  printf("\n\rI (%d) ", (int)(esp_timer_get_time()/1000.0) );

  return true;
}

#if(0)
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
#endif