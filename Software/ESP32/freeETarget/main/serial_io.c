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
#include "diag_tools.h"

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
    .source_clk = UART_SCLK_DEFAULT
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
    .source_clk = UART_SCLK_DEFAULT
};

const int uart_aux_size= (1024 * 2);
QueueHandle_t uart_aux_queue;

static char tcpip_in[1024];       // TCPIP input buffer
static int  in_in_ptr  = 0;       // Queue pointers
static int  in_out_ptr = 0;       

static char tcpip_out[1024];      // TCPOP output buffer
static int  out_in_ptr  = 0;      // Queue pointers
static int  out_out_ptr = 0;

/******************************************************************************
 * 
 * @function: serial_io_init
 * 
 * @brief: Initalize the various Serial ports
 * 
 * @return: None
 * 
 *******************************************************************************
 *
 * The serial port is initialized and the interrupt 
 * driver assigned
 * 
 ******************************************************************************/

void serial_io_init(void)
{
  
/*
 *  Load the driver
 */
  uart_driver_install(UART_NUM_0, uart_console_size,  uart_console_size, 10, &uart_console_queue, 0);
  uart_driver_install(UART_NUM_1, uart_aux_size,      uart_aux_size,     10, &uart_aux_queue, 0);

/*
 *  Setup the communications parameters
 */
  uart_param_config(uart_console, &uart_console_config);
  setvbuf(stdout, NULL, _IONBF, 0);                         // Send something out as soon as you get it.
  uart_param_config(uart_aux,     &uart_aux_config);

/*
 *  Set UART pins(TX: IO4, RX: IO5, RTS: IO18, CTS: IO19)
 */
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_1, 17, 18, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

 /* 
  *  Prepare the TCPIP queues
  */
  in_in_ptr   = 0;      // Queue pointers
  in_out_ptr  = 0;       
  out_in_ptr  = 0;      // Queue pointers
  out_out_ptr = 0;

/*
 * All done, return
 */  
  return;
}

#if(0)
/* UART Echo Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define ECHO_TEST_TXD (CONFIG_EXAMPLE_UART_TXD)
#define ECHO_TEST_RXD (CONFIG_EXAMPLE_UART_RXD)
#define ECHO_TEST_RTS (UART_PIN_NO_CHANGE)
#define ECHO_TEST_CTS (UART_PIN_NO_CHANGE)

#define ECHO_UART_PORT_NUM      (CONFIG_EXAMPLE_UART_PORT_NUM)
#define ECHO_UART_BAUD_RATE     (CONFIG_EXAMPLE_UART_BAUD_RATE)
#define ECHO_TASK_STACK_SIZE    (CONFIG_EXAMPLE_TASK_STACK_SIZE)

static const char *TAG = "UART TEST";

#define BUF_SIZE (1024)

static void echo_task(void *arg)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = ECHO_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    int intr_alloc_flags = 0;

#if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
#endif

    ESP_ERROR_CHECK(uart_driver_install(ECHO_UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(ECHO_UART_PORT_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(ECHO_UART_PORT_NUM, ECHO_TEST_TXD, ECHO_TEST_RXD, ECHO_TEST_RTS, ECHO_TEST_CTS));

    // Configure a temporary buffer for the incoming data
    uint8_t *data = (uint8_t *) malloc(BUF_SIZE);

    while (1) {
        // Read data from the UART
        int len = uart_read_bytes(ECHO_UART_PORT_NUM, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
        // Write data back to the UART
        int i;
        for (i=0; i != len; i++)
        {
            data[i] += 2;
        }
        uart_write_bytes(ECHO_UART_PORT_NUM, (const char *) data, len);
        if (len) {
            data[len] = '\0';
            ESP_LOGI(TAG, "Recv str: %s", (char *) data);
        }
    }
}

void app_main(void)
{
    xTaskCreate(echo_task, "uart_echo_task", ECHO_TASK_STACK_SIZE, NULL, 10, NULL);
}
#endif
/*******************************************************************************
 * 
 * @function: serial_available
 * 
 * @brief:    Determine if bytes are waiting in any port
 * 
 * @return:   Number of characters in all of the ports
 * 
 *******************************************************************************
 *
 * 
 ******************************************************************************/
unsigned int serial_available
(
  bool console,    // TRUE if reading console
  bool aux,        // TRUE if reading AUX port
  bool tcpip       // TRUE if checking the TCPIP port
)
{
  int n_available;
  int length;

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

  if ( tcpip )
  {
    if (in_in_ptr != in_out_ptr )
    {
      length = in_in_ptr - in_out_ptr;
      if ( length < 0 )
      {
        length += sizeof(tcpip_in);
      }
      n_available += length;
    }
  }

  return n_available;
}



/*******************************************************************************
 * 
 * @function: serial_flush
 * 
 * @brief:    purge everything in the queue
 * 
 * @return:   Number of characters in all of the ports
 * 
 *******************************************************************************
 *
 * 
 ********************************************************************************/
void serial_flush
(
  bool console,    // TRUE if reading console
  bool aux,        // TRUE if reading AUX port
  bool tcpip       // TRUE if flushing the TCPIP channel
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

  if ( tcpip )
  {
    in_in_ptr  = 0;
    in_out_ptr = 0;
  }
  return;
}


/*******************************************************************************
 * 
 * @function: serial_getch
 * 
 * @brief:    Read one or more of the serial ports
 * 
 * @return:   Next character in the  selected serial port
 * 
 *******************************************************************************
 *
 * 
 *******************************************************************************-*/
char serial_getch
  (
    bool console,       // Read the console
    bool aux,           // Read the AUX port
    bool tcpip
  )
{
  char ch;

/*
 * Bring in the console bytes
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
 *  Bring in the TCPIP bytes
 */
  if ( tcpip )
  {
    if ( in_in_ptr != in_out_ptr )
    {
      ch = tcpip_in[in_out_ptr];
      in_out_ptr = (in_out_ptr+1) % sizeof(tcpip_in);
    }
    return ch;
  }

/*
 * Got nothing
 */
  return 0;
}
/*******************************************************************************
 * 
 * @function: serial_putch
 * 
 * @brief:    Send a string to the available serial ports
 * 
 * @return:   None
 * 
 *******************************************************************************
 *
 * Send a string to all of the serial devices that are 
 * in use. 
 * 
 ******************************************************************************/
 void serial_putch
 (
    char ch,
    bool console, 
    bool aux,
    bool tcpip
)
{

/*
 * Output to the devices
 */
  if ( console )
  {
    printf("%c", ch);
  }

  if ( aux )
  {
    uart_write_bytes(uart_aux, (const char *) &ch, 1);
  }
  
  if ( tcpip )
  {
    tcpip_out[out_out_ptr] = ch;
    out_out_ptr = (out_out_ptr + 1) % sizeof(tcpip_out);
  }

/*
 * All done
 */
  return;
}

 
void serial_to_all
(
  char*   str,                      // String to output
  bool  console,                  // Output to the console
  bool  aux,                      // Output to the aux port
  bool  tcpip                     // Output to the TCPIP socket
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
    while(len)
    {
      tcpip_out[out_out_ptr] = *str;
      str++;
      out_out_ptr = (out_out_ptr + 1) % sizeof(tcpip_out);
    }
  }

/*
 * All done
 */
  return;
}

/*******************************************************************************
 * 
 * @function: serial_to_tcpip
 * 
 * @brief:    Get waiting bytes for the tcpip
 * 
 * @return:   Buffer updated
 * 
 *******************************************************************************
 *
 * Called from the TCPIP driver, this function returns the requested number
 * of bytes back to the TCPIP handler for output
 * 
 ******************************************************************************/
int serial_to_tcpip
(
  char* buffer,         // Where to return the bytes
  int   length          // Maximum transfer size
)
{
  int i;

  i=0;
  while ( length )
  {
    *buffer = tcpip_out[out_out_ptr];
    out_out_ptr = (out_out_ptr+1) % sizeof(tcpip_out);
    length--;
    i++;
  }

  return i;

}
