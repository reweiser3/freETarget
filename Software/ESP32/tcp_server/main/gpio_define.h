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

/*
 *  Scdhematic Capture
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
