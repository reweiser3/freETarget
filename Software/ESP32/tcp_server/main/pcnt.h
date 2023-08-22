/*----------------------------------------------------------------
 *
 * pcnt.h
 *
 * Header file for Pulse Counter Module
 *
 *---------------------------------------------------------------*/
#ifndef _PCNT_H_
#define _PCNT_H_
#include "gpio_define.h"

/*
 * Global functions
 */
void pcnt_init(PCNT_struct_t*  pcnt);       // pcnt Control

/*
 * Typedefs
 */

/*
 * Definitions
 */
#define NORTH_LOW  0                    // Sensor counter registers
#define EAST_LOW   1
#define SOUTH_LOW  2
#define WEST_LOW   3
#define NORTH_HIGH 4
#define EAST_HIGH  5
#define SOUTH_HIGH 6
#define WEST_HIGH  7

#endif
