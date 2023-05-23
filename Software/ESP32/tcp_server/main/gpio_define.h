/*************************************************************************
 * 
 * file: gpio_define.h
 * 
 * description:  Public interface for gpio definition
 * 
 **************************************************************************
 *
 * 
 ***************************************************************************/

#ifndef __GPIO_DEFINE_H__
#define __GPIO_DEFINE_H__

#define GPIO_NUM_1   1
#define GPIO_NUM_2   2
#define GPIO_NUM_3   3
#define GPIO_NUM_4   4
#define GPIO_NUM_5   5

#define GPIO_NUM_6   6
#define GPIO_NUM_7   7
#define GPIO_NUM_8   8
#define GPIO_NUM_9   9
#define GPIO_NUM_10 10

#define GPIO_NUM_11 11
#define GPIO_NUM_12 12
#define GPIO_NUM_13 13
#define GPIO_NUM_14 14
#define GPIO_NUM_15 15

#define GPIO_NUM_16 16
#define GPIO_NUM_17 17
#define GPIO_NUM_18 18
#define GPIO_NUM_19 19
#define GPIO_NUM_20 20

#define GPIO_NUM_21 21
#define GPIO_NUM_22 22
#define GPIO_NUM_23 23
#define GPIO_NUM_24 24
#define GPIO_NUM_25 25

#define GPIO_NUM_26 26
#define GPIO_NUM_27 27
#define GPIO_NUM_28 28
#define GPIO_NUM_29 29
#define GPIO_NUM_30 30

#define GPIO_NUM_31 31
#define GPIO_NUM_32 32
#define GPIO_NUM_33 33
#define GPIO_NUM_34 34
#define GPIO_NUM_35 35

#define GPIO_NUM_36 36
#define GPIO_NUM_37 37
#define GPIO_NUM_38 38
#define GPIO_NUM_39 39
#define GPIO_NUM_40 40

/*
 *  Schematic Capture
 */

#define D0  GPIO_NUM_10
#define D1  GPIO_NUM_11
#define D2  GPIO_NUM_12
#define D3  GPIO_NUM_13
#define D4  GPIO_NUM_14
#define D5  GPIO_NUM_15
#define D6  GPIO_NUM_16
#define D7  GPIO_NUM_17

#define RCLK GPIO_NUM_18

#define NORTH_HI GPIO_NUM1
#define NORTH_LO GPIO_NUM2
#define EAST_HI  GPIO_NUM1
#define EAST_LO  GPIO_NUM2
#define SOUTH_HI GPIO_NUM4
#define SOUTH_LO GPIO_NUM5
#define WEST_HI  GPIO_NUM6 
#define WEST_LO  GPIO_NUM6 


/*
 * Function Prototypes
 */
void gpio_init(void);

/*
 * Type defs
 */
enum gpio_type {
    DIGITAL_IO,                                             // GPIO is used for igital IO
    ANALOG_IO                                               // GPIO is used for Analog IO
};

typedef enum gpio_type gpio_type_t;

struct DIO_struct  {
    gpio_type_t     type;                                    // What type of structure am I
    int             mode;                                    // Mode used by the DIO
    int             initial_value;                           // Value set on initialization
};

typedef struct DIO_struct DIO_struct_t;

struct analogIO_struct  {
    gpio_type_t                 type;                        // What type of structure am I
    int                         adc_handle;                  // Handle given by OS
    int adc_config[2];                  // Channel setup
};

typedef struct analogIO_struct analogIO_struct_t;

struct gpio_struct  {
    char* gpio_name;                            // GPIO name
    int   gpio_number;                          // Number associated with GPIO
    void* gpio_uses;                            // Pointer to IO specific structure
};

typedef struct gpio_struct gpio_struct_t;

extern gpio_struct_t gpio_table[];              // List of available devices

#endif
