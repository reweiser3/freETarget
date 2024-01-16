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
 * @function Prototypes
 */
void gpio_init(void);

/*
 * Type defs
 */
typedef enum gpio_type {
    DIGITAL_IO,                                        // GPIO is used for igital IO
    ANALOG_IO,                                         // GPIO is used for Analog IO
    SERIAL_AUX,                                        // GPIO is used as Serial auxilary port
    PWM_OUT,                                           // GPIO is used as a PWM port
    I2C_PORT,                                          // GPIO is used as a i2c port
    PCNT,                                              // GPIO is used as a Pulse Counter
    LED_STRIP                                          // GPIO is used to drives a LED strip (status LEDs) 
} gpio_type_t;

typedef struct DIO_struct  {
    gpio_type_t     type;                              // What type of structure am I
    int             mode;                              // Mode used by the DIO
    int             initial_value;                     // Value set on initialization
} DIO_struct_t;

typedef struct ADC_struct  {
    gpio_type_t     type;                               // What type of structure am I
    int             adc_channel;                        // What channel are we using?
    int             adc_attenuation;                        // What is the attenuation setting
} ADC_struct_t;

#define ADC(adc, channel) (((adc) * 10) + (channel))    // Pack the channel and ADC into an integer
#define ADC_ADC(id) (id / 10)                           // Unpack the ADC from the integer
#define ADC_CHANNEL(id) (id % 10)                       // Unpack the channel from the interger

typedef struct serialIO_struct  {
    gpio_type_t     type;                                // What type of structure am I
    int             serial_config[4];                    // baud, parity, length, stop bits
} serialIO_struct_t;

typedef struct I2C_struct  {
    gpio_type_t     type;                               // What type of structure am I
    int             gpio_number_SDA;                    // Number associated with SDA
    int             gpio_number_SCL;                    // Number associated with SDA
} I2C_struct_t;

typedef struct PWM_struct  {
    gpio_type_t     type;                               // What type of structure am I
    int             pwm_channel;                        // PWM chanel assigned to this PWM
    int             gpio_number;                        // Number associated with PWM
} PWM_struct_t;

typedef struct PCNT_struct  {
    gpio_type_t     type;                                // What type of structure am I
    int             pcnt_unit;                           // What unit to use
    int             pcnt_control;                        // GPIO associated with PCNT control
    int             pcnt_signal;                         // GPIO associated with PCNT signal
} PCNT_struct_t;

typedef struct LED_strip_struct {
    gpio_type_t     type;                                // What type of structure am I
    int             gpio_number;                         // What unit to use
    int             led_qty;                             // How many LEDs are in the strip
} LED_strip_struct_t;

typedef struct gpio_struct  {
    char*           gpio_name;                           // GPIO name
    int             gpio_number;                         // Number associated with GPIO
    void*           gpio_uses;                           // Pointer to IO specific structure
} gpio_struct_t;

extern gpio_struct_t gpio_table[];                      // List of available devices
#endif
