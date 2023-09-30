/*************************************************************************
 * 
 * file: gpio_init.c
 * 
 * description:  Initialize all of the GPIO pins
 * 
 **************************************************************************
 *
 * This file sets up the configuration of ALL of the GPIOs in one place
 * 
 * It contains definitions for all gpios even if they are not used in thie
 * particular implemenation.  The final table gpio_table contains a list of
 * the GPIOs and a link to the initialization table used for that particular
 * pin
 * 
 ***************************************************************************/
#include "string.h"
#include "esp_event.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/pulse_cnt.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"
//#include "helpers.h"
#include "i2c.h"
#include "led_strip.h"
#include "led_strip_types.h"
#include "pwm.h"
#include "gpio_define.h"
#include "pcnt.h"
#include "diag_tools.h"

/*
 *  Generic Definitions
 */

/*
 *  Digital IO definitions
 */

DIO_struct_t dio00 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio01 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio02 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio03 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio04 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio05 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio06 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio07 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio08 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio09 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 0}; // Mode and Initial Value

DIO_struct_t dio10 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio11 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio12 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio13 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio14 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio15 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio16 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio17 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio18 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio19 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value

DIO_struct_t dio20 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio21 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio22 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio23 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio24 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio25 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio26 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio27 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio28 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio29 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT, .initial_value = 0};  // Mode and Initial Value

DIO_struct_t dio30 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio31 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio32 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio33 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio34 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Can only be input
DIO_struct_t dio35 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Can only be input
DIO_struct_t dio36 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Can only be input
DIO_struct_t dio37 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio38 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0}; // Mode and Initial Value
DIO_struct_t dio39 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 0}; // Can only be input

DIO_struct_t dio40 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio41 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 1};  // Mode and Initial Value
DIO_struct_t dio42 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 1};  // Mode and Initial Value
DIO_struct_t dio43 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio44 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio45 = { .type = DIGITAL_IO, .mode = GPIO_MODE_OUTPUT, .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio46 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio47 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio48 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0};  // Mode and Initial Value
DIO_struct_t dio49 = { .type = DIGITAL_IO, .mode = GPIO_MODE_INPUT,  .initial_value = 0};  // Mode and Initial Value

/*
 *  Analog IO usage
 */
analogIO_struct_t adc1_ch0 = { .type = ANALOG_IO, .gpio = GPIO_NUM_39};   // CHANNEL 1, ADC 0
analogIO_struct_t adc1_ch1 = { .type = ANALOG_IO, .gpio = GPIO_NUM_38};   // CHANNEL 1, ADC 1
analogIO_struct_t adc1_ch2 = { .type = ANALOG_IO, .gpio = GPIO_NUM_3 };   // CHANNEL 1, ADC 2
analogIO_struct_t adc1_ch3 = { .type = ANALOG_IO, .gpio = GPIO_NUM_4 };   // CHANNEL 1, ADC 3
analogIO_struct_t adc1_ch4 = { .type = ANALOG_IO, .gpio = GPIO_NUM_5 };   // CHANNEL 1, ADC 4
analogIO_struct_t adc1_ch5 = { .type = ANALOG_IO, .gpio = GPIO_NUM_6 };   // CHANNEL 1, ADC 5
analogIO_struct_t adc1_ch6 = { .type = ANALOG_IO, .gpio = GPIO_NUM_7 };   // CHANNEL 1, ADC 6
analogIO_struct_t adc1_ch7 = { .type = ANALOG_IO, .gpio = GPIO_NUM_8 };   // CHANNEL 1, ADC 7
analogIO_struct_t adc1_ch8 = { .type = ANALOG_IO, .gpio = GPIO_NUM_9 };   // CHANNEL 1, ADC 9
analogIO_struct_t adc1_ch9 = { .type = ANALOG_IO, .gpio = GPIO_NUM_10};   // CHANNEL 1, ADC 10

analogIO_struct_t adc2_ch0 = { .type = ANALOG_IO, .gpio = GPIO_NUM_11 };  // CHANNEL 2, ADC 0
analogIO_struct_t adc2_ch1 = { .type = ANALOG_IO, .gpio = GPIO_NUM_12 };  // CHANNEL 2, ADC 1
analogIO_struct_t adc2_ch2 = { .type = ANALOG_IO, .gpio = GPIO_NUM_13 };  // CHANNEL 2, ADC 2
analogIO_struct_t adc2_ch3 = { .type = ANALOG_IO, .gpio = GPIO_NUM_14 };  // CHANNEL 2, ADC 3
analogIO_struct_t adc2_ch4 = { .type = ANALOG_IO, .gpio = GPIO_NUM_15 };  // CHANNEL 2, ADC 4
analogIO_struct_t adc2_ch5 = { .type = ANALOG_IO, .gpio = GPIO_NUM_16 };  // CHANNEL 2, ADC 5
analogIO_struct_t adc2_ch6 = { .type = ANALOG_IO, .gpio = GPIO_NUM_17 };  // CHANNEL 2, ADC 6
analogIO_struct_t adc2_ch7 = { .type = ANALOG_IO, .gpio = GPIO_NUM_18 };  // CHANNEL 2, ADC 7
analogIO_struct_t adc2_ch8 = { .type = ANALOG_IO, .gpio = GPIO_NUM_19 };  // CHANNEL 2, ADC 8
analogIO_struct_t adc2_ch9 = { .type = ANALOG_IO, .gpio = GPIO_NUM_20 };  // CHANNEL 2, ADC 9

/*
 *  PWM Control, GPIO filled in by GPIO definition above
 */
PWM_struct_t pwm0 = { .pwm_channel = 0, .type = PWM_OUT};
PWM_struct_t pwm1 = { .pwm_channel = 1, .type = PWM_OUT};
PWM_struct_t pwm2 = { .pwm_channel = 2, .type = PWM_OUT};
PWM_struct_t pwm3 = { .pwm_channel = 3, .type = PWM_OUT};

/*
 *  I2C COntrol.  GPIO explicitly filled in here
 */
I2C_struct_t i2c = { I2C_PORT, GPIO_NUM_38, GPIO_NUM_39 };

PCNT_struct_t pcnt0 = { .type=PCNT, .pcnt_unit = 0, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_5  };
PCNT_struct_t pcnt1 = { .type=PCNT, .pcnt_unit = 1, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_6  };
PCNT_struct_t pcnt2 = { .type=PCNT, .pcnt_unit = 2, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_7  };
PCNT_struct_t pcnt3 = { .type=PCNT, .pcnt_unit = 3, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_15 };
PCNT_struct_t pcnt4 = { .type=PCNT, .pcnt_unit = 4, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_16 };
PCNT_struct_t pcnt5 = { .type=PCNT, .pcnt_unit = 5, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_9  };
PCNT_struct_t pcnt6 = { .type=PCNT, .pcnt_unit = 6, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_10 };
PCNT_struct_t pcnt7 = { .type=PCNT, .pcnt_unit = 7, .pcnt_signal = GPIO_NUM_8, .pcnt_control = GPIO_NUM_11 };

/*
 *  LED Strip
 */
led_strip_config_t        led_strip_config = {  .max_leds = 3 };           // 3 LEDs on the board
led_strip_rmt_config_t    rmt_config       = {.resolution_hz = 10 * 1000 * 1000}; // 10MHz
led_strip_handle_t        led_strip;

/*
 *  GPIO Usage
 *
 *  This table contains the use of each of the individual pins
 * 
 */

gpio_struct_t gpio_table[] = {
//   Name      Number       Assigned
    {"EN",           GPIO_NUM_0,  NULL},              // RTC_GPIO0, GPIO0                       (Strapping EN)
    {"BD_REV",       GPIO_NUM_4,  (void*)&adc1_ch3},  // BD_REV
    {"RUN_NORTH_LO", GPIO_NUM_5,  (void*)&pcnt0},     // RUN_NORTH_LO
    {"RUN_EAST_LO",  GPIO_NUM_6,  (void*)&pcnt1},     // RUN_EAST_LO
    {"RUN_SOUTH_LO", GPIO_NUM_7,  (void*)&pcnt2},     // RUN_SOUTH_LO
    {"RUN_WEST_LO",  GPIO_NUM_15, (void*)&pcnt3},     // RUN_WEST_LO
    {"RUN_NORTH_HI", GPIO_NUM_16, (void*)&pcnt4},     // RUN_NORTH_HI

    {"ATX",          GPIO_NUM_17, NULL},              // ATX
    {"ARX",          GPIO_NUM_18, NULL},              // ARX
    {"REF_CLK",      GPIO_NUM_8,  (void*)&dio08},     // Reference CLodk
    {"USB_D-",       GPIO_NUM_19, NULL},              // JTAG USB D-
    {"USB_D+",       GPIO_NUM_20, NULL},              // JTAG USB D+
    {"USB_JTAG",     GPIO_NUM_3,  NULL},              // JTAG Strap to 3V3
    {"ROM_MSG",      GPIO_NUM_46, NULL},              // Enable ROM messages
    {"RUN_EAST_HI",  GPIO_NUM_9,  (void*)&pcnt5},     // RUN_EAST_HI
    {"RUN_SOUTH_HI", GPIO_NUM_10, (void*)&pcnt6},     // RUN_SOUTH_HI
    {"RUN_WEST_HI",  GPIO_NUM_11, (void*)&pcnt7},     // RUN_WEST_HI
    {"PAPER",        GPIO_NUM_12, (void*)&dio12},     // PAPER  Drive

    {"LED_FB",       GPIO_NUM_1, (void*)&adc1_ch0},  // LED Feedback (Measure 12VDC)
    {"LED_PWM",      GPIO_NUM_2, (void*)&pwm0},      // LED_PWM
    {"TXD",          GPIO_NUM_22, NULL},             // UART Transmit   Initialized in serial_io_init
    {"RXD",          GPIO_NUM_23, NULL},             // UART Receive
    {"LDAC*",        GPIO_NUM_42, (void*)&dio42},    // Load DAC* 
    {"CLK_START*",   GPIO_NUM_41, (void*)&dio41},    // Clock Test Start
    {"FACE_HALF",    GPIO_NUM_40, (void*)&dio40},    // FACE Interrupt
    {"STOP*",        GPIO_NUM_39, (void*)&dio39},    // STOP Clock
    {"A",            GPIO_NUM_38, (void*)&dio38},    // Auxilary Input A

    {"B",            GPIO_NUM_37, (void*)&dio37},    // Auxilary Input B
    {"C",            GPIO_NUM_36, (void*)&dio36},    // Auxilary Input C
    {"D",            GPIO_NUM_35, (void*)&dio35},    // Auxilary Input D
    {"BOOT",         GPIO_NUM_0,  NULL},             // Hold in BOOT
    {"STATUS",       GPIO_NUM_45, (void*)&led_strip_config},// Status LEDs
    {"SPARE0",       GPIO_NUM_48, (void*)&dio48},    // SPARE 0
    {"SPARE1",       GPIO_NUM_47, (void*)&dio47},    // Spare 1
    {"SPARE2",       GPIO_NUM_21, (void*)&dio21},    // Spare 2 
    {"SDA",          GPIO_NUM_38, (void*)&i2c},      // SDA
    {"SCL",          GPIO_NUM_39, NULL},             // SCL
    {0, 0, 0 } 
};


/*************************************************************************
 * 
 * file: gpio_init()
 * 
 * description:  Initialize the GPIO states
 * 
 * return:  Nope
 * 
 **************************************************************************
 *
 * The gpio_table is used to program the gpio hardware
 * 
 ***************************************************************************/

void gpio_init(void)
{
    int i;

    if ( DLT(DLT_CRITICAL) )
    {
        printf("gpio_init()\r\n");
    }
/*
 *  Loop and setup the GPIO
 */
    i=0;
    while (gpio_table[i].gpio_name != 0 )
    {
        if (gpio_table[i].gpio_uses != NULL )
        {
            switch (((DIO_struct_t*)(gpio_table[i].gpio_uses))->type)
            {      
                default:
                    break;

                case DIGITAL_IO:
                    gpio_set_direction(gpio_table[i].gpio_number, ((DIO_struct_t*)(gpio_table[i].gpio_uses))->mode);
                    switch (((DIO_struct_t*)(gpio_table[i].gpio_uses))->mode)
                    {
                        case GPIO_MODE_INPUT:
                            gpio_set_pull_mode(gpio_table[i].gpio_number, GPIO_PULLUP_ONLY);
                            break;

                        case GPIO_MODE_OUTPUT:
                        case GPIO_MODE_OUTPUT_OD:
                        case GPIO_MODE_INPUT_OUTPUT_OD:
                        case GPIO_MODE_INPUT_OUTPUT:
                            gpio_set_level(gpio_table[i].gpio_number, ((DIO_struct_t*)(gpio_table[i].gpio_uses))->initial_value);
                            break;

                        default:
                            break;
                    }
                    break;

                case ANALOG_IO:
#if (0)
                    adc_oneshot_new_unit(gpio_table[i].config[CONFIG], &analogIO_table[i].config[HANDLE]);
                    adc_oneshot_config_channel(gpio_table[i].config[CONFIG], &analogIO_table[i].config[HANDLE]);
#endif
                    break;

                case PWM_OUT:
                    pwm_init(((PWM_struct_t*)(gpio_table[i].gpio_uses))->pwm_channel,\
                       gpio_table[i].gpio_number);
                    break;

                case I2C_PORT:
                    i2c_init(((I2C_struct_t*)(gpio_table[i].gpio_uses))->gpio_number_SDA,\
                       ((I2C_struct_t*)(gpio_table[i].gpio_uses))->gpio_number_SCL );
                    break;

                case PCNT:
                    pcnt_init(((PCNT_struct_t*)(gpio_table[i].gpio_uses))->pcnt_unit,\
                        ((PCNT_struct_t*)(gpio_table[i].gpio_uses))->pcnt_control, \
                        ((PCNT_struct_t*)(gpio_table[i].gpio_uses))->pcnt_signal);
                    break;             

                case LED_STRIP: 
                    led_strip_config.strip_gpio_num = gpio_table[i].gpio_number;
                    led_strip_new_rmt_device(&led_strip_config, &rmt_config, &led_strip);
                
            }
        }
        i++;
    }

/*
 *  all done, return
 */
    return;
}
