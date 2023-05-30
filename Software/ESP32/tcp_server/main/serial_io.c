/*-------------------------------------------------------
 * 
 * serial_io.c
 * 
 * General purpose Serial port driver
 * 
 *-------------------------------------------------------
 * 
 * https://docs.espressif.com/projects/esp-idf/en/v4.3/esp32/api-reference/peripherals/uart.html
 * 
 *------------------------------------------------------*/

#include "freETarget.h"
#include "diag_tools.h"
#include "stdio.h"
#include "serial_io.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"

/*
 *  Serial IO port configuration
 */

const int uart_console = UART_NUM_0;
uart_config_t uart_console_config =
{
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
};
const int uart_console_size = (1024 * 2);
QueueHandle_t uart_console_queue;

const int uart_aux = UART_NUM_1;
uart_config_t uart_aux_config =
{
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
};

const int uart_aux_size= (1024 * 2);
QueueHandle_t uart_aux_queue;

/*-----------------------------------------------------
 * 
 * function: serial_io_init
 * 
 * brief: Initalize the various Serial ports
 * 
 * return: None
 * 
 *-----------------------------------------------------
 *
 * The serial port is initialized and the interrupt 
 * driver assigned
 * 
 *-----------------------------------------------------*/

void serial_io_init(void)
{
  if ( DLT(DLT_CRITICAL) ) 
  {
    printf("serial_io_init()");  
  }

/*
 *  Setup the communications parameters
 */
  uart_param_config(uart_console, &uart_console_config);
  uart_param_config(uart_aux,     &uart_aux_config);

/*
 *  Load the driver
 */
  uart_driver_install(UART_NUM_0, uart_console_size,  uart_console_size, 10, &uart_console_queue, 0);
  uart_driver_install(UART_NUM_1, uart_aux_size,      uart_aux_size,     10, &uart_aux_queue, 0);

/*
 * All done, return
 */  
  return;
}

/*-----------------------------------------------------
 * 
 * function: serial_available
 * 
 * brief:    Determine if bytes are waiting in any port
 * 
 * return:   Number of characters in all of the ports
 * 
 *-----------------------------------------------------
 *
 * 
 *-----------------------------------------------------*/
unsigned int serial_available
(
  bool_t console,    // TRUE if reading console
  bool_t aux,        // TRUE if reading AUX port
  bool_t tcpip       // TRUE if checking the TCPIP port
)
{
  unsigned int n_available;
  unsigned int length;

  n_available = 0;

  if ( console )
  {
    uart_get_buffered_data_len(uart_console, (size_t*)&length);
    n_available += length;
  }

  if ( aux )
  {
    uart_get_buffered_data_len(uart_aux, (size_t*)&length);
    n_available += length;
  }

  return n_available;
}



/*-----------------------------------------------------
 * 
 * function: serial_flush
 * 
 * brief:    purge everything in the queue
 * 
 * return:   Number of characters in all of the ports
 * 
 *-----------------------------------------------------
 *
 * 
 *-----------------------------------------------------*/
void serial_flush
(
  bool_t console,    // TRUE if reading console
  bool_t aux,        // TRUE if reading AUX port
  bool_t tcpip       // TRUE if flushing the TCPIP channel
)
{
  if ( console )
  {
    uart_flush(uart_console);
  }

  if ( aux )
  {
    uart_flush(uart_aux);
  }

  return;
}


/*-----------------------------------------------------
 * 
 * function: serial_getch
 * 
 * brief:    Read one or more of the serial ports
 * 
 * return:   Next character in the  selected serial port
 * 
 *-----------------------------------------------------
 *
 * 
 *-----------------------------------------------------*/
char serial_getch
  (
    bool_t console,       // Read the console
    bool_t aux,           // Read the AUX port
    bool_t tcpip
  )
{
  char ch;

/*
 Bring in the console bytes
 */
  if ( console )
  {
    if ( uart_read_bytes(uart_console, &ch, 1, 0) > 0 )
    {
      return ch;
    }
  }

/*
 *  Bring in the AUX bytes
 */
  if ( aux )
  {
    if ( uart_read_bytes(uart_aux, &ch, 1, 0) > 0 )
    {
      return ch;
    }
  }

/*
 * Got nothing
 */
  return 0;
}
/*-----------------------------------------------------
 * 
 * function: serial_to_all
 * 
 * brief:    Send a string to the available serial ports
 * 
 * return:   None
 * 
 *-----------------------------------------------------
 *
 * Send a string to all of the serial devices that are 
 * in use. 
 * 
 *-----------------------------------------------------*/
 void char_to_all
 (
    char ch,
    bool_t console, 
    bool_t aux,
    bool_t tcpip
)
{
  char str_a[2];
  str_a[0] = ch;
  str_a[1] = 0;
  serial_to_all(str_a, console, aux, tcpip);
  return;
 }
 
void serial_to_all
(
  char*   str,                      // String to output
  bool_t  console,                  // Output to the console
  bool_t  aux,                      // Output to the aux port
  bool_t  tcpip                     // Output to the TCPIP socket
)
{
  unsigned int len;

/*
 *  Figure out the string length
 */
  len = 0;
  while (str[len])
  {
    len++;
  }

/*
 * Output to the devices
 */
  if ( console )
  {
    printf("%s", str);
  }
  if ( aux )
  {
    uart_write_bytes(uart_aux, (const char *) str, len);
  }
  if ( tcpip )
  {

  }

/*
 * All done
 */
  return;
}
