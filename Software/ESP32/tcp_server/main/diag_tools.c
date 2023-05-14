
/*----------------------------------------------------------------
 *
 * diag_tools.c
 *
 * Debug and test tools 
 *
 *---------------------------------------------------------------*/

#include "freETarget.h"
#include "gpio.h"
#include "diag_tools.h"
#include "nonvol.h"
#include "analog_io.h"
#include "token.h"            // Time provided by the token ring
#include "json.h"
#include "stdio.h"
#include "esp_random.h"
#include "compute_hit.h"
#include "math.h"
#include "serial_io.h"

const char* which_one[4] = {"North:", "East:", "South:", "West:"};

#define TICK(x) (((x) / 0.33) * OSCILLATOR_MHZ)   // Distance in clock ticks
#define RX(Z,X,Y) (16000 - (sqrt(sq(TICK(x)-s[(Z)].x) + sq(TICK(y)-s[(Z)].y))))
#define GRID_SIDE 25                              // Should be an odd number
#define TEST_SAMPLES ((GRID_SIDE)*(GRID_SIDE))

static void  show_analog_on_PC(int v);
static void  unit_test(unsigned int mode);
static bool_t sample_calculations(unsigned int mode, unsigned int sample);
static void  log_sensor(int sensor);
extern int   json_clock[4];

extern void sound_test(void);
extern long in_shot_timer;

/*----------------------------------------------------------------
 *
 * function: void self_test
 *
 * brief: Execute self tests based on the jumper settings
 * 
 * return: None
 *
 *----------------------------------------------------------------
 *   
 *   This function is a large case statement with each element
 *   of the case statement 
 *--------------------------------------------------------------*/
unsigned int tick;
void self_test
(
  unsigned int test                 // What test to execute
)
{
  unsigned int i;
  char         ch;
  unsigned int sensor_status;       // Sensor running inputs
  unsigned long sample;             // Sample used for comparison
  unsigned int random_delay;        // Random sampe time
  bool_t       pass;
  shot_record_t shot;               // Shot history
  char s[128];             // Text buffer
  double       volts;

/*
 *  Update the timer
 */
  tick++;
  volts = TO_VOLTS(analogRead(V_REFERENCE));
  
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
      printf("\r\n 2 - Counter values (internal trigger)");
      printf("\r\n 3 - Advance paper backer");
      printf("\r\n 4 - Spiral Unit Test");
      printf("\r\n 5 - Grid calibration pattern");
      printf("\r\n 6 - One time calibration pattern");
      printf("\r\n 7 - Grid calibration pattern");
      printf("\r\n 8 - Aux port passthrough");
      printf("\r\n 9 - Calibrate"); 
      printf("\r\n10 - Transfer loopback");
      printf("\r\n11 - Serial port test");
      printf("\r\n12 - LED brightness test");
      printf("\r\n13 - Face strike test");
      printf("\r\n14 - WiFi test");
      printf("\r\n15 - Dump NonVol");
      printf("\r\n16 - Send sample shot record");
      printf("\r\n17 - Show WiFi status");
      printf("\r\n18 - Send clock out of all serial ports");
      printf("\r\n19 - Unit Test speed_of_sound()");
      printf("\r\n20 - Token Ring Test()");
      printf("\r\n21 - Count on the LEDs");
      printf("\r\n");
      break;

/*
 * Test 1, Display GPIO inputs
 */
    case T_DIGITAL: 
      digital_test();
      break;

/*
 * Test 2, 3, Test the timing circuit
 */
    case T_TRIGGER:                       // Show the timer values (Wait for analog input)
      printf("\r\nWaiting for Trigger\r\n");
    case T_CLOCK:                        // Show the timer values (Trigger input)
      stop_timers();
      arm_timers();

      set_LED(L('*', '-', '-'));
      
      if ( test == T_CLOCK )
      {
        if ( revision() >= REV_220 )  
        {
          random_delay = esp_random() % 6000;   // Pick a random delay time in us
          printf("\r\nRandom clock test: %dus. All outputs must be the same. ", random_delay);
          trip_timers();
          delayMicroseconds(random_delay);  // Delay a random time
        }
        else
        {
          printf("\r\nThis test not supported on this hardware revision");
          break;
        }
      }
  
      while ( !is_running() )
      {
        continue;
      }
      sensor_status = is_running();       // Remember all of the running timers
      stop_timers();
      read_timers(&shot.timer_count[0]);

      if ( test == T_CLOCK )       // Test the results
      {
        sample = shot.timer_count[N];
        pass = true;

        for(i=N; i<=W; i++)
        {
          if ( shot.timer_count[i] != sample )
          {
            pass = false;                 // Make sure they all match
          }
        }

        if ( pass == true )
        {
          printf(" PASS\r\n");
        }
        else
        {
          printf(" FAIL\r\n");
        }
      }
      send_timer(sensor_status);
      
      set_LED(L('-', '-', '-'));
      delay(ONE_SECOND);
      break;

/*
 * Test 6, Advance the paper
 */
    case T_PAPER: 
      printf("\r\nAdvancing backer paper %dms  %d steps", ((json_paper_time) + (json_step_time)) * 10, json_step_count);
      drive_paper();
      printf("\r\nDone");
      break;

/*
 * Test 7, 8, 9, Generate test pattern for diagnosing software problems
 */
    case T_SPIRAL: 
      printf("\r\nSpiral Calculation\r\n");
      unit_test( T_SPIRAL );            // Generate a spiral
      break;

    case T_GRID:
      printf("\r\nGrid Calculation\r\n");
      unit_test( T_GRID);               // Generate a grid
      break;  
      
    case T_ONCE:
      printf("\r\nSingle Calculation\r\n");
      unit_test( T_ONCE);               // Generate a SINGLE calculation
      break;

/*
 * Test 10, Test the pass through connector
 */
    case T_PASS_THRU:
      printf("\r\nPass through active.  Cycle power to exit\r\n");
      while (1)
      {
        if ( serial_available(CONSOLE))
        {
          ch = serial_getch(CONSOLE);
          char_to_all(ch, AUX);
        }
        if ( serial_available(AUX))
        {
          ch = serial_getch(AUX);
          char_to_all(ch, CONSOLE);
        }
      }
      break;

/*
 * Test 11
 */
    case T_SET_TRIP:
      set_trip_point(0);          // Stay in the trip point loop
      break;

/*
 * Test 13
 */
    case T_SERIAL_PORT:
      serial_to_all("\r\nArduino Serial Port: Hello World\r\n", true, true);
      break;

/* 
 *  Test 14
 */
    case T_LED:
      printf("\r\nRamping the LED");
      for (i=0; i != 256; i++)
      {
        analogWrite(LED_PWM, i);
        delay(ONE_SECOND/50);
      }
      for (i=255; i != -1; i--)
      {
        analogWrite(LED_PWM, i);
        delay(ONE_SECOND/50);
      }
      analogWrite(LED_PWM, 0);
      printf(" Done\r\n");
      break;

 /*
  * Test 15
  */
    case T_FACE:
      printf("\r\nFace strike test\n\r");
      face_strike = 0;                        // Reset the interrupt count
      sample = 0;
      enable_face_interrupt();
      ch = 0;
      
      while ( ch != '!' )
      {        
        if ( face_strike != 0 )
        {
          face_strike--;
          set_LED(L('*', '*', '*'));           // If something comes in, turn on all of the LEDs 
        }
        else
        {
          set_LED(L('.', '.', '.'));          // face_strike complete
        }
        if ( sample != face_strike )          // If there is a change, display it
        {
          if ( (face_strike % 20) == 0 )
          {
            printf("\r\n");
          }
          printf(" S: %d", face_strike);
          sample = face_strike;        
        }
        ch = get_all(ALL);
    }
    printf("\r\nDone\n\r");
    break;

 /*
  * TEST 16 WiFI
  */
   case T_WIFI:
   break;

/*
 * TEST 17 Dump NonVol
 */
   case T_NONVOL:
    dump_nonvol();
   break;

  
/*
 * Test 18 Sample shot value 
 */
  case T_SHOT:
#if ( CLOCK_TEST )
    printf(" Shot Test using entered values"); // {"NC":30, "EC":0, "SC":80, "WC":83, "TEST":18} 
    is_trace = 0x255;       // Turn on all tracing
    shot.face_strike = 0;
    shot.timer_count[N] = 4096 - json_clock[N];
    shot.timer_count[E] = 4096 - json_clock[E];
    shot.timer_count[S] = 4096 - json_clock[S];
    shot.timer_count[W] = 4096 - json_clock[W];
    compute_hit(&shot);
#else
    shot.x = 10;
    shot.y = 20;
#endif
    shot.shot_time = (FULL_SCALE - in_shot_timer);
    shot.shot_number = 1;
    send_score(&shot);
    break;
    
 /*
  * TEST 19 WiFi Status
  */
   case T_WIFI_STATUS:
   break;
   
 /*
  * TEST 20 WiFi Broadcast
  */
   case T_WIFI_BROADCAST:
    serial_to_all("Type ! to exit ", ALL); 
    ch = 0;
    while( ch != '!' )
    {
      i = millis() / 1000;
      if ( (i % 60) == 0 )
      {
        sprintf(&s, "\r\n%d:%d ", i/60, i % 60);
        serial_to_all(s, ALL);
      }
      else
      {
      sprintf(&s, " %d:%d ", i/60, i % 60);
      serial_to_all(s, ALL);
      }
    }
    sprintf(s, "\r\nDone");
   break;
   
/*
 * Test 26 Test speed_of_sound()
 */
  case T_S_OF_SOUND:
    sound_test();
    printf("\n\rDone");
    break;

/*
 * Test 27 Token Ring Test
 */
  case T_TOKEN:
    token_init();
    printf("\n\rDone");
    break;

/*
 * Test 28 Cycle the LEds
 */
  case T_LED_CYCLE:
    i = 0;
    while ( !json_spool_available() )                     // Keep blinking the LEDs
    {
      set_LED( (i>>0) & 1, (i>>1) & 1, (i>>2)&1 );
      delay(ONE_SECOND/2);
      i++;
      token_poll();                                       // Until something arrives
    }
    printf("\n\rDone");
    break;
  }
 
 /* 
  *  All done, return;
  */
    return;
}

/*----------------------------------------------------------------
 * 
 * function: POST_version()
 * 
 * brief: Show the Version String
 * 
 * return: None
 * 
 *----------------------------------------------------------------
 *
 *  Common function to show the version. Routed to the selected
 *  port(s)
 *  
 *--------------------------------------------------------------*/
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

  set_LED(L('*', '.', '.'));
  delay(ONE_SECOND/4);
  set_LED(L('.', '*', '.'));
  delay(ONE_SECOND/4);
  set_LED(L('.', '.', '*'));
  delay(ONE_SECOND/4);
  set_LED(L('.', '.', '.'));
  
  return;
 }

/*----------------------------------------------------------------
 * 
 * function: void POST_counteres()
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
   unsigned int i, j;            // Iteration counter
   unsigned int random_delay;    // Delay duration
   unsigned int sensor_status;   // Sensor status
   int          x;               // Time difference (signed)
   bool_t       test_passed;     // Record if the test failed
   long         now;             // Current time
   
/*
 * The test only works on V2.2 and higher
 */

  if ( revision() < REV_300 )
  {
    return true;                      // Fake a positive response  
  }
  
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
  delay(1);                           // Wait a millisecond  
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
    random_delay = esp_random() % 6000; // Pick a random delay time in us
    now = micros();                     // Grab the current time
    trip_timers();
    sensor_status = is_running();       // Remember all of the running timers

    while ( micros() < (now + random_delay ) )
    {
      continue;
    }
    
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
        printf("Failed Clock Test. Counter: %c  Is: %d  Should be: %d   Delta: %d", nesw[j], read_counter(j), random_delay, x);
        test_passed = false;          // since there is delay  in
      }                               // Turning off the counters
    }
  }
  
/*
 * Got here, the test completed successfully
 */
  set_LED(L('.', '.', '.'));
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
   
   set_trip_point(20);              // Show the trip point once (20 cycles used for blinking values)
   set_LED(LED_RESET);               // Show test test Ending
   return;
 }
 
/*----------------------------------------------------------------
 * 
 * function: set_trip_point
 * 
 * brief: Read the pot and display the voltage on the LEDs as a grey code
 * 
 * return: Potentiometer set for the desired trip point
 *----------------------------------------------------------------
 *
 * The various running registers and displays them for use
 *  
 *--------------------------------------------------------------*/
#define CT(x) (1023l * (long)(x+25) / 5000l )   // 1/16 volt = 12.8 counts
#define SPEC_RANGE   50            // Out of spec if within 50 couts of the rail
#define BLINK        0x80
#define NOT_IN_SPEC  0x40
//                                         0           1         2       3         4        5        6        7        8        9        10      11        12          13      14          15
const unsigned int volts_to_LED[] = { NOT_IN_SPEC,     1,    BLINK+1,    2,     BLINK+2,    3,    BLINK+3,    4,    BLINK+4,    5,    BLINK+5,    6,     BLINK+6,       7,   BLINK+7,  NOT_IN_SPEC, 0 };
const unsigned int mv_to_counts[] = {   CT(350),    CT(400), CT(450), CT(500),  CT(550), CT(600), CT(650), CT(700), CT(750), CT(800), CT(900), CT(1000), CT(1100), CT(1200), CT(1300),   CT(5000),  0 };

static void start_over(void)    // Start the test over again
{
  stop_timers();
  arm_timers();                 // Reset the latch state
  enable_face_interrupt();      // Turn on the face strike interrupt
  face_strike = 0;              // Reset the face strike count
  enable_face_interrupt();      // Turning it on above creates a fake interrupt with a disable
  return;
}

void set_trip_point
  (
  int pass_count                                            // Number of passes to allow before exiting (0==infinite)
  )
{
  bool_t        stay_forever;                               // Stay forever if called with pass_count == 0;
  
  printf("Setting trip point. Type ! of cycle power to exit\r\n");

  sensor_status = 0;                                        // No sensors have tripped
  stay_forever = false;
  if (pass_count == 0 )                                     // A pass count of 0 means stay
  {
    stay_forever = true;                                    // For a long time
  }
  arm_timers();                                             // Arm the flip flops for later
  enable_face_interrupt();                                  // Arm the face sensor
  disable_timer_interrupt();                                // Prevent the timer from overwriting
  face_strike = 0;

/*
 * Set the PWM at 50%
 */
  json_vset_PWM = 128;
  set_vset_PWM(json_vset_PWM);
  nvs_set_i32(my_handle, NONVOL_VSET_PWM, json_vset_PWM);

/*
 * Loop if not in spec, passes to display, or the CAL jumper is in
 */
  while ( ( stay_forever )                                  // Passes to go
          ||   ( CALIBRATE )                                // Held in place by DIP switch
          ||   (pass_count != 0))                           // Wait here for N cycles
  {
/*
 * Got to the end.  See if we are going to do this for a fixed time or forever
 */
    switch (get_all(ALL))
    {
      case '!':                       // ! waiting in the serial port
        printf("\r\nExiting calibration\r\n");
        return;

      case 'B':
      case 'b':                       // Blink the Motor Drive
        printf("\r\nBlink Motor Drive\r\n");
        paper_on_off(1);
        delay(ONE_SECOND);
        paper_on_off(0);
        break;
      
      case 'W':
      case 'w':                      // Test the WiFI
        printf("\r\nTest WiFi");
        break;
        
      case 'R':
      case 'r':                       // Reset Cancel
      case 'X':
      case 'x':                       // X Cancel
        start_over();
        break;
        
      default:
        break;
    }
    

   show_sensor_status(is_running());
   printf("\n\r");
   if ( stay_forever )
   {
      if ( (is_running() == 0x0f) && (face_strike != 0) )
      {
        start_over();
      }
   }
   else
   {
     if ( pass_count != 0 )             // Set for a finite loop?
     {
        pass_count--;                   // Decriment count remaining
        if ( pass_count == 0 )          // And bail out when zero
        {
          return;
        }
      }
   }
   delay(ONE_SECOND/5);
 }

 /*
  * Return
  */
  enable_timer_interrupt();
  return;
}

/*----------------------------------------------------------------
 *
 * function: unit_test
 *
 * brief: Setup a known target for sample calculations
 * 
 * return: None
 *
 *----------------------------------------------------------------
 * 
 * See excel spread sheet sample calculations.xls
 * 
 * Estimate 0.02mm / delta count   
 *   --> 400 counts -> 8mm
 *   
 *--------------------------------------------------------------*/

/*
 * Prompt the user for a test number and execute the test.
 */
static void unit_test(unsigned int mode)
{
  unsigned int i;
  unsigned int shot_number;
  
 /*
  * Auto Generate spiral
  */
  init_sensors();
  shot_number = 1;
  for ( i = 0; i != TEST_SAMPLES; i++)
  {
    if ( sample_calculations(mode, i) )
    {
      compute_hit(&record[0]);
      sensor_status = 0xF;        // Fake all sensors good
      record[0].shot_number = shot_number++;
      send_score(&record[0]);
      delay(ONE_SECOND/2);        // Give the PC program some time to catch up
    }
    if ( mode == T_ONCE )
    {
      break;
    }
  }

/*
 * All done, return
 */
  return;
}

/*----------------------------------------------------------------
 *
 * function: sample_calculations
 *
 * brief: Work out the clock values to generate a particular pattern
 *
 * return: TRUE to be compatable with other calcuation functions
 * 
 *----------------------------------------------------------------
 * 
 * This function is used to generate a test pattern that the
 * PC or Arduino software is compared to.
 *   
 *--------------------------------------------------------------*/
/*
 * Fill up counters with sample values.  Return false if the sample does not exist
 */
static bool_t sample_calculations
  (
  unsigned int mode,            // What test mode are we generating
  unsigned int sample           // Current sample number
  )
{
  double x, y;                  // Resulting target position
  double angle;                 // Polar coordinates
  double radius;
  double polar;
  int    ix, iy;
  double grid_step;
  shot_record_t shot;
  
  switch (mode)
  {
/*
 * Generate a single calculation
 */
  case T_ONCE:
    angle = 0;
    radius =json_sensor_dia / sqrt(2.0d) / 2.0d;
    
    x = radius * cos(angle);
    y = radius * sin(angle);
    shot.timer_count[N] = RX(N, x, y);
    shot.timer_count[E] = RX(E, x, y);
    shot.timer_count[S] = RX(S, x, y);
    shot.timer_count[W] = RX(W, x, y);
    shot.timer_count[W] -= 200;              // Inject an error into the West sensor

    printf("\r\nResult should be: ");   printf("x:"); Serial.print(x); printf(" y:"); Serial.print(y); printf(" radius:"); Serial.print(radius); printf(" angle:"); Serial.print(angle * 180.0d / PI);
    break;

 /*
 * Generate a spiral pattern
 */
  default:
  case T_SPIRAL:
    angle = (PI_ON_4) / 5.0 * ((double)sample);
    radius = 0.99d * (json_sensor_dia/2.0) / sqrt(2.0d) * (double)sample / TEST_SAMPLES;

    x = radius * cos(angle);
    y = radius * sin(angle);
    shot.timer_count[N] = RX(N, x, y);
    shot.timer_count[E] = RX(E, x, y);
    shot.timer_count[S] = RX(S, x, y);
    shot.timer_count[W] = RX(W, x, y);
    break;

 /*
 * Generate a grid
 */
  case T_GRID:
    radius = 0.99d * (json_sensor_dia / 2.0d / sqrt(2.0d));                      
    grid_step = radius * 2.0d / (double)GRID_SIDE;

    ix = -GRID_SIDE/2 + (sample % GRID_SIDE);      // How many steps
    iy = GRID_SIDE/2 - (sample / GRID_SIDE);

    x = (double)ix * grid_step;                     // Compute the ideal X-Y location
    y = (double)iy * grid_step;
    polar = sqrt(sq(x) + sq(y));
    angle = atan2(y, x) - (PI * json_sensor_angle / 180.0d);
    x = polar * cos(angle);
    y = polar * sin(angle);                        // Rotate it through the sensor position.

    if ( sqrt(sq(x) + sq(y)) > radius )
    {
      return false;
    }

    shot.timer_count[N] = RX(N, x, y);
    shot.timer_count[E] = RX(E, x, y);
    shot.timer_count[S] = RX(S, x, y);
    shot.timer_count[W] = RX(W, x, y);
    break;   
  }
  
/*
 * All done, return
 */
  return true;
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
  unsigned int   sensor_status,
  shot_record_t* shot
  )
{
  unsigned int i;
  
  printf(" Latch:");

  for (i=N; i<=W; i++)
  {
    if ( sensor_status & (1<<i) )   Serial.print(nesw[i]);
    else                            printf(".");
  }

  if ( shot != 0 )
  {
    Serial.print(" Timers:");

    for (i=N; i<=W; i++)
    {
      printf(" ");  Serial.print(nesw[i]); printf(":"); Serial.print(shot->timer_count[i]); 
    }
  }
  
  printf("  Face Strike:"); Serial.print(face_strike);
  
  printf("  V_Ref:"); Serial.print(TO_VOLTS(analogRead(V_REFERENCE)));
  
  printf("  Temperature:"); Serial.print(temperature_C());
  
  printf("  WiFi:"); Serial.print(esp01_is_present());                           // TRUE if WiFi is available

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
 * function: log_sensor()
 *
 * brief:    Sample and display the North sensor value
 *
 * return:   Nothing
 * 
 *----------------------------------------------------------------
 * 
 * The North sensor is sampled for one second and the maximum
 * for that sample is displayed.
 * 
 * The process repeats until the user types in a !
 *   
 *--------------------------------------------------------------*/
#define LOG_TIME  1000            // 1 second loop time

void log_sensor
  (
  int sensor                      // Sensor to be monitored
  )
{
  unsigned int i;
  unsigned long start;
  unsigned int max_cycle;         // Largest value this cycle
  unsigned int max_all;           // Largest value ever
  unsigned int sample;            // Sample read from ADC
  unsigned int sensor_status;     // Sensor running latch
  char         s[128];            // String Holder
  char         ch;                // Input character
  bool_t       is_new;            // TRUE if a change was found

  sprintf(s, "\r\nLogging %s Use X to reset,  ! to exit\r\n", which_one[sensor]);
  serial_to_all(s);
  serial_to_all(0);
  max_all =  0;
  arm_timers();
  
  while (1)
  {
/*
 * Loop for the sample time, picking up the analog voltage as quick as we can
 */
    start = LOG_TIME;                         // Pick up the starting time
    max_cycle = analogRead(channel[sensor]);
    is_new = false;
    while ( (--start) )                       // For Log Time, 
    {
      sample = analogRead(channel[sensor]);   // Read the ADC
      if ( sample > max_cycle )
      {
        max_cycle = sample;
        is_new = true;
      }
      if ( sample > max_all )
      {
        max_all = sample;
        is_new = true;
      }
    }

/*
 * If there is a change output the result
 */
    sensor_status = is_running();
    if ( is_new  || (sensor_status != 0) )
    {
      sprintf(s, "\r\n%s cycle:%d  max:%d is_running:", which_one[sensor], max_cycle, max_all);
      serial_to_all(s);   

      s[1] = 0;
      for (i=N; i<=W; i++)
      {
        if ( sensor_status & (1<<i) )   s[0] = nesw[i];
        else                            s[0] = '.';
        serial_to_all(s);
      }
      arm_timers();
    }

    while ( available_all() )
    {
      ch = get_all();
      switch ( ch )
      {
        case '!':
          sprintf(s, "\r\nDone");
          serial_to_all(s);
          return;

        case 'x':
        case 'X':
          max_all = 0;
          max_cycle = 0;
          break;
      }
    }
  }

/*  
 *   We never get here
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
  char s[20], str[20];

  if ((level & (is_trace | DLT_CRITICAL)) == 0 )
  {
    return false;      // Send out if the trace is higher than the level 
  }

  dtostrf(micros()/1000000.0, 7, 6, str );
  sprintf(s, "\n\r%s: ", str);
  Serial.print(s);

  return true;
}
