/*-------------------------------------------------------
 * 
 * nonvol.c
 * 
 * Nonvol storage managment
 * 
 *-------------------------------------------------------
 *
 * See https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/nvs_flash.html
 * 
 * ----------------------------------------------------*/
#include "freETarget.h"
#include "json.h"
#include "nonvol.h"

#include "gpio.h"
#include "diag_tools.h"
#include "stdio.h"

#include "nvs_flash.h"
#include "nvs.h"

/*
 *  Local variables
 */
static nvs_handle_t my_handle;

/*----------------------------------------------------------------
 * 
 * funciton: check_nonvol
 * 
 * brief:    Read nonvol and set up variables
 * 
 * return:   Nonvol values copied to RAM
 * 
 *---------------------------------------------------------------
 *
 * Check to see if the NONVOL has been initialized correctly
 *
 *------------------------------------------------------------*/
void check_nonvol(void)
{
  unsigned int nonvol_init;
  
  if ( DLT(DLT_DIAG) )
  {
    printf("check_nonvol()");
  }
  
/*
 * Read the nonvol marker and if uninitialized then set up values
 */
  nvs_get_i32(my_handle, "NONVOL_INIT", &nonvol_init);
  if ( nonvol_init != INIT_DONE)                       // EEPROM never programmed
  {
    factory_nonvol(true);                              // Force in good values
  }

/*
 * Check to see if there has been a change to the persistent storage version
 */
  nvs_get_i32(my_handle, "NONVOL_PS_VERSION", &nonvol_init);
  if ( nonvol_init != PS_VERSION )                    // Is what is in memory not the same as now
  {
    backup_nonvol();                                  // Copy what we have 
    update_nonvol(nonvol_init);                       // Then update the version
  }
  
/*
 * All OK now
 */
  return;
}

/*----------------------------------------------------------------
 * 
 * function: factory_nonvol
 * 
 * brief: Initialize the NONVOL back to factory settings
 * 
 * return: None
 *---------------------------------------------------------------
 *
 * If the init_nonvol location is not set to INIT_DONE then
 * initilze the memory
 * 
 *------------------------------------------------------------*/
void factory_nonvol
  (
   bool_t new_serial_number
  )
{
  unsigned int nonvol_init;               // Initialization token
  unsigned int serial_number;             // Board serial number
  char         ch;
  unsigned int x;                         // Temporary Value
  double       dx;                        // Temporarty Value
  unsigned int i;                         // Iteration Counter
  
/*
 * Fill up all of memory with a known (bogus) value
 */
  printf("\r\nReset to factory defaults. This may take a while\r\n");
  ch = 0xAB;
  for ( i=0; i != NONVOL_SIZE; i++)
  {
    if ( (i < NONVOL_SERIAL_NO) || (i >= NONVOL_SERIAL_NO + sizeof(int) + 2) ) // Bypass the serial number
    {
      EEPROM.put(i, ch);                    // Fill up with a bogus value
      if ( (i % 64) == 0 )
      {
        printf(".");
      }
    }
  }
  
  gen_position(0); 
  x = 0;
  EEPROM.put(vset_PWM, x);
  if ( new_serial_number == false )
  {
    EEPROM.put(NONVOL_SERIAL_NO, serial_number);  // Put the serial number back
  }
 
/*
 * Use the JSON table to initialize the local variables
 */
  i=0;
  while ( JSON[i].token != 0 )
  {
    switch ( JSON[i].convert )
    {
       case IS_VOID:                                        // Variable does not contain anything 
       case IS_FIXED:                                       // Variable cannot be overwritten
       break;
       
       case IS_TEXT:
       case IS_SECRET:
        printf("\r\n%s \"\"", JSON[i].token);
        if ( JSON[i].non_vol != 0 )
        {
          EEPROM.put(JSON[i].non_vol, 0);                    // Zero out the text
        }
        break;
        
      case IS_INT16:
        x = JSON[i].init_value;                            // Read in the value 
        printf("\r\n%d %d", JSON[i].token, x);
        if ( JSON[i].non_vol != 0 )
        {
          EEPROM.put(JSON[i].non_vol, x);                    // Read in the value
        }
        break;

      case IS_FLOAT:
        dx = (double)JSON[i].init_value;
        printf("\r\n%s %d", JSON[i].token, dx);
        EEPROM.put(JSON[i].non_vol, dx);                    // Read in the value
        break;
    }
   i++;
 }
  printf("\r\nDone\r\n");
  
/*    
 *      Make a backup of the settings
 */  backup_nonvol();

/*    
 *     Test the board only if it is a factor init
 */
  if ( new_serial_number )
  {
    printf("\r\n Testing motor drive ");
    for (x=10; x != 0; x--)
    {
      Serial.print(x); printf("+ ");
      paper_on_off(true);
      delay(ONE_SECOND/4);
      printf("- ");
      paper_on_off(false);
      delay(ONE_SECOND/4);
    }
    printf(" Test Complete\r\n");
  }
/*
 * Set the trip point
 */
  set_trip_point(0);                      // And stay forever in the set trip mode

/*
 * Ask for the serial number.  Exit when you get !
 */
  if ( new_serial_number )
  {
    ch = 0;
    serial_number = 0;
    while ( Serial.available() )    // Eat any pending junk
    {
      Serial.read();
    }
  
    printf("\r\nSerial Number? (ex 223! or x))");
    while (i)
    {
      if ( Serial.available() != 0 )
      {
        ch = Serial.read();
        if ( ch == '!' )
        {  
          EEPROM.put(NONVOL_SERIAL_NO, serial_number);
          printf(" Setting Serial Number to: "); Serial.print(serial_number);
          break;
        }
        if ( ch == 'x' )
        {
          break;
        }
        serial_number *= 10;
        serial_number += ch - '0';
      }
    }
  }
  
/*
 * Initialization complete.  Mark the init done
 */
  nonvol_init = PS_VERSION;
  EEPROM.put(NONVOL_PS_VERSION, nonvol_init); // Write in the version number

  nonvol_init = INIT_DONE;
  EEPROM.put(NONVOL_INIT, nonvol_init);

/*
 * Read the NONVOL and print the results
 */
  read_nonvol();                          // Read back the new values
  show_echo();                            // Display these settings
  
/*
 * All done, return
 */    

  return;
}

 
/*----------------------------------------------------------------
 * 
 * function: init_nonvol
 * 
 * brief: Initialize the NONVOL back to factory settings
 * 
 * return: None
 *---------------------------------------------------------------
 *
 * init_nonvol requires an arguement == 1234 before the 
 * initialization command will be executed.
 * 
 * The variable NONVOL_INIT is corrupted. and the values
 * copied out of the JSON[] table.  If the serial number has
 * not been initialized before, the user is prompted to enter
 * a serial number.
 * 
 * The function then continues to display the current trip
 * point value.
 * 
 *------------------------------------------------------------*/
 #define INIT_ALLOWED         1234        // Number user must enter to allow initialization
 #define INIT_SERIAL_NUMBER   1235        // Number used to re-enter the serial number
 
void init_nonvol
  (
    int verify                            // Verification code entered by user
  )
{
/*
 * Ensure that the user wants to init the unit
 */
  if ( (verify != INIT_ALLOWED) && (verify != INIT_SERIAL_NUMBER) )
  {
    printf("\r\nUse {\"INIT\":1234}\r\n");
    return;
  }

  factory_nonvol(verify == INIT_SERIAL_NUMBER);
  
/*
 * All done, return
 */    

  return;
}

/*----------------------------------------------------------------
 * 
 * funciton: read_nonvol
 * 
 * brief: Read nonvol and set up variables
 * 
 * return: Nonvol values copied to RAM
 * 
 *---------------------------------------------------------------
 *
 * Read the nonvol into RAM.  
 * 
 * If the results is uninitalized then force the factory default.
 * Then check for out of bounds and reset those values
 *
 *------------------------------------------------------------*/
void read_nonvol(void)
{
  unsigned int nonvol_init;
  unsigned int  i, j;           // Iteration Counter
  unsigned int  x;              // 16 bit number
  double       dx;              // Floating point number
  unsigned char ch;             // Text value
  esp_err_t err;
  
  if ( DLT(DLT_CRITICAL) )
  {
    printf("read_nonvol()");
  }
  
/*
 * Read the nonvol marker and if uninitialized then set up values
 */
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    nvs_flash_erase();
    nvs_flash_init();
  }

  err = nvs_open(NAME_SPACE, NVS_READWRITE, &my_handle);
  if (err != ESP_OK)
  {
    DLT(DLT_CRITICAL);
    printf("read_nonvol(): Failed to open NVM");
  }
        
  err = nvs_get_i32(my_handle, "NONVOL_INIT", &nonvol_init);
  switch (err)
  {
    case ESP_OK:
      break;

      case ESP_ERR_NVS_NOT_FOUND:
        printf("The value is not initialized yet!\n");
        break;
   
      default :
        printf("Error (%s) reading!\n", esp_err_to_name(err));
  }
  
  if ( nonvol_init != INIT_DONE)                       // EEPROM never programmed
  {
    factory_nonvol(true);                              // Force in good values
  }

  nvs_get_i32(my_handle, "NVM_SERIAL_NO", &nonvol_init);

  if ( nonvol_init == (-1) )                          // Serial Number never programmed
  {
    factory_nonvol(true);                             // Force in good values
  }

  nvs_get_i32(my_handle, "NVM_PS_VERSION", &nonvol_init);
  if ( nonvol_init != PS_VERSION )                    // persistent storage version
  {
    update_nonvol(nonvol_init);
  }
  
/*
 * Use the JSON table to initialize the local variables
 */
 i=0;
 while ( JSON[i].token != 0 )
 {
   if ( (JSON[i].value != 0) || (JSON[i].d_value != 0)  )    // There is a value stored in memory
   {
     switch ( JSON[i].convert )
     {
        case IS_VOID:
          break;
          
        case IS_TEXT:
        case IS_SECRET:
          if ( JSON[i].non_vol != 0 )                           // Is persistent storage enabled?
          {
            j=0;
            while (1)                                           // Loop and copy the NONVOL 
            { 
              nvs_get_i8(my_handle, JSON[i].non_vol, &ch);
              *((unsigned char*)(JSON[i].value) + j) = ch;     // Over to the working storage
              if ( ch == 0 )
              {
                break;
              }
              j++;
            }
          }
          break;

        case IS_INT16:
        case IS_FIXED:
          if ( JSON[i].non_vol != 0 )                          // Is persistent storage enabled?
          {
            nvs_get_i16(my_handle, JSON[i].non_vol, &x);                   // Read in the value
            if ( x == 0xABAB )                                 // Is it uninitialized?
            {
              x = JSON[i].init_value;                          // Yes, overwrite with the default
              nvs_set_i32(my_handle, JSON[i].non_vol, x);
            }
            *JSON[i].value = x;
          }
          else
          {
            *JSON[i].value = JSON[i].init_value;              // Persistent storage is not enabled, force a known value
          }
          break;

        case IS_FLOAT:
        case IS_DOUBLE:
          EEPROM.get(JSON[i].non_vol, dx);                    // Read in the value
          *JSON[i].d_value = dx;
          break;
      }
   }
   i++;
 }

/*
 * Go through and verify that the special cases are taken care of
 */
  multifunction_switch();                                   // Look for an override on the target type
  
/*
 * All done, begin the program
 */
  return;
}


/*----------------------------------------------------------------
 * 
 * function: update_nonvol
 * 
 * brief:  Update the nonvol values if there has been a change
 * 
 * return: None
 *---------------------------------------------------------------
 *
 * If the init_nonvol location is not set to INIT_DONE then
 * initilze the memory
 * 
 *------------------------------------------------------------*/

void update_nonvol
  (
    unsigned int current_version          // Version present in persistent storage
  )
{
  unsigned int i;                         // Iteration counter
  unsigned int ps_value;                  // Value read from persistent storage           
  unsigned int x;                         // Value read from table
  double       y;                         // Floating point number
  
  if ( DLT(DLT_CRITICAL) )
  {
    printf("update_nonvol(%d)", current_version);
  }
  
/*
 * Check to see if this persistent storage has never had a version number
 */
  if ( PS_UNINIT(current_version) )
  {
    printf("\n\rUpdating legacy persistent storage");
    i=0;
    while ( JSON[i].token != 0 )
    { 
      switch ( JSON[i].convert )
      {        
      case IS_INT16:
        EEPROM.get(JSON[i].non_vol, ps_value);              // Pull up the value from memory
        if ( PS_UNINIT(ps_value) )                          // Uninitilazed?
        {
          EEPROM.put(JSON[i].non_vol, JSON[i].init_value);  // Initalize it from the table
        }
        break;

      default:
        break;
      }
      i++;
   }
   current_version = PS_VERSION;                            // Initialized, force in the current version
   EEPROM.put(NONVOL_PS_VERSION, current_version);
   printf("\r\nDone\r\n");
  }

/*
 * Look through the list of PS versions and see if we have initialized before
 * Old memory has a completly bogus verion number that is not in range
 */
/*
 * Up to date, return
 */
  return;
}

/*----------------------------------------------------------------
 *
 * function: gen_postion
 * 
 * brief: Generate new position varibles based on new sensor diameter
 * 
 * return: Position values stored in NONVOL
 * 
 *---------------------------------------------------------------
 *
 *  This function resets the offsets to 0 whenever a new 
 *  sensor diameter is entered.
 *  
 *------------------------------------------------------------*/
void gen_position(int v)
{
 /*
  * Work out the geometry of the sensors
  */
  json_north_x = 0;
  json_north_y = 0;
  
  json_east_x = 0;
  json_east_y = 0;

  json_south_x = 0;
  json_south_y = 0;
  
  json_west_x = 0;
  json_west_y = 0;

 /*
  * Save to persistent storage
  */
  EEPROM.put(NONVOL_NORTH_X, json_north_x);  
  EEPROM.put(NONVOL_NORTH_Y, json_north_y);  
  EEPROM.put(NONVOL_EAST_X,  json_east_x);  
  EEPROM.put(NONVOL_EAST_Y,  json_east_y);  
  EEPROM.put(NONVOL_SOUTH_X, json_south_x);  
  EEPROM.put(NONVOL_SOUTH_Y, json_south_y);  
  EEPROM.put(NONVOL_WEST_X,  json_west_x);  
  EEPROM.put(NONVOL_WEST_Y,  json_west_y);  
   
 /* 
  *  All done, return
  */
  return;
}

