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
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\gpio_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\hal\include\hal\adc_types.h"
#include "C:\Users\allan\esp\esp-idf\esp-idf\components\esp_adc\include\esp_adc\adc_oneshot.h"

#include "helpers.h"
#include "gpio_define.h"
#include "pwm.h"
#include "i2c.h"

/*
 *  Generic Definitions
 */

/*
 *  Digital IO definitions
 */

DIO_struct_t dio00 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio01 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio02 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio03 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio04 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio05 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio06 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio07 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio08 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                  // Mode and Initial Value
DIO_struct_t dio09 = { DIGITAL_IO, GPIO_MODE_OUTPUT, 0};                  // Mode and Initial Value

DIO_struct_t dio10 = { DIGITAL_IO, GPIO_MODE_OUTPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio11 = { DIGITAL_IO, GPIO_MODE_OUTPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio12 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio13 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio14 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio15 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio16 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio17 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio18 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio19 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value

DIO_struct_t dio20 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio21 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio22 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio23 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio24 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio25 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio26 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio27 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio28 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio29 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value

DIO_struct_t dio30 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio31 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio32 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio33 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio34 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Can only be input
DIO_struct_t dio35 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Can only be input
DIO_struct_t dio36 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Can only be input
DIO_struct_t dio37 = { DIGITAL_IO, GPIO_MODE_INPUT,  0};                 // Mode and Initial Value
DIO_struct_t dio38 = { DIGITAL_IO, GPIO_MODE_OUTPUT, 0};                 // Mode and Initial Value
DIO_struct_t dio39 = { DIGITAL_IO, GPIO_MODE_OUTPUT, 0};                 // Can only be input

DIO_struct_t dio40 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio41 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio42 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio43 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio44 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio45 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio46 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio47 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio48 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value
DIO_struct_t dio49 = { DIGITAL_IO, GPIO_MODE_INPUT, 0};                  // Mode and Initial Value

/*
 *  Analog IO usage
 */
analogIO_struct_t adc1_ch0 = { ANALOG_IO, 0, {0, 0} };          // CHANNEL 1, ADC 0
analogIO_struct_t adc1_ch1 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 1
analogIO_struct_t adc1_ch2 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 2
analogIO_struct_t adc1_ch3 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 3
analogIO_struct_t adc1_ch4 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 4
analogIO_struct_t adc1_ch5 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 5
analogIO_struct_t adc1_ch6 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 6
analogIO_struct_t adc1_ch7 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 7
analogIO_struct_t adc1_ch8 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 8
analogIO_struct_t adc1_ch9 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 1, ADC 9

analogIO_struct_t adc2_ch0 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 0
analogIO_struct_t adc2_ch1 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 1
analogIO_struct_t adc2_ch2 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 2
analogIO_struct_t adc2_ch3 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 3
analogIO_struct_t adc2_ch4 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 4
analogIO_struct_t adc2_ch5 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 5
analogIO_struct_t adc2_ch6 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 6
analogIO_struct_t adc2_ch7 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 7
analogIO_struct_t adc2_ch8 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 8
analogIO_struct_t adc2_ch9 = { ANALOG_IO, 0, {1, 0} };          // CHANNEL 2, ADC 9

/*
 *  PWM Control
 */
pwm_struct_t pwm0 = { PWM_OUT, 0};
pwm_struct_t pwm1 = { PWM_OUT, 0};
pwm_struct_t pwm2 = { PWM_OUT, 0};
pwm_struct_t pwm3 = { PWM_OUT, 0};

I2C_struct_t i2c = { I2C_PORT, GPIO_NUM_38, GPIO_NUM_39 };

/*
 *  GOIO Usage
 *
 *  This table contains the use of each of the individual pins
 * 
 */

gpio_struct_t gpio_table[] = {
//   Name      Number       Assigned
    {"EN",     GPIO_NUM_0,  NULL},              // RTC_GPIO0, GPIO0                       (Strapping EN)
    {"BD_REV", GPIO_NUM_4,  (void*)&adc1_ch3},  // BD_REV
    {"D0",     GPIO_NUM_5,  (void*)&dio05},     // D0
    {"D1",     GPIO_NUM_6,  (void*)&dio06},     // D1
    {"D2",     GPIO_NUM_7,  (void*)&dio07},     // D2
    {"D3",     GPIO_NUM_15, (void*)&dio15},     // D3
    {"D4",     GPIO_NUM_16, (void*)&dio16},     // D4

    {"ATX",    GPIO_NUM_17, NULL},              // ATX
    {"ARX",    GPIO_NUM_18, NULL},              // ARX
    {"C/R*",   GPIO_NUM_8,  (void*)&dio08},     // C/R*   Counters / Register *
    {"USB_D-", GPIO_NUM_19, NULL},              // JTAG USB D-
    {"USB_D+", GPIO_NUM_20, NULL},              // JTAG USB D+
    {"USB_JTAG",GPIO_NUM_3, NULL},              // JTAG Strap to 3V3
    {"ROM_MSG", GPIO_NUM_46,NULL},              // Enable ROM messages
    {"A0",     GPIO_NUM_9,  (void*)&dio09},     // A0     Clock
    {"A1",     GPIO_NUM_10, (void*)&dio10},     // A1     Clock
    {"A2",     GPIO_NUM_11, (void*)&dio11},     // A2     CLOCK
    {"PAPER",  GPIO_NUM_12, (void*)&dio12},     // PAPER  Drive

    {"VREF_FB", GPIO_NUM_1, (void*)&adc1_ch0},  // VREF_FB
    {"LED_PWM", GPIO_NUM_2, (void*)&pwm0},      // LED_PWM
    {"TXD",     GPIO_NUM_22, NULL},             // UART Transmit   Initialized in serial_io_init
    {"RXD",     GPIO_NUM_23, NULL},             // UART Receive
    {"V_PWM",   GPIO_NUM_42, (void*)&pwm1},     // V_REF PWM 
    {"DIAG_C",  GPIO_NUM_41, (void*)&dio41},    // DIAG PIN C
    {"DIAG_D",  GPIO_NUM_40, (void*)&dio40},    // DIAG PIN D
    {"STOP*",   GPIO_NUM_39, (void*)&dio39},    // STOP Clock
    {"CLK_CLR*",GPIO_NUM_29, (void*)&dio38},    // Clear Clock

    {"D5",      GPIO_NUM_37, (void*)&dio37},    // D5
    {"D6",      GPIO_NUM_36, (void*)&dio36},    // D6
    {"D7",      GPIO_NUM_35, (void*)&dio35},    // D7
    {"BOOT",    GPIO_NUM_0,  NULL},             // Hold in BOOT
    {"STATUS",  GPIO_NUM_45, (void*)&dio45},    // Status LEDs
    {"FACE",    GPIO_NUM_48, (void*)&dio48},    // Face Strike
    {"SPARE1",  GPIO_NUM_47, (void*)&dio47},    // Spare 1
    {"SPARE2",  GPIO_NUM_21, (void*)&dio21},    // Spare 2 
    {"SDA",     GPIO_NUM_38, (void*)&i2c},      // SDA
    {"SCL",     GPIO_NUM_39, NULL},             // SCL
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
                    pwm_init((ledc_channel_config_t*)gpio_table[i].gpio_uses, gpio_table[i].gpio_number);
                    break;

                case I2C_PORT:
                    i2c_init(((I2C_struct_t*)(gpio_table[i].gpio_uses))->gpio_number_SDA, ((I2C_struct_t*)(gpio_table[i].gpio_uses))->gpio_number_SCL );
                    break;
            }
        }
        i++;
    }

/*
 *  all done, return
 */
    return;
}
