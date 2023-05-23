/*----------------------------------------------------------------
 *
 * gpio.h
 *
 * Header file for GPIO functions
 *
 *---------------------------------------------------------------*/
#ifndef _SERIAL_IO_H_
#define _SERIAL_IO_H_

/*
 * Global functions
 */
void serial_io_init(void);                                // Initialize the Serial ports
void serial_to_all(char* s, bool_t console, bool_t aux, bool_t tcpip);  // Multipurpose driver
void char_to_all(char ch, bool_t console, bool_t aux, bool_t tcpip);    // Output a single character
char serial_gets(bool_t console, bool_t aux, bool_t tcpip);             // Read from all of the ports
char serial_getch(bool_t console, bool_t aux, bool_t tcpip);            // Read the selected port
unsigned int serial_available(bool_t console, bool_t aux, bool_t tcpip);// Find out how much is waiting for us
void serial_flush(bool_t console, bool_t aux, bool_t tcpip);            // Get rid of everything

/*
 *  Definitions
 */
#define CONSOLE true,  false, false
#define AUX     false, true,  false
#define TCPIP   false, false, true
#define ALL     true,  true,  true

#endif
